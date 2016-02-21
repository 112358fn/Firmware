
/**
 * @addtogroup CIAA_Firmware CIAA Firmware
 * @{
 * @addtogroup USB USB Stack
 * @{
 * @addtogroup USBD USB Driver
 * @{
 */

/*==================[inclusions]=============================================*/
#include <stdint.h>
#include <stdio.h>

#include "usb.h"
#include "usbhci.h"
#include "usb_std.h"
#include "usb_desc.h"
#include "usbd.h"
#include "usbd_states.h"

#include "drivers/usb_drivers.h"
#include "drivers/usb_hub.h"


/*==================[internal data declaration]==============================*/

/** @brief USBD state function type. */
typedef int (*_state_fn_t)( usb_stack_t* pstack, uint8_t index );


/*==================[internal functions declaration]=========================*/

/**
 * @brief Update device state and next-state.
 *
 * @param pdev       Pointer to device to update.
 * @param state      New-state.
 * @param next_state Next-state, used when  waiting  for  some  event.  Once  it
 *                   happens the device will automatically jump  from  new-state
 *                   to this one.
 */
static void _next_state(
      usb_device_t*   pdev,
      usb_dev_state_t state,
      usb_dev_state_t next_state
);

/** @brief Update state to next-state, see @ref _next_state description. */
static void _update_state( usb_device_t* pdev );

/** @brief Request device reset either directly through the HCI or a HUB. */
static void _port_reset( usb_device_t* pdev, uint8_t index );

/** @brief Setup delay in ms (it'll be rounded to ticks). */
static void _set_delay( usb_stack_t* pstack, uint8_t index, uint16_t delay );

/** @brief Request control transfer and wait for its completion. */
static int _ctrl_request( usb_stack_t* pstack, uint8_t idx, usb_stdreq_t* preq);


/*
 * State functions must follow the usb_dev_state_t enumeration order.
 *
 * Following functions do not assert input parameters, this is because they must
 * not be called directly, rather through usbd_state_run() so there's no need.
 */
static int _state_wait_delay( usb_stack_t* pstack, uint8_t index );
static int _state_disconnected( usb_stack_t* pstack, uint8_t index );
static int _state_attached( usb_stack_t* pstack, uint8_t index );
static int _state_powered( usb_stack_t* pstack, uint8_t index );
static int _state_reset( usb_stack_t* pstack, uint8_t index );
static int _state_default( usb_stack_t* pstack, uint8_t index );
static int _state_mps( usb_stack_t* pstack, uint8_t index );
static int _state_address( usb_stack_t* pstack, uint8_t index );
static int _state_dev_desc( usb_stack_t* pstack, uint8_t index );
static int _state_cfg_desc_len9( usb_stack_t* pstack, uint8_t index );
static int _state_cfg_desc( usb_stack_t* pstack, uint8_t index );
static int _state_set_cfg( usb_stack_t* pstack, uint8_t index );
static int _state_unlock( usb_stack_t* pstack, uint8_t index );
static int _state_configured( usb_stack_t* pstack, uint8_t index );
static int _state_suspended( usb_stack_t* pstack, uint8_t index );


/*==================[internal data definition]===============================*/

static _state_fn_t _state_fn[] =
{
   _state_wait_delay,
   _state_disconnected,
   _state_attached,
   _state_powered,
   _state_reset,
   _state_default,
   _state_mps,
   _state_address,
   _state_dev_desc,
   _state_cfg_desc_len9,
   _state_cfg_desc,
   _state_set_cfg,
   _state_unlock,
   _state_configured,
   _state_suspended,
};


/*==================[internal functions definition]==========================*/

static void _next_state(
      usb_device_t*   pdev,
      usb_dev_state_t state,
      usb_dev_state_t next_state
)
{
   pdev->status    |= USB_DEV_STATUS_REQUEST;
   pdev->state      = state;
   pdev->next_state = next_state;
   return;
}

static void _update_state( usb_device_t* pdev )
{
   _next_state(pdev, pdev->next_state, pdev->next_state);
   return;
}

