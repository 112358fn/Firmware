/**********************************************************************
* $Id$		main.c			2011-06-02
*//**
* @file		main.c
* @brief	USB composite example project.
* @version	1.0
* @date		02. June. 2011
* @author	NXP MCU SW Application Team
*
* Copyright(C) 2011, NXP Semiconductor
* All rights reserved.
*
***********************************************************************
* Software that is described herein is for illustrative purposes only
* which provides customers with programming information regarding the
* products. This software is supplied "AS IS" without any warranties.
* NXP Semiconductors assumes no responsibility or liability for the
* use of the software, conveys no license or title under any patent,
* copyright, or mask work right to the product. NXP Semiconductors
* reserves the right to make changes in the software without
* notification. NXP Semiconductors also make no representation or
* warranty that such application will be suitable for the specified
* use without further testing or modification.
* Permission to use, copy, modify, and distribute this software and its
* documentation is hereby granted, under NXP Semiconductors'
* relevant copyright in the software, without fee, provided that it
* is used in conjunction with NXP Semiconductors microcontrollers.  This
* copyright, permission, and disclaimer notice must appear in all copies of
* this code.
**********************************************************************/
#include <string.h>
#include "lpc43xx.H"
#include "config.h"
#include "lpc43xx_cgu.h"

#include "usbd/usbd_mscuser.h"
#include "usbd/usbd_core.h"
#include "usbd/usbd_hw.h"
#include "usbd/usbd_mscuser.h"
#include "usbd/usbd_rom_api.h"

/* Example group ----------------------------------------------------------- */
/** @defgroup USBDEV_ROM_USBComposite	USBComposite
 * @ingroup USBDEV_ROM_Examples
 * @{
 */
/**********************************************************************
 ** Extern Function prototyping
**********************************************************************/
extern ErrorCode_t usb_msc_mem_init(USBD_HANDLE_T hUsb,
  USB_INTERFACE_DESCRIPTOR* pIntfDesc, uint32_t* mem_base,
  uint32_t* mem_size);
extern ErrorCode_t usb_dfu_init(USBD_HANDLE_T hUsb,
  USB_INTERFACE_DESCRIPTOR* pIntfDesc, uint32_t* mem_base,
  uint32_t* mem_size);
extern ErrorCode_t usb_hid_init(USBD_HANDLE_T hUsb,
  USB_INTERFACE_DESCRIPTOR* pIntfDesc, uint32_t* mem_base,
  uint32_t* mem_size);
extern uint32_t copy_descriptors(USB_CORE_DESCS_T* pDesc,
  uint32_t mem_base);
extern ErrorCode_t USB_Configure_Event (USBD_HANDLE_T hUsb);
extern void overwrt_dfu_only_desc(uint8_t* pFsDesc, uint8_t* pHsDesc);


/**********************************************************************
 ** Function prototyping
**********************************************************************/
void dfu_detach(USBD_HANDLE_T hUsb);
void USB0_IRQHandler(void);
void USB1_IRQHandler(void);
void vCatchError(uint8_t u8Error);


/**********************************************************************
 ** Global data
**********************************************************************/
volatile uint32_t u32Milliseconds = 0;
volatile uint32_t dfu_detach_sig = 0;


/* local data */
static USBD_HANDLE_T hUsb;



/**********************************************************************
 ** Function name:
 **
 ** Description:
 **
 ** Parameters:
 **
 ** Returned value:
 **********************************************************************/
