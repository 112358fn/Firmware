#ifndef USB_HUB_H
#define USB_HUB_H

/**
 * @addtogroup CIAA_Firmware CIAA Firmware
 * @{
 * @addtogroup USB USB Stack
 * @{
 * @addtogroup USB_DRV USB Drivers
 * @{
 * @addtogroup USB_HUB USB HUB
 * @brief CIAA HUB driver header
 *
 * @{
 */


/*==================[inclusions]=============================================*/
#include <stdint.h>
#include "usb.h"


/*==================[cplusplus]==============================================*/
#ifdef __cplusplus
extern "C" {
#endif


/*==================[macros]=================================================*/


/** @brief HUB transactions buffer maximum length. */
#define USB_HUB_BUFF_LEN   128


/** @brief USB HUB descriptor type. */
#define USB_HUB_DESC_TYPE  0x29


/**
 * @name HUB descriptor
 * @{
 */

/** @brief USB HUB descriptor's minimum size, depends on number of ports. */
#define USB_HUB_DESC_SIZE  9
/** @brief Calculate actual HUB descriptor's size based on number of ports N. */
#define USB_HUB_DESC_REAL_SIZE(N) \
   (USB_HUB_DESC_SIZE + 2 * ((N) >> 3))


/** @brief Get HUB descriptor's bDescLength from buffer x. */
#define USB_HUB_DESC_GET_bDescLength(x) \
   ((uint8_t ) (x)[0])
/** @brief Get HUB descriptor's bDescriptorType from buffer x. */
#define USB_HUB_DESC_GET_bDescriptorType(x) \
   ((uint8_t ) (x)[1])
/** @brief Get HUB descriptor's bNbrPorts from buffer x. */
#define USB_HUB_DESC_GET_bNbrPorts(x) \
   ((uint8_t ) (x)[2])
/** @brief Get HUB descriptor's wHubCharacteristics from buffer x. */
#define USB_HUB_DESC_GET_wHubCharacteristics(x) \
   USB_ARCH_ENDIANNESS16((uint16_t) (((x)[4] << 8) | (x)[3]))
/** @brief Get HUB descriptor's bPwrOn2PwrGood from buffer x. */
#define USB_HUB_DESC_GET_bPwrOn2PwrGood(x) \
   ((uint8_t ) (x)[5])
/** @brief Get HUB descriptor's bHubContrCurrent from buffer x. */
#define USB_HUB_DESC_GET_bHubContrCurrent(x) \
   ((uint8_t ) (x)[6])
/**
 * @brief Get HUB descriptor's DeviceRemovable from buffer x and  port n.  Ports
 * identifier n starts at 0.
 */
#define USB_HUB_DESC_GET_DeviceRemovable(x, n) \
   ((uint8_t ) ((((x)[7 + (((n)+1) >> 3)]) >> (((n)+1) % 8)) & 0x01))
/**
 * @brief Get HUB descriptor's PortPwrCtrlMask from buffer x and port n.
 * @param  x  Buffer.
 * @param  N  Total number of ports.
 * @param  n  Port number, starting from 0.
 */
#define USB_HUB_DESC_GET_PortPwrCtrlMask(x, N, n) \
   ((uint8_t ) ((((x)[8 + (((N)+1) >> 3) + (((n)+1) >> 3)]) >> (((n)+1) % 8)) & 0x01))


/** @brief Logical Power Switching Mode mask (before shift). */
#define USB_HUB_LPSM_MASK  0x0003
/** @brief Logical Power Switching Mode shift. */
#define USB_HUB_LPSM_SHIFT 0

/** @brief Compound device identification bit mask. */
#define USB_HUB_CDI        (1 << 2)

/** @brief Over-Current Protection Mode mask (before shift). */
#define USB_HUB_OCPM_MASK  0x0018
/** @brief Over-Current Protection Mode shift. */
#define USB_HUB_OCPM_SHIFT 3

/** @brief TT Think Time mask (before shift). Unused. */
#define USB_HUB_TTTT_MASK  0x0060
/** @brief TT Think Time shift. Unused. */
#define USB_HUB_TTTT_SHIFT 5

/** @brief Port Indicators Supported bit mask. */
#define USB_HUB_PIS        (1 << 7)

/** @} HUB descriptor */


/**
 * @name HUB status field
 * @{
 */

/** @brief HUB status length. */
#define USB_HUB_STATUS_LENGTH    4

/** @brief Get HUB status field. */
#define USB_HUB_STATUS_GET_wHubStatus(x) \
   USB_ARCH_ENDIANNESS16((uint16_t) (((x)[1] << 8) | (x)[0]))
/**
 * @brief Get HUB status change field:
 *    .- USB_HUB_C_HUB_LOCAL_POWER
 *    .- USB_HUB_C_HUB_OVER_CURRENT
 */
#define USB_HUB_STATUS_GET_wHubChange(x) \
   USB_ARCH_ENDIANNESS16((uint16_t) (((x)[3] << 8) | (x)[2]))

/** @brief Local Power Source bit mask. */
#define USB_HUB_STATUS_LPS         (1 << 0)
/** @brief Over-current bit mask. */
#define USB_HUB_STATUS_OVC         (1 << 1)

/** @brief Local Power Status (C_HUB_LOCAL_POWER). */
#define USB_HUB_STATUS_CHANGE_LPS  (1 << 0)
/** @brief Over-current (C_HUB_OVER_CURRENT). */
#define USB_HUB_STATUS_CHANGE_OVC  (1 << 1)

/** @} HUB status field */


/**
 * @name HUB port status field
 * @{
 */

/** @brief HUB port status length. */
#define USB_HUB_PORT_STATUS_LENGTH    4

/** @brief Get HUB port status field. */
#define USB_HUB_PORT_STATUS_GET_wPortStatus(x) \
   USB_ARCH_ENDIANNESS16((uint16_t) (((x)[1] << 8) | (x)[0]))
/** @brief Get HUB port status change field. */
#define USB_HUB_PORT_STATUS_GET_wPortChange(x) \
   USB_ARCH_ENDIANNESS16((uint16_t) (((x)[3] << 8) | (x)[2]))

/** @brief Current Connect Status (PORT_CONNECTION). */
#define USB_HUB_PORT_STATUS_CCS  (1 <<  0)
/** @brief Port Enable/Disabled (PORT_ENABLE). */
#define USB_HUB_PORT_STATUS_PED  (1 <<  1)
/** @brief Suspend. (PORT_SUSPEND). */
#define USB_HUB_PORT_STATUS_SUS  (1 <<  2)
/** @brief Over-current (PORT_OVER_CURRENT). */
#define USB_HUB_PORT_STATUS_OVC  (1 <<  3)
/** @brief Reset (PORT_RESET). */
#define USB_HUB_PORT_STATUS_RST  (1 <<  4)
/** @brief Port Power (PORT_POWER). */
#define USB_HUB_PORT_STATUS_PWR  (1 <<  8)
/** @brief Low-Speed Device Attached (PORT_LOW_SPEED). */
#define USB_HUB_PORT_STATUS_LSP  (1 <<  9)
/** @brief High-speed Device Attached (PORT_HIGH_SPEED). */
#define USB_HUB_PORT_STATUS_HSP  (1 << 10)
/** @brief Port Test Mode (PORT_TEST). */
#define USB_HUB_PORT_STATUS_PTM  (1 << 11)
/** @brief Port Indicator Control (PORT_INDICATOR). */
#define USB_HUB_PORT_STATUS_PIC  (1 << 12)

/** @brief Connect Status Change (C_PORT_CONNECTION). */
#define USB_HUB_PORT_STATUS_CHANGE_CSC  (1 << 0)
/** @brief Port Enable/Disable Change (C_PORT_ENABLE). */
#define USB_HUB_PORT_STATUS_CHANGE_PED  (1 << 1)
/** @brief Suspend Change (C_PORT_SUSPEND). */
#define USB_HUB_PORT_STATUS_CHANGE_SUS  (1 << 2)
/** @brief Over-Current Indicator Change (C_PORT_OVER_CURRENT). */
#define USB_HUB_PORT_STATUS_CHANGE_OVC  (1 << 3)
/** @brief Reset Change (C_PORT_RESET). */
#define USB_HUB_PORT_STATUS_CHANGE_RST  (1 << 4)

/** @} HUB port status field */


/**
 * @name HUB device
 * @{
 */

/** @brief Whether device is free or currently in use. */
#define USB_HUB_STATUS_FREE         (1 <<  0)
/** @brief Whether device has already been initialized. */
#define USB_HUB_STATUS_INIT         (1 <<  1)
/** @brief Whether device has been opened by user code. */
#define USB_HUB_STATUS_OPEN         (1 <<  2)
/** @brief Whether entry actions are to be executed.  */
#define USB_HUB_STATUS_REQUEST      (1 <<  3)
/** @brief Whether HUB supports individual port power switching. */
#define USB_HUB_STATUS_INDIV_POWER  (1 <<  4)
/** @brief Whether it is a compound device. */
#define USB_HUB_STATUS_COMPOUND_DEV (1 <<  5)
/** @brief Whether HUB reports individual port over-current protection. */
#define USB_HUB_STATUS_INDIV_OVC    (1 <<  6)
/** @brief Whether HUB reports global port over-current protection. */
#define USB_HUB_STATUS_GLOBAL_OVC   (1 <<  7)
/** @brief Whether HUB supports port indicators. */
#define USB_HUB_STATUS_INDICATORS   (1 <<  8)
/** @brief HUB's local power supply is good, only if self-powered.*/
#define USB_HUB_STATUS_LPS_GOOD     (1 <<  9)
/** @brief HUB is in an over-current condition, see @ref usb_hub_ocpm_t. */
#define USB_HUB_STATUS_OVC_ACTIVE   (1 << 10)

/** @} HUB device */


/**
 * @name HUB port
 * @{
 */

/** @brief Whether device is removable. */
#define USB_HUB_PORT_REM            (1 << 0)
/** @brief Whether a device is currently connected. */
#define USB_HUB_PORT_CONNECTION     (1 << 1)
/** @brief Whether port is currently enabled. */
#define USB_HUB_PORT_ENABLED        (1 << 2)
/** @brief Whether port is currently suspended. */
#define USB_HUB_PORT_SUSPENDED      (1 << 3)
/** @brief Whether port is currently in over-current condition. */
#define USB_HUB_PORT_OVC_ACTIVE     (1 << 4)
/** @brief Whether port is being held in the reset state. */
#define USB_HUB_PORT_RESET_ACTIVE   (1 << 5)
/** @brief Whether port is in the powered-off state. */
#define USB_HUB_PORT_POWERED_STATE  (1 << 6)
/** @brief Low-speed device attached. */
#define USB_HUB_PORT_LOWSPEED       (1 << 7)
/** @brief High-speed device attached. */
#define USB_HUB_PORT_HIGHSPEED      (1 << 8)

/** @} HUB port */


/*==================[typedef]================================================*/

/**
 * @name HUB descriptor
 * @{
 */
/**
 * @brief HUB descriptor data structure.
 *
 * Structure fields have been reordered to account for byte alignment.
 *
 * @warning This is \b not the complete descriptor because  it  has  a  variable
 * number of members depending on the number of ports on HUB.
 */
typedef struct _usb_hub_desc_hub_t
{
   uint16_t wHubCharacteristics; /**< See USB_HUB_CHAR_PARAMS.                */
   uint8_t  bDescLength;         /**< Number of bytes in this descriptor.     */
   uint8_t  bDescriptorType;     /**< Descriptor type: 0x29 for HUB.          */
   uint8_t  bNbrPorts;           /**< Number of downstream facing ports.      */
   uint8_t  bPwrOn2PwrGood;      /**< Power up sequence time, in 2 ms units.  */
   uint8_t  bHubContrCurrent;    /**< Maximum current requirements in mA.     */
   /* Following members depend on the number of ports for a given HUB. */
   uint8_t  DeviceRemovable;     /**< Indicated removable device attached on a
                                      one bit per port basis. This field has a
                                      one-byte granularity.                   */
   uint8_t  PortPwrCtrlMask;     /**< Should be set to all ones. It  might  be
                                      just safe to ignore ,  it ' s  here  for
                                      compatibility reasons with 1.0 devices. */
} usb_hub_desc_hub_t;


/** @brief Logical Power Switching Mode options. */
typedef enum _usb_hub_lpsm_t {
   USB_HUB_LPSM_GANGED = 0x00, /**< Ganged. All ports' power switching at once*/
   USB_HUB_LPSM_INDIV  = 0x01, /**< Individual port power switching.          */
   USB_HUB_LPSM_RES0   = 0x02, /**< Reserved. Only for 1.0 compliant HUBs.    */
   USB_HUB_LPSM_RES1   = 0x03, /**< Reserved. Only for 1.0 compliant HUBs.    */
} usb_hub_lpsm_t;

/** @brief Over-Current Protection Mode options. */
typedef enum _usb_hub_ocpm_t {
   USB_HUB_OCPM_GLOBAL = 0x00, /**< Global. OC conditions reported together.  */
   USB_HUB_OCPM_INDIV  = 0x01, /**< Individual. Individual OC status per port.*/
   USB_HUB_OCPM_NA0    = 0x02, /**< No over-current protection.               */
   USB_HUB_OCPM_NA1    = 0x03, /**< No over-current protection.               */
} usb_hub_ocpm_t;
/** @} HUB descriptor */

/**
 * @name HUB requests
 * @{
 */
/**
 * @brief USB HUB class request codes.
 * 
 * Any code not present in the following list is reserved.
 */
typedef enum _usb_hub_classreq_t
{
   USB_HUB_CLASSREQ_GET_STATUS      =  0,
   USB_HUB_CLASSREQ_CLEAR_FEATURE   =  1,
   USB_HUB_CLASSREQ_SET_FEATURE     =  3,
   USB_HUB_CLASSREQ_GET_DESCRIPTOR  =  6,
   USB_HUB_CLASSREQ_SET_DESCRIPTOR  =  7,
   USB_HUB_CLASSREQ_CLEAR_TT_BUFFER =  8,
   USB_HUB_CLASSREQ_RESET_TT        =  9,
   USB_HUB_CLASSREQ_GET_TT_STATE    = 10,
   USB_HUB_CLASSREQ_STOP_TT         = 11,
} usb_hub_classreq_t;

/**
 * @brief USB HUB class feature selectors.
 *
 * Those with *_C_HUB_ suffix should be sent to a HUB recipient, all the  others
 * to a port one.
 */
typedef enum _usb_hub_featsel_t
{
   USB_HUB_FEATURE_C_HUB_LOCAL_POWER   =  0,
   USB_HUB_FEATURE_C_HUB_OVER_CURRENT  =  1,
   USB_HUB_FEATURE_PORT_CONNECTION     =  0,
   USB_HUB_FEATURE_PORT_ENABLE         =  1,
   USB_HUB_FEATURE_PORT_SUSPEND        =  2,
   USB_HUB_FEATURE_PORT_OVER_CURRENT   =  3,
   USB_HUB_FEATURE_PORT_RESET          =  4,
   USB_HUB_FEATURE_PORT_POWER          =  8,
   USB_HUB_FEATURE_PORT_LOW_SPEED      =  9,
   USB_HUB_FEATURE_C_PORT_CONNECTION   = 16,
   USB_HUB_FEATURE_C_PORT_ENABLE       = 17,
   USB_HUB_FEATURE_C_PORT_SUSPEND      = 18,
   USB_HUB_FEATURE_C_PORT_OVER_CURRENT = 19,
   USB_HUB_FEATURE_C_PORT_RESET        = 20,
   USB_HUB_FEATURE_PORT_TEST           = 21,
   USB_HUB_FEATURE_PORT_INDICATOR      = 22,
} usb_hub_featsel_t;
/** @} HUB requests */

/**
 * @name HUB device
 * @{
 */
/** @brief HUB possible states. */
typedef enum _usb_hub_state_t
{
   USB_HUB_STATE_IDLE,
   USB_HUB_STATE_DESC,
   USB_HUB_STATE_HUB_STATUS,
   USB_HUB_STATE_PORT_STATUS,
   USB_HUB_STATE_RUNNING,
} usb_hub_state_t;

/**
 * @brief Basic HUB device structure.
 */
typedef struct _usb_hub_t
{
   usb_stack_t*    pstack;      /**< USB stack the device belongs to.         */
   usb_hub_state_t state;       /**< Device's state.                          */
   uint32_t        status;      /**< HUB's status                             */
   uint32_t        port_status[USB_MAX_HUB_PORTS]; /**< Ports' status.        */
   uint16_t        id;          /**< Device's ID within the USB stack.        */
   uint8_t         poweron_t;   /**< Power-on sequence duration, in 2ms units.*/
   uint8_t         power_req;   /**< Power requirements in mA.                */
   uint8_t         n_ports;     /**< Total number of ports on HUB.            */
   uint8_t         current_port;/**< Current port being queried.              */
   uint8_t         buffer[USB_HUB_BUFF_LEN]; /**< Transaction buffer length,
                                                  capped at USB_HUB_BUFF_LEN. */
   uint8_t         buffer_len;  /**< Transaction buffer length, capped at
                                     USB_HUB_BUFF_LEN.                        */
} usb_hub_t;
/** @} HUB device */

/**
 * @brief USB HUB devices' stack.
 *
 * This structure will hold all the HUB devices information and current state. A
 * USB HUB stack will be attached to just \b one  USB  stack,  this  means  that
 * every HUB within (hubs[]) will use the same USB stack when  issuing  requests
 * and such.
 */
typedef struct _usb_hub_stack_t
{
   usb_hub_t    hubs[USB_MAX_HUBS]; /**< USB HUB devices state and members.   */
   uint8_t      n_hubs;      /**< Number of HUBs currently connected.         */
} usb_hub_stack_t;


/*==================[external functions declaration]=========================*/

/**
 * @brief Driver registration probing function.
 *
 * @param buffer  Buffer containing the entire interface descriptor.
 * @param len     Buffer's length.
 */
int usb_hub_probe( const uint8_t* buffer, const uint8_t len );

/**
 * @brief Driver registration assignment function.
 *
 * This method assumes you previously probed the interface  with  usb_hub_probe,
 * this means it will \b not check whether it's the correct descriptor  and  its
 * characteristics, only what wasn't validated before.
 *
 * @param buffer  Buffer containing the entire interface descriptor.
 * @param len     Buffer's length.
 */

int usb_hub_assign(
      usb_stack_t*   pstack,
      uint16_t       id,
      const uint8_t* buffer,
      uint8_t        length
);

/**
 * @brief Driver registration removal function.
 *
 * @param pstack Pointer to the USB stack this device belongs to.
 * @param id     Device's unique identifier within the specified USB stack.
 */
int usb_hub_remove( usb_stack_t* pstack, const uint16_t id );

/**
 * @brief Initialize HUB stack.
 * @return Non-zero on error.
 */
int usb_hub_init( void );

/**
 * @brief Update HUBs' state and status.
 * @return Non-zero on error.
 */
int usb_hub_update( void );




/*
================================================================================
================================================================================
*/




/** @FIXME
 * Everything below this point was sketched before  even  starting  implementing
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
//usb_speed_t usb_hub_get_speed( usb_hub_t* phub, uint8_t port );
usb_speed_t usb_hub_get_speed( uint8_t hub_idx, uint8_t port );


/*==================[cplusplus]==============================================*/
#ifdef __cplusplus
}
#endif
/** @} USB_HUB */
/** @} USB_DRV */
/** @} USB */
/** @} CIAA_Firmware */
/*==================[end of file]============================================*/
#endif /* USB_HUB_H */