static void _port_reset( usb_device_t* pdev, uint8_t index )
{
#if (USB_MAX_HUBS > 0)
   /*
    * If index isn't 0, then we're enumerating a device through a  HUB,  this
    * means we have to use HUB commands for reseting and such.
    */
   if (index > 0)
   {
      /* HUB port reset. */
      usb_hub_port_reset_start(pdev->parent_hub, pdev->parent_port);
      /* Reset duration is handled by the HUB, just wait for it to finish. */
   }
   else
#endif
   {
      /* Hardware reset. */
      usbhci_reset_start();
   }
   return;
}

static void _set_delay( usb_stack_t* pstack, uint8_t index, uint16_t delay )
{
   pstack->devices[index].ticks_delay = pstack->ticks + delay;
   return;
}

static int _ctrl_request( usb_stack_t* pstack, uint8_t idx, usb_stdreq_t* preq )
{
   int status;
   usb_device_t* pdev = &pstack->devices[idx];

   usb_assert(preq->wLength <= 0xFF);

   if (pdev->status & USB_DEV_STATUS_REQUEST)
   {
      status = usb_ctrlirp_bypass(
            pstack,
            &pdev->ticket,
            USB_TO_ID(idx, 0),
            preq,
            pdev->xfer_buffer
      );
      if (status == USB_STATUS_OK)
      {
         pdev->xfer_length = (uint8_t) preq->wLength;
         pdev->status &= ~USB_DEV_STATUS_REQUEST;
         status = USB_STATUS_XFER_WAIT;
      }
      else if (status == USB_STATUS_BUSY)
      {
         status = USB_STATUS_XFER_WAIT;
      }
   }
   else
   {
      /* Waiting for acknowledge... */
      status = usb_irp_status(pstack, USB_TO_ID(idx, 0), pdev->ticket);
   }
   return status;
}

/****************************** States follow *********************************/

static int _state_wait_delay( usb_stack_t* pstack, uint8_t index )
{
   usb_device_t* pdev = &pstack->devices[index];
   if (((int32_t) pdev->ticks_delay) - ((int32_t) pstack->ticks) <= 0)
   {
      /* If delay is done, go to next state. */
      _update_state(pdev);
   }
   return USB_STATUS_OK;
}

static int _state_disconnected( usb_stack_t* pstack, uint8_t index )
{
   /* Waiting for connection, updated via usb_device_attach(). */
   return USB_STATUS_OK;
}

static int _state_attached( usb_stack_t* pstack, uint8_t index )
{
   /* Device attached, wait for powerline to settle. */
   usb_device_t* pdev = &pstack->devices[index];
   _set_delay(pstack, index, 100);
   _next_state(pdev, USB_DEV_STATE_WAIT_DELAY, USB_DEV_STATE_POWERED);
   return USB_STATUS_OK;
}

static int _state_powered( usb_stack_t* pstack, uint8_t index )
{
   usb_device_t* pdev = &pstack->devices[index];
#if (USB_MAX_HUBS > 0)
   /* We're accessing the root device or one with a valid parent HUB index. */
   usb_assert(index == 0 || pdev->parent_hub  < USB_MAX_HUBS);
   usb_assert(index == 0 || pdev->parent_port < USB_MAX_HUB_PORTS);
#endif
   /*
    * Drive USB reset for 10~20 ms, but  only  if  the  host  is  currently  not
    * resetting or communicating to another device on address 0.   Also,  notify
    * the stack that someone  will  begin  an  enumeration  process,  this  mean
    * address 0 will be in use by this device for the  time  being.   Any  other
    * devices' reset and addressing process pending will have to wait until this
    * one completes.
    */
   if (!(pstack->status & USB_STACK_STATUS_ZERO_ADDR))
   {
      /* Address 0 is free, otherwise we'd have to wait. */
      pstack->status |= USB_STACK_STATUS_ZERO_ADDR;/* Mark it as in-use */
      _port_reset(pdev, index);
      _next_state(pdev, USB_DEV_STATE_RESET, USB_DEV_STATE_DEFAULT);
   }
   return USB_STATUS_OK;
}

