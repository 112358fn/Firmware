#ifndef __USB_H__
#define __USB_H__
/**
* @addtogroup USB
* @brief CIAA USB common types and structures
*
* At the moment, only the most important information is stored in memory,  other
* attributes needed will have to be requested from the device via IRPs.
*
* @warning
* All devices use the  same  structure  with  fixed  number  of  interfaces  and
* endpoints per interface.  This is not the most efficient way of  using  memory
* since the  "biggest"  device  will  make  all  the  others  potentially  waste
* resources.  One way to improve this would be generating the device's structure
* with a script (like the php OSEK one) depending on what the user needs.   This
* is also particularly important regarding  the  control  transactions  buffer's
* size, since it  will  need  to  be  as  large  as  the  largest  configuration
* descriptor among all devices (which can be quite so in complex devices such as
* cellphones).
*
* @{ */

#include <stdint.h>
#include "usb_std.h"


/**
* @name Constants
* @{ */

/** @brief Maximum number of devices.  */
#define USB_MAX_DEVICES      1

/** @brief Maximum number of interfaces per device.  */
#define USB_MAX_INTERFACES   1

/** @brief Maximum number of endpoints per interface (besides ep. 0).  */
#define USB_MAX_ENDPOINTS    3

/** @brief Maximum number of HUBs, 0 equals no HUB support.  */
#define USB_MAX_HUBS         0

/** @brief Maximum number of HUB ports per HUB.  */
#define USB_MAX_HUB_PORTS    4

/** @brief Address mask.  */
#define USB_ADDR_MASK        0x7F
/** @brief Direction mask.  */
#define USB_DIR_MASK         0x80
/** @brief Direction shift.  */
#define USB_DIR_S            7

/** @brief Control transfer's buffer length.  */
#define USB_XFER_BUFFER_LEN  256

/** @} Constants */

/**
 * @brief Status codes.
 *
 */
enum _usb_status_t
{
    USB_STATUS_OK = 0,         /**< Success.                                  */
	USB_STATUS_INV_PARAM,      /**< Invalid argument/parameter to function.   */

	USB_STATUS_DEV_NOT_FOUND,  /**< Device not found in any HUB.              */
	USB_STATUS_DRIVER_NA,      /**< No driver found for device.               */
	USB_STATUS_EP_NA,          /**< No endpoint descriptor found.             */
	USB_STATUS_DRIVER_FAIL,    /**< Driver could not identify device.         */

	USB_STATUS_PIPE_CFG,       /**< Unable to setup pipe.                     */
	USB_STATUS_HCD_INIT,       /**< Cannot initialize Host Controller Driver. */

	USB_STATUS_INV_DESC,       /**< Invalid descriptor.                       */
	USB_STATUS_EP_AVAIL,       /**< Not enough pipes available.               */

	USB_STATUS_EP_STALL,
	USB_STATUS_DEV_UNREACHABLE,
	USB_STATUS_BUSY,           /**< Control endpoint is currently busy.       */

	USB_STATUS_XFER_ERR,       /**< Error during transfer.                    */
	USB_STATUS_XFER_WAIT,      /**< Transfer in progress.                     */
	USB_STATUS_XFER_DONE,      /**< Transfer complete.              ????????  */
};


/** @brief Device's speed identifier.  */
typedef enum _usb_speed_t
{
	USB_SPD_LS,  /**< Low-speed.     */
	USB_SPD_FS,  /**< Full-speed.    */
	USB_SPD_HS,  /**< High-speed.    */
	USB_SPD_INV  /**< Invalid/error. */
} usb_speed_t;


/** @brief Endpoint/Pipe's direction.  */
typedef enum _usb_dir_t
{
	USB_DIR_OUT = 0,  /**< Towards device from host.     */
	USB_DIR_IN  = 1,  /**< Towards host from device.     */
	USB_DIR_TOK = 2,  /**< Token: for control transfers. */
} usb_dir_t;


/** @brief Endpoint/Pipe's type.  */
typedef enum _usb_xfer_type_t
{
	USB_CTRL = 0,  /**< Control.        */
	USB_ISO  = 1,   /**< Isochronous.    */
	USB_BULK = 2,  /**< Bulk.           */
	USB_INT  = 3,   /**< Interrupt.      */
} usb_xfer_type_t;


/**
 * @brief Pipe structure.
 *
 * Pipe's direction is stored in the most significant bit of addr.
 *
 * The interval represents every how many  frames  a  transfer  is  expected  to
 * happen. For example, a period of 0 means a transfer on  every  frame  whereas
 * a period of 1 means a transfer on every other frame.
 */
typedef struct _usb_pipe_t
{
    uint8_t          handle;    /**< Hardware pipe's handle.              */
	uint8_t          number;    /**< Endpoint number (0 - 15).            */
    usb_xfer_type_t  type;      /**< Type (ctrl, bulk, int, iso).         */
    usb_dir_t        dir;       /**< Endpoint's direction.                */
    uint8_t          mps;       /**< Maximum packet size.                 */
    uint8_t          interval;  /**< Frames between INT or ISO transfers. */
} usb_pipe_t;


