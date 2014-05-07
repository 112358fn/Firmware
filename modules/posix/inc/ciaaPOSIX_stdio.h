/* Copyright 2014, ACSE & CADIEEL
 *    ACSE   : http://www.sase.com.ar/asociacion-civil-sistemas-embebidos/ciaa/
 *    CADIEEL: http://www.cadieel.org.ar
 *
 * This file is part of CIAA Firmware.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef _CIAAPOSIX_STDIO_H_
#define _CIAAPOSIX_STDIO_H_
/** \brief ciaa POSIX stdio header file
 **
 ** ciaa POSIX stdio header file
 **
 **/

/** \addtogroup CIAA_Firmware CIAA Firmware
 ** @{ */
/** \addtogroup POSIX POSIX Implementation
 ** @{ */

/*
 * Initials     Name
 * ---------------------------
 * EzEs         Ezequiel Esposito
 */

/*
 * modification history (new versions first)
 * -----------------------------------------------------------
 * 20140422 v0.0.2 EzEs initial version
 * 20140420 v0.0.1 EzEs initial version
 */

/*==================[inclusions]=============================================*/
#include "ciaaDevices.h"
#include "ciaaMemory.h"
#include "ciaaErrorsCodeSystem.h"
#include "ciaaMessagesCodeSystem.h"

#include "ciaaUART.h"

/*==================[cplusplus]==============================================*/
#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/
/** \brief Max devices available
 **/
#define ciaaPOSIX_MAX_DEVICES 				100

/** \brief Open for read only */
#define O_RDONLY            1

/** \brief Open to write only */
#define O_WRONLY            2

/** \brief Open to read write */
#define O_RDWR              4

/*==================[typedef]================================================*/
/** \brief TODO
 **
 **/
typedef struct
{
	int32_t fd;
	char * const name;
	ciaaDevices_Enum_Status status;
	int32_t (*pOpen) (const char* pathName, int32_t flags);
	int32_t (*pClose) (int32_t fd);
	int32_t (*pIoctl) (int32_t fd, int32_t arg, void* param);
	int32_t (*pRead) (int32_t fd, uint8_t * const buffer, uint32_t size);
	int32_t (*pWrite) (int32_t fd, uint8_t const * const buffer, uint32_t size);
	void* data;
} ciaaPOSIX_Type_Base;

/** \brief ciaaPOSIX errors enum
 ** Minimum allowed number: -1
 ** Maximum allowed number: -1000
 **/
typedef enum
{
	ciaaPOSIX_Enum_Errors_DeviceAlreadyOpen = ciaaPOSIX_MINERRORCODE,
	ciaaPOSIX_Enum_Errors_DeviceNotAllocated = ciaaPOSIX_MINERRORCODE - 1,
	ciaaPOSIX_Enum_Errors_BadFileDescriptor = ciaaPOSIX_MINERRORCODE - 2
} ciaaPOSIX_Enum_Errors;

/** \brief ciaaPOSIX return message codes
 ** Minimum allowed number: 1
 ** Maximum allowed number: 1000
 **/
typedef enum
{
	ciaaPOSIX_Enum_Messages_Example = ciaaPOSIX_MACRO_MinMessageCode,
} ciaaPOSIX_Enum_Messages;

/*==================[external data declaration]==============================*/
/** \brief List of posix devices
 **
 **/
extern ciaaPOSIX_Type_Base* ciaaPOSIX_devicesArray [];
extern uint32_t ciaaPOSIX_devicesArraySize;

/*==================[external functions declaration]=========================*/
/** \brief ciaaPOSIX Initialization
 **
 ** Performs the initialization of the ciaaPOSIX
 **
 **/
extern void ciaaPOSIX_init(void);

/** \brief Open a file
 **
 ** Opens a file or device path for read/write/readwrite depending on oflag.
 **
 ** \param[in] 	  path path of the device to be opened
 ** \param[in]    oflag may take one of the following values:
 **               O_RDONLY: opens files to read only
 **               O_WRONLY: opens files to write only
 **               O_RDWR: opens file to read and write
 ** \return       a negative value if failed, a positive
 **               value representing the file handler if success.
 **/
extern int32_t ciaaPOSIX_open(char const * const path, uint8_t const oflag);

/** \brief Close a file descriptor
 **
 ** Closes the file descriptor fildes
 **
 ** \param[in]  fildes file descriptor to be closed
 ** \return     a negative value if failed, a positive value
 **             if success.
 **/
extern int32_t ciaaPOSIX_close (int32_t const fildes);

/** \brief Control a stream device
 **
 ** Performs special control of a stream device
 **
 ** \param[in]  fildes file descriptor to be closed
 ** \param[in]  request type of the request, depends on the device
 ** \param[in]	param
 ** \return     a negative value if failed, a positive value
 **             if success.
 **/
extern int32_t ciaaPOSIX_ioctl (int32_t const fildes, int32_t request, void* param);

/** \brief Reads from a file descriptor
 **
 ** Reads nbyte from the file descriptor fildes and store them in buf.
 **
 ** \param[in]  fildes  file descriptor to read from
 ** \param[out] buf     buffer to store the read data
 ** \param[in]  nbyte   count of bytes to be read
 ** \return     the count of read bytes is returned
 **
 **/
extern int32_t ciaaPOSIX_read (int32_t const fildes, uint8_t * const buf, uint32_t nbyte);

/** \brief Writes to a file descriptor
 **
 ** Writes nbyte to the file descriptor fildes from the buffer buf
 **
 ** \param[in]  fildes  file descriptor to write to
 ** \param[in]  buf     buffer with the data to be written
 ** \param[in]  nbyte   count of bytes to be written
 ** \return     the count of bytes written
 **/
extern int32_t ciaaPOSIX_write (int32_t const fildes, uint8_t const * const buf, uint32_t nbyte);

/*==================[cplusplus]==============================================*/
#ifdef __cplusplus
}
#endif
/** @} doxygen end group definition */
/** @} doxygen end group definition */
/*==================[end of file]============================================*/
#endif /* #ifndef _CIAAPOSIX_H_ */