static int _state_reset( usb_stack_t* pstack, uint8_t index )
{
   int status;
   usb_device_t* pdev = &pstack->devices[index];
#if (USB_MAX_HUBS > 0)
   /* We're accessing the root device or one with a valid parent HUB index. */
   usb_assert(index == 0 || pdev->parent_hub  < USB_MAX_HUBS);
   usb_assert(index == 0 || pdev->parent_port < USB_MAX_HUB_PORTS);

   if (index > 0)
   {
      /* Here we wait until the HUB releases the bus reset. */
      status = usb_hub_port_reset_status(pdev->parent_hub, pdev->parent_port);
   }
   else
#endif
   {
      /* Wait until reset is released. */
      status = usbhci_reset_stop();
   }
   if (status == USB_STATUS_OK)
   {
      _update_state(pdev);
   }
   return USB_STATUS_OK;
}

static int _state_default( usb_stack_t* pstack, uint8_t index )
{
   /*
    * Get the device's speed and  create  the  0  address  control  pipes,  then
    * configure its new address and release the zero-address  communication  bit
    * so the host can setup other devices.
    */
   usb_device_t* pdev = &pstack->devices[index];

#if (USB_MAX_HUBS > 0)
   /* We're accessing the root device or one with a valid parent HUB index. */
   usb_assert(index == 0 || pdev->parent_hub  < USB_MAX_HUBS);
   usb_assert(index == 0 || pdev->parent_port < USB_MAX_HUB_PORTS);

   if (index > 0)
   {
      /* Get speed of connected device from HUB's port. */
      pdev->speed = usb_hub_get_speed(pdev->parent_hub, pdev->parent_port);
   }
   else
#endif
   {
      /* Otherwise, get it directly from the HCI. */
      pdev->speed = usbhci_get_speed();
   }

   /*
    * Once the reset and speed reading is done,  talking  with  the  HUB  is  no
    * longer needed for this device.
    * Now, we need to request the std descriptor to find  out  the  MPS  of  the
    * default endpoint. Start with a MPS of 8.
    */
   pdev->addr = 0;
   pdev->mps  = 8;
   _next_state(pdev, USB_DEV_STATE_MPS, USB_DEV_STATE_MPS);

   return USB_STATUS_OK;
}

static int _state_mps( usb_stack_t* pstack, uint8_t index )
{
   int           status;
   usb_stdreq_t  stdreq;
   usb_device_t* pdev = &pstack->devices[index];

   stdreq.bmRequestType = USB_STDREQ_REQTYPE(
         USB_DIR_IN,
         USB_STDREQ_TYPE_STD,
         USB_STDREQ_RECIP_DEV );
   stdreq.bRequest      = USB_STDREQ_GET_DESCRIPTOR;
   stdreq.wValue        = USB_STDDESC_DEVICE << 8;
   stdreq.wIndex        = 0;
   stdreq.wLength       = 8;

   status = _ctrl_request(pstack, index, &stdreq);
   if (status == USB_STATUS_XFER_WAIT)
   {
      /* Do nothing and keep waiting. */
      status = USB_STATUS_OK;
   }
   else if (status != USB_STATUS_OK)
   {
      usb_assert(0); /** @TODO: handle error */
   }
   else /* USB_STATUS_OK */
   {
      /* Get MPS and reset device once again. */
      pdev->mps = USB_STDDESC_DEV_GET_bMaxPacketSize0(pdev->xfer_buffer);
      _port_reset(pdev, index);
      _next_state(pdev, USB_DEV_STATE_RESET, USB_DEV_STATE_ADDRESS);
   }
   return status;
}