/** @brief USB driver's handle type. */
typedef uint8_t usb_driver_handle_t;

/**
 * @brief USB device structure.
 *
 * Endpoints are instantiated as pipes because, even though they are referred to
 * as endpoints on the device end, the  host  connects  to  them  through  local
 * pipes. Also, an interface cannot be created  without  its  associated  pipes,
 * thus, configuring them here is the best option.
 *
 * The driver bound to  the  interface  will  talk  to  the  device's  endpoints
 * directly, although this will be handled as needed internally.
 */
typedef struct _usb_interface_t
{
	usb_pipe_t          endpoints[USB_MAX_ENDPOINTS]; /**< Array of endpoints.*/
	uint16_t            n_endpoints;    /**< Number of endpoints.             */
	usb_driver_handle_t driver_handle;  /**< Interface's driver handle.       */
	uint8_t             class;
	uint8_t             subclass;
	uint8_t             protocol;
} usb_interface_t;

#define USB_IFACE_NO_DRIVER ((usb_driver_handle_t)-1)


/**
 * @brief Device's possible states from the stacks perspective.
 *
 * This are all the possible states a device can be in according to the  USB 2.0
 * specification, plus the DISCONNECTED state to represent a device not  present
 * and a couple more used internally by the host for setup and enumeration.
 *
 * The enumeration and driver bonding process are handled from state POWERED  to
 * CONFIGURED.
 */
typedef enum _usb_dev_state_t
{
	USB_DEV_STATE_WAITING_ACK,       /**< Waiting for a control transaction 
	                                      during enumeration to ACK.          */
	USB_DEV_STATE_WAITING_DELAY,     /**< Waiting for a delay to expire.      */
    USB_DEV_STATE_DISCONNECTED,      /**< No device connected.                */
    USB_DEV_STATE_ATTACHED,          /**< Attached, waiting for power.        */
    USB_DEV_STATE_POWERED,           /**< Powered, assert USB reset.          */
	USB_DEV_STATE_RESET,             /**< Holding USB reset high for 10~20 ms.*/
    USB_DEV_STATE_DEFAULT,           /**< Reseted, configuring pipes and new
	                                      address.                            */
    USB_DEV_STATE_ADDRESS,           /**< Address being assigned.             */
	USB_DEV_STATE_CONFIGURING_PIPES, /**< Configure ctrl. pipe to new address
	                                      and request dev. desc.              */
	USB_DEV_STATE_DEV_DESC,          /**< Parse dev. desc. and request config.
	                                      descriptor's first 9 bytes.         */
	USB_DEV_STATE_CFG_DESC_9,        /**< Get cfg. desc.'s length and request
	                                      it full (up to 256 bytes).          */
	USB_DEV_STATE_CFG_DESC,          /**< Parse cfg. desc. and request ifaces.
	                                      descriptors.                        */
	USB_DEV_STATE_IFACE_DESC,        /**< */
    USB_DEV_STATE_CONFIGURED,        /**< Configured, in stand by for
	                                      transactions.                       */
    USB_DEV_STATE_SUSPENDED          /**< Bus inactive, waiting for activity. */
} usb_dev_state_t;

/**
 * @brief USB device structure.
 *
 * A device structure can have different statuses at a given time,  each  status
 * will be represented by a bit in the status member. As of  now,  there's  only
 * the 'connected' and the 'opened' status.
 *
 * 'connected' devices are being handled by drivers but no user code has claimed
 * them.
 *
 * 'opened' devices have been claimed by  user  code  and  cannot  be  otherwise
 * claimed by someone else.
 *
 * @warning A single endpoint is used for control pipes, it will be reconfigured
 * as needed for every IN or OUT transaction.
 *
 * @warning Control pipe's buffer can become quite  large  on  devices  with  an
 * extensive configuration descriptor, as it  is  now  this  will  affect  *all*
 * devices' structure.
 *
 * @warning The buffer's size is currently limited to 256  bytes,  however,  the
 * USB specification assigns it a 2-byte value, so this limit is  being  imposed
 * by the implementation.  On the other hand, 256 bytes should be plenty  enough
 * to handle quite some complex devices, even with multiple interfaces.
 *
 */
typedef struct _usb_device_t
{
	uint32_t        status;         /**< Device status, see description.      */
	usb_dev_state_t state;          /**< Device's current state.              */
	usb_dev_state_t next_state;     /**< Device's state after a delay.        */
    usb_speed_t     speed;          /**< Speed (LS, FS or HS).                */
    uint8_t         addr;           /**< Device's address.                    */
	usb_pipe_t      default_ep;     /**< Default control pipes (endpoint 0).  */
	usb_stdreq_t    stdreq;         /**< Standard requests.                   */
	uint8_t         xfer_buffer[USB_XFER_BUFFER_LEN]; /**< Control buffer.    */
	usb_interface_t interfaces[USB_MAX_INTERFACES]; /**< Array of interfaces. */
	uint16_t        vendor_ID;      /**< Vendor ID.                           */
	uint16_t        product_ID;     /**< Product ID.                          */
	uint16_t        ticks_delay;    /**< Delay this number of ticks count.    */
#if (USB_MAX_HUBS > 0)
	uint8_t         parent_hub;     /**< Index of upstream HUB.               */
	uint8_t         parent_port;    /**< Index of upstream HUB.               */
#endif
	uint8_t         n_interfaces;   /**< Number of interfaces.                */
	uint8_t         xfer_length;    /**< Control buffer's length.             */
} usb_device_t;

