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

/** \brief Blinking example source file
 **
 ** This is a mini example of the CIAA Firmware
 **
 **/

/** \addtogroup CIAA_Firmware CIAA Firmware
 ** @{ */
/** \addtogroup Examples CIAA Firmware Examples
 ** @{ */
/** \addtogroup Blinking Blinking example source file
 ** @{ */

/*
 * Initials     Name
 * ---------------------------
 *
 */

/*
 * modification history (new versions first)
 * -----------------------------------------------------------
 * yyyymmdd v0.0.1 initials initial version
 */

/*==================[inclusions]=============================================*/
#include "os.h"
#include "ciaaPOSIX_stdio.h"
#include "ciaaModbusSlave.h"
#include "ciaak.h"
#include "blinking.h"

/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/
int32_t blinkingModbus_ledFildes;
uint16_t blinkingModbus_modbusRegister;

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/
int main(void)
{
   StartOS(AppMode1);
   return 0;
}

void ErrorHook(void)
{
   ciaaPOSIX_printf("ErrorHook was called\n");
   ciaaPOSIX_printf("Service: %d, P1: %d, P2: %d, P3: %d, RET: %d\n", OSErrorGetServiceId(), OSErrorGetParam1(), OSErrorGetParam2(), OSErrorGetParam3(), OSErrorGetRet());
   ShutdownOS(0);
}

TASK(InitTask) {
   /* init the ciaa kernel */
   ciaak_start();

   /* open led device */
   blinkingModbus_ledFildes = ciaaPOSIX_open("/dev/dio/port/0",O_RDWR);

   /* terminate task */
   TerminateTask();
}

TASK(Blinking) {
   static uint8_t ledStatus = 0;

   /* write led */
   ciaaPOSIX_write(blinkingModbus_ledFildes, &ledStatus, sizeof(ledStatus));
   ciaaPOSIX_printf("LED: %d\n", ledStatus);
   /* toggle bit 0 */
   ledStatus ^= 0x01;

   /* terminate task */
   TerminateTask();
}

TASK(ModbusSlave)
{
   /* initialize modbus slave */
   ciaaModbus_init();

   /* start modbus main task */
   ciaaModbus_slaveMainTask();

   /* terminate task */
   TerminateTask();
}


int8_t readInputRegisters(
      uint16_t startingAddress,
      uint16_t quantityOfInputRegisters,
      uint8_t * exceptionCode,
      uint8_t * buf
      )
{
   int8_t ret;

   if ( (0x0000 == startingAddress) &&
        (0x01 == quantityOfInputRegisters) )
   {
      buf[0] = blinkingModbus_modbusRegister >> 8;
      buf[1] = blinkingModbus_modbusRegister & 0xFF;
      ret = 1;
   }
   else
   {
      *exceptionCode = CIAAMODBUS_E_WRONG_STR_ADDR;
      ret = -1;
   }

   return ret;
}

int8_t writeSingleRegister(
      uint16_t registerAddress,
      uint16_t registerValue,
      uint8_t * exceptionCode
      )
{
   int8_t ret;

   if (0x0000 == registerAddress)
   {
      blinkingModbus_modbusRegister = registerValue;
      ret = 1;
   }
   else
   {
      *exceptionCode = CIAAMODBUS_E_WRONG_STR_ADDR;
      ret = -1;
   }


   return ret;
}

/** @} doxygen end group definition */
/** @} doxygen end group definition */
/** @} doxygen end group definition */
/*==================[end of file]============================================*/