static int _state_address( usb_stack_t* pstack, uint8_t index )
{
   int           status;
   usb_stdreq_t  stdreq;
   usb_device_t* pdev = &pstack->devices[index];

   /* Send control request to set device's new address. */
   stdreq.bmRequestType = USB_STDREQ_REQTYPE(
         USB_DIR_OUT,
         USB_STDREQ_TYPE_STD,
         USB_STDREQ_RECIP_DEV );
   stdreq.bRequest      = USB_STDREQ_SET_ADDRESS;
   stdreq.wValue        = index + 1;
   stdreq.wIndex        = 0;
   stdreq.wLength       = 0;

   status = _ctrl_request(pstack, index, &stdreq);
   if (status == USB_STATUS_XFER_WAIT)
   {
      /* Do nothing and keep waiting. */
      status = USB_STATUS_OK;
   }
   else if (status != USB_STATUS_OK)
   {
      usb_assert(0); /** @TODO: handle error */
   }
   else /* USB_STATUS_OK */
   {
      /* Address has been set, go get the device descriptor. */
      pdev->addr = index + 1;
      _next_state(pdev, USB_DEV_STATE_DEV_DESC, USB_DEV_STATE_DEV_DESC);
   }
   return status;
}

static int _state_dev_desc( usb_stack_t* pstack, uint8_t index )
{
   int           status;
   usb_stdreq_t  stdreq;
   usb_device_t* pdev = &pstack->devices[index];

   /* Request device descriptor. */
   stdreq.bmRequestType = USB_STDREQ_REQTYPE(
         USB_DIR_IN,
         USB_STDREQ_TYPE_STD,
         USB_STDREQ_RECIP_DEV );
   stdreq.bRequest      = USB_STDREQ_GET_DESCRIPTOR;
   stdreq.wValue        = USB_STDDESC_DEVICE << 8;
   stdreq.wIndex        = 0;
   stdreq.wLength       = USB_STDDESC_DEV_SIZE;

   status = _ctrl_request(pstack, index, &stdreq);
   if (status == USB_STATUS_XFER_WAIT)
   {
      /* Do nothing and keep waiting. */
      status = USB_STATUS_OK;
   }
   else if (status != USB_STATUS_OK)
   {
      usb_assert(0); /** @TODO: handle error */
   }
   else /* USB_STATUS_OK */
   {
      pdev->vendor_ID  = USB_STDDESC_DEV_GET_idVendor(pdev->xfer_buffer);
      pdev->product_ID = USB_STDDESC_DEV_GET_idProduct(pdev->xfer_buffer);
      _next_state(pdev,USB_DEV_STATE_CFG_DESC_LEN9,USB_DEV_STATE_CFG_DESC_LEN9);
   }
   return status;
}

static int _state_cfg_desc_len9( usb_stack_t* pstack, uint8_t index )
{
   int           status;
   usb_stdreq_t  stdreq;
   usb_device_t* pdev = &pstack->devices[index];

   /* Request first 9 bytes of the configuration descriptor. */
   stdreq.bmRequestType = USB_STDREQ_REQTYPE(
         USB_DIR_IN,
         USB_STDREQ_TYPE_STD,
         USB_STDREQ_RECIP_DEV );
   stdreq.bRequest      = USB_STDREQ_GET_DESCRIPTOR;
   stdreq.wValue        = USB_STDDESC_CONFIGURATION << 8;
   stdreq.wIndex        = 0;
   stdreq.wLength       = USB_STDDESC_CFG_SIZE;

   status = _ctrl_request(pstack, index, &stdreq);
   if (status == USB_STATUS_XFER_WAIT)
   {
      /* Do nothing and keep waiting. */
      status = USB_STATUS_OK;
   }
   else if (status != USB_STATUS_OK)
   {
      usb_assert(0); /** @TODO: handle error */
   }
   else /* USB_STATUS_OK */
   {
      if (USB_STDDESC_CFG_GET_wTotalLength(pdev->xfer_buffer)
            > USB_XFER_BUFFER_LEN)
      {
         usb_assert(0); /** @TODO handle error */
      }
      _next_state(pdev, USB_DEV_STATE_CFG_DESC, USB_DEV_STATE_CFG_DESC);
   }
   return status;
}

