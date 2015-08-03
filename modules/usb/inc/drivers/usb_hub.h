#ifndef USB_HUB_H
#define USB_HUB_H
/**
* @addtogroup USB_HUB
* @brief CIAA HUB driver
*
* @{ */

#include <stdint.h>
#include "usb.h"


/** @brief Basic HUB device structure. */
typedef struct _usb_hub_t
{
   uint32_t status;   /**< HUB's status                 */
   uint8_t  n_ports;  /**< Total number of ports on HUB */
   /** TODO: fill this up */
} usb_hub_t;


/**
 * @brief Driver registration probing function.
 *
 * @param buffer  Buffer containing the entire interface descriptor.
 * @param len     Buffer's length.
 */
int hub_probe( const uint8_t* buffer, const uint8_t len );
/**
 * @brief Driver registration assignment function.
 *
 * This method assumes you previously probed the interface with hid_probe,  this
 * means it will \b not check  whether  it's  the  correct  descriptor  and  its
 * characteristics, only what wasn't validated before.
 *
 * @param buffer  Buffer containing the entire interface descriptor.
 * @param len     Buffer's length.
 */
int hub_assign( usb_stack_t* pstack, const uint16_t id );
/**
 * @brief Driver registration removal function.
 *
 * @param pstack Pointer to the USB stack this device belongs to.
 * @param id     Device's unique identifier within the specified USB stack.
 */
int hub_remove( usb_stack_t* pstack, const uint16_t id );


/**
 * @brief Update HUB's state.
 * @param phub Pointer to HUB structure.
 */
int usb_hub_update( usb_hub_t* phub );


/** @FIXME
 * Everything below this point was sketched before even starting implementing
 * the stack, some things have changed a little so it will have to be revised.
 */

/**
 * @brief Check whether the HUB port is active.
 *
 * A HUB port is active when a device has been detected on it  and  powered  up.
 * This is used to tell whether a connection (usb_hub_is_connected()) is  a  new
 * event or just the same device not having been yet disconnected.
 *
 * @param phub Pointer to HUB structure.
 * @param port Port number.
 * @return Non-zero if port is active.
 */
int usb_hub_is_active( usb_hub_t* phub, uint8_t port );

/**
 * @brief Check whether a device has been connected on the specified port.
 * @param phub Pointer to HUB structure.
 * @param port Port number.
 * @return Non-zero if a device is connected.
 */
int usb_hub_is_connected( usb_hub_t* phub, uint8_t port );

/**
 * @brief Power up HUB's specified port.
 * @param phub Pointer to HUB structure.
 * @param port Port number.
 * @todo Write return values.
 */
int usb_hub_poweron( usb_hub_t* phub, uint8_t port );

/**
 * @brief Power off HUB's specified port.
 * @param phub Pointer to HUB structure.
 * @param port Port number.
 * @todo Write return values.
 */
int usb_hub_poweroff( usb_hub_t* phub, uint8_t port );

/**
 * @brief Start driving a USB reset on the specified port.
 * @param phub Pointer to HUB structure.
 * @param port Port number.
 * @todo Write return values.
 */
int usb_hub_begin_reset( usb_hub_t* phub, uint8_t port );

/**
 * @brief Stop driving a USB reset on the specified port.
 * @param phub Pointer to HUB structure.
 * @param port Port number.
 * @todo Write return values.
 */
int usb_hub_end_reset( usb_hub_t* phub, uint8_t port );

/**
 * @brief Get speed of device attached to specified port.
 * @param phub Pointer to HUB structure.
 * @param port Port number.
 */
usb_speed_t usb_hub_get_speed( usb_hub_t* phub, uint8_t port );


/**  @} USB_HUB */
#endif /* USB_HUB_H */