#define	USB_DEV_DEFAULT_ADDR      ((uint8_t) -1) /* Invalid address */

/**
* @name Constants
* @{ */
/** @brief Device connected (updated in ISR). */
#define USB_DEV_STATUS_CONNECTED  (1 << 0)
/** @brief Device active and connected. */
#define USB_DEV_STATUS_ACTIVE     (1 << 1)
/** @brief Device's control endpoint opened by user code. */
#define USB_DEV_STATUS_LOCKED     (1 << 2)
/** @brief Root HUB/device identifier.  */
#define	USB_DEV_PARENT_ROOT       ((uint8_t) -1)
/** @} Constants */

/**
 * @brief Host stack's possible states.
 *
 * This states are mostly circumstantial,  everything  from  powering  the  Vbus
 * lines to enumeration will be  handled  by  the  Host  through  each  device's
 * state-machine.
 */
typedef enum _usb_host_state_t
{
	USB_HOST_STATE_IDLE,      /**< Waiting for a device connection.           */
	USB_HOST_STATE_RUNNING,   /**< At least one device connected and running. */
	USB_HOST_STATE_SUSPENDED  /**< Bus inactive, waiting for activity.        */
} usb_host_state_t;

/** @brief USB stack structure.  */
typedef struct _usb_stack_t
{
	uint32_t         status;     /**< Stack status, see description.          */
	usb_host_state_t state;      /**< Stack's current state.                  */
	usb_device_t     devices[USB_MAX_DEVICES];  /**< Array of devices.        */
	uint16_t         ticks;      /**< 1-millisecond ticks count.              */
#if (USB_MAX_HUBS > 0)
	usb_hub_t        hubs[USB_MAX_HUBS];        /**< Array of HUBs.           */
	uint8_t          n_hubs;     /**< Number of connected HUBs.               */
#endif
	uint8_t          n_devices;  /**< Number of connected devices.            */
} usb_stack_t;


/**
* @name Constants
* @{ */
/** @brief Currently writing to a device on address 0. */
#define	USB_STACK_STATUS_ZERO_ADDR  (1 << 0)
/** @} Constants */


/** @brief USB driver callbacks structure. */
typedef struct _usb_driver_t
{
	uint16_t  vendor_ID;
	/**< Only probe devices that match this vendor ID, 0xFFFF to force. */

	uint16_t  product_ID;
	/**< Only probe devices that match this product ID, 0xFFFF to force. */

	int (*probe) ( const uint8_t* buffer, const uint8_t len );
	/**< Probing function, to determine driver compatibility with interface. */

	int (*assign)( usb_stack_t* pstack, const uint16_t id );
	/**< Assignment function to bind interface to driver.                    */

	int (*remove)( usb_stack_t* pstack, const uint16_t id );
	/**< Remove and unbind interface from driver.                            */
} usb_driver_t;

#define USB_FORCE_PROBING_ID  0xFFFF

#define	USB_ID_TO_DEV(id)     ((id) >> 8)
#define	USB_ID_TO_IFACE(id)   ((id) & 0xFF)
#define	USB_TO_ID(dev, iface) (((dev) << 8) | (iface))


/* Validate constant definitions */
#if (USB_MAX_DEVICES < 1) || (USB_MAX_DEVICES > 127)
# error "USB_MAX_DEVICES must be greater than zero and less than 128."
#endif
#if (USB_MAX_INTERFACES < 1) || (USB_MAX_INTERFACES > 127)
# error "USB_MAX_INTERFACES must be greater than zero and less than 128."
#endif
#if (USB_MAX_ENDPOINTS < 1)
# error "USB_MAX_ENDPOINTS must be greater than zero."
#endif
#if (USB_MAX_HUBS < 0) || (USB_MAX_HUBS > 126)
# error "USB_MAX_HUBS must be greater than or equal to zero and less than 127."
#endif
#if (USB_MAX_HUB_PORTS < 1) || (USB_MAX_HUB_PORTS > 126)
/** @FIXME: fix the max. number of hub ports, I'm sure it's not 126 ! */
# error "USB_MAX_HUB_PORTS must be greater than zero and less than 127."
#endif
#if (USB_MAX_HUBS == 0) && (USB_MAX_DEVICES > 1)
# error "Cannot support more than one device without HUB support."
#endif

/**  @} USB */
#endif  /* __USB_H__ */