static int _state_cfg_desc( usb_stack_t* pstack, uint8_t index )
{
   int           status;
   usb_stdreq_t  stdreq;
   usb_device_t* pdev = &pstack->devices[index];

   /* Request the entire configuration descriptor. */
   stdreq.bmRequestType = USB_STDREQ_REQTYPE(
         USB_DIR_IN,
         USB_STDREQ_TYPE_STD,
         USB_STDREQ_RECIP_DEV );
   stdreq.bRequest      = USB_STDREQ_GET_DESCRIPTOR;
   stdreq.wValue        = USB_STDDESC_CONFIGURATION << 8;
   stdreq.wIndex        = 0;
   stdreq.wLength       = USB_STDDESC_CFG_GET_wTotalLength(pdev->xfer_buffer);

   status = _ctrl_request(pstack, index, &stdreq);
   if (status == USB_STATUS_XFER_WAIT)
   {
      /* Do nothing and keep waiting. */
      status = USB_STATUS_OK;
   }
   else if (status != USB_STATUS_OK)
   {
      usb_assert(0); /** @TODO: handle error */
   }
   else /* USB_STATUS_OK */
   {
      /* Parse configuration descriptor and assign each interface a driver. */
      status = usb_device_parse_cfgdesc(pstack, index);
      if (status)
      {
         usb_assert(0); /** @TODO handle error */
      }
      _next_state(pdev, USB_DEV_STATE_SET_CFG, USB_DEV_STATE_SET_CFG);
   }

   return status;
}

static int _state_set_cfg( usb_stack_t* pstack, uint8_t index )
{
   int           status;
   usb_stdreq_t  stdreq;
   usb_device_t* pdev = &pstack->devices[index];

   /* Set the device's configuration to the first one (0). */
   stdreq.bmRequestType = USB_STDREQ_REQTYPE(
         USB_DIR_OUT,
         USB_STDREQ_TYPE_STD,
         USB_STDREQ_RECIP_DEV );
   stdreq.bRequest      = USB_STDREQ_SET_CONFIGURATION;
   stdreq.wValue        = pdev->cfg_value;
   stdreq.wIndex        = 0;
   stdreq.wLength       = 0;

   status = _ctrl_request(pstack, index, &stdreq);
   if (status == USB_STATUS_XFER_WAIT)
   {
      /* Do nothing and keep waiting. */
      status = USB_STATUS_OK;
   }
   else if (status != USB_STATUS_OK)
   {
      usb_assert(0); /** @TODO: handle error */
   }
   else /* USB_STATUS_OK */
   {
      _set_delay(pstack, index, 500);
      _next_state(pdev, USB_DEV_STATE_WAIT_DELAY, USB_DEV_STATE_UNLOCK);
   }
   return status;
}

static int _state_unlock( usb_stack_t* pstack, uint8_t index )
{
   usb_device_t* pdev = &pstack->devices[index];
   pstack->status &= ~USB_STACK_STATUS_ZERO_ADDR; /* Unlock address 0. */
   pdev->status |= USB_DEV_STATUS_INIT; /* Device enumerated. */
   _next_state(pdev, USB_DEV_STATE_CONFIGURED, USB_DEV_STATE_CONFIGURED);
   return USB_STATUS_OK;
}

static int _state_configured( usb_stack_t* pstack, uint8_t index )
{
   /* Nothing to do here... ? */
   return USB_STATUS_OK;
}

static int _state_suspended( usb_stack_t* pstack, uint8_t index )
{
   /* @TODO implement the suspended state */
   return USB_STATUS_OK;
}


/*==================[external functions definition]==========================*/

int usbd_state_run( usb_stack_t* pstack, uint8_t index )
{
   usb_assert(pstack != NULL);
   usb_assert(index < USB_MAX_DEVICES);
   return _state_fn[pstack->devices[index].state](pstack, index);
}

/** @} USB_DESC */
/** @} USB */
/** @} CIAA_Firmware */
/*==================[end of file]============================================*/