int main (void)
{
  USBD_API_INIT_PARAM_T usb_param;
  USB_CORE_DESCS_T desc;
	ErrorCode_t ret;
  USB_COMMON_DESCRIPTOR *pD;
  USB_INTERFACE_DESCRIPTOR* pIntfDesc;
  uint32_t next_desc_adr, total_len = 0;

	SystemInit();
	CGU_Init();
	CGU_SetPLL0();
	CGU_EnableEntity(CGU_CLKSRC_PLL0, ENABLE);
	CGU_EntityConnect(CGU_CLKSRC_PLL0,CGU_BASE_USB0);

  /* Generate interrupt @ 1000 Hz */
  SysTick_Config(CGU_GetPCLKFrequency(CGU_PERIPHERAL_M4CORE)/1000);
  /* initilize call back structures */
  memset((void*)&usb_param, 0, sizeof(USBD_API_INIT_PARAM_T));
  usb_param.usb_reg_base = LPC_USB0_BASE;
  usb_param.mem_base = 0x20002000;
  usb_param.mem_size = 0x2000;
  usb_param.max_num_ep = 6;
  usb_param.USB_Configure_Event = USB_Configure_Event;

  /* for eagle/raptor the local SRAM is not accesable to USB
  * so copy the descriptors to USB accessable memory
  */
  copy_descriptors(&desc, usb_param.mem_base + usb_param.mem_size);

 	/* Turn on the phy */
 	LPC_CREG->CREG0 &= ~(1<<5);

  /* USB Initialization */
  ret = USBD_API->hw->Init(&hUsb, &desc, &usb_param);
  if (ret == LPC_OK) {

    pD = (USB_COMMON_DESCRIPTOR *)desc.high_speed_desc;
    next_desc_adr = (uint32_t)desc.high_speed_desc;

    while (pD->bLength) {
      if (pD->bDescriptorType == USB_INTERFACE_DESCRIPTOR_TYPE) {

        pIntfDesc = (USB_INTERFACE_DESCRIPTOR*)pD;

        switch(pIntfDesc->bInterfaceClass) {
          case USB_DEVICE_CLASS_STORAGE:
            ret = usb_msc_mem_init(hUsb, pIntfDesc, &usb_param.mem_base, &usb_param.mem_size);
            if (ret != LPC_OK)
              vCatchError(0); //"usb_msc_mem_init error!!!"
            break;
          case USB_DEVICE_CLASS_APP:
            ret = usb_dfu_init(hUsb, pIntfDesc, &usb_param.mem_base, &usb_param.mem_size);
            if (ret != LPC_OK)
              vCatchError(1); //"\r\n usb_dfu_init error!!!"
            break;
          case USB_DEVICE_CLASS_HUMAN_INTERFACE:
            ret = usb_hid_init(hUsb, pIntfDesc, &usb_param.mem_base, &usb_param.mem_size);
            if (ret != LPC_OK)
              vCatchError(2); //"\r\n usb_hid_init error!!!"

            break;
        }
        if (ret != LPC_OK)
          break; /* break while loop no point proceeding further */
      }
      pIntfDesc = 0;
      total_len += pD->bLength;
      next_desc_adr = (uint32_t)pD + pD->bLength;
      pD = (USB_COMMON_DESCRIPTOR*)next_desc_adr;
    }

    if (total_len != ((USB_CONFIGURATION_DESCRIPTOR *)desc.high_speed_desc)->wTotalLength)
      vCatchError(3); //"\r\nBadly formed config descriptor!!!"

    if (ret == LPC_OK) {
      NVIC_EnableIRQ(USB0_IRQn); //  enable USB0 interrrupts
      /* now connect */
      USBD_API->hw->Connect(hUsb, 1);
    }

  }
  else {
    vCatchError(4); //"\r\nhwUSB_Init error!!!"
  }

	while (1)
	{
		u32Milliseconds = 100;

    if (dfu_detach_sig) {
      /* disconnect */
      USBD_API->hw->Connect(hUsb, 0);
      /* for current test wrap-around condition is overlooked */
      while (u32Milliseconds < 98);
      /* connect the device back */
      USBD_API->hw->Connect(hUsb, 1);
      dfu_detach_sig = 0;
    }
		/* Wait... */
		while(u32Milliseconds);

	}
}

/**********************************************************************
 ** Function name:
 **
 ** Description:
 **
 ** Parameters:
 **
 ** Returned value:
 **********************************************************************/
void SysTick_Handler (void)
{
	if(u32Milliseconds > 0)
	{
		u32Milliseconds--;
	}
}

/**********************************************************************
 ** Function name:
 **
 ** Description:
 **
 ** Parameters:
 **
 ** Returned value:
 **********************************************************************/
void dfu_detach(USBD_HANDLE_T hUsb)
{
  USB_CORE_CTRL_T* pCtrl = (USB_CORE_CTRL_T*)hUsb;

  overwrt_dfu_only_desc(pCtrl->full_speed_desc, pCtrl->high_speed_desc);

  dfu_detach_sig = 1;
}



/**********************************************************************
 ** Function name:
 **
 ** Description:
 **
 ** Parameters:
 **
 ** Returned value:
 **********************************************************************/
void USB0_IRQHandler(void)
{
  USBD_API->hw->ISR(hUsb);
}

/**********************************************************************
 ** Function name:
 **
 ** Description:
 **
 ** Parameters:
 **
 ** Returned value:
 **********************************************************************/
void vCatchError(uint8_t u8Error)
{
	volatile uint8_t u8theError = u8Error;

	while(1);
}

/**
 * @}
 */

/**********************************************************************
 **                            End Of File
 **********************************************************************/
