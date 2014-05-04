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

/** \brief CIAA Devices source file
 **
 ** This header file describes the Devices.
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
 * MaCe			 Mariano Cerdeiro
 */

/*
 * modification history (new versions first)
 * -----------------------------------------------------------
 * 20140420 v0.0.1 EzEs initial version
 * 20140503 v0.0.2 MaCe implement all functions
 */

/*==================[inclusions]=============================================*/
#include "ciaaDevices.h"
#include "ciaaPOSIX_semaphore.h"
#include "ciaaPOSIX_stdlib.h"
#include "ciaaPOSIX_stdbool.h"
#include "ciaaPOSIX_string.h"

/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/
typedef struct {
	ciaaDevice_deviceType const * device[ciaaDEVICES_MAXDEVICES];
	uint8_t position;
} ciaaDevice_devicesType;

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/
/** \brief List of devices */
ciaaDevice_devicesType ciaaDevices;

/** \brief ciaa Device sempahore */
sem_t ciaaDevice_sem;

/*==================[external data definition]===============================*/
/** \brief UART1 Device
 **
 **/
char * ciaaDevices_UART1 = "/dev/UART/UART1";

/** \brief UART2 Device
 **
 **/
char * ciaaDevices_UART2 = "/dev/UART/UART2";

/** \brief I2C1  Device
 **
 **/
char * ciaaDevices_I2C1 = "/dev/I2C/I2C1";

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/
extern void ciaaDevice_init(void)
{
	/* reset position of the devices */
	ciaaDevices.position = 0;
	
	/* init sempahore */
	sem_init(&ciaaDevice_sem);
}

extern void ciaaDevice_addDevice(ciaaDevice_deviceType const * device)
{
	/* enter critical section */
	sem_wait(&ciaaDevice_sem);
	
	/* store the device in the list */
	ciaaDevices.device[ciaaDevices.position] = device;
	
	/* increment the device position */
	ciaaDevices.position++;
	
	/* exit critical section */
	sem_post(&ciaaDevice_sem);
}

extern ciaaDevice_deviceType const * ciaaDevice_getDevice(char const * const path)
{
	bool found = false;
	ciaaDevice_deviceType const * ret = NULL;
	uint8_t device;
	
	/* search over all devices */
	for(device = 0; (device < ciaaDevices.position) && !found; device++) {
		/* if the same path is found */
		if (ciaaPOSIX_strcmp(path, ciaaDevices.device[device]->path)) {
			/* return the device */
			ret = ciaaDevices.device[device];
			
			/* break the for */
			found = true;
		}	
	}
	
	return ret;
}

/** @} doxygen end group definition */
/** @} doxygen end group definition */
/*==================[end of file]============================================*/

