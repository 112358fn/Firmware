/* Copyright 2016, Alvaro Alonso Bivou <alonso.bivou@gmail.com>
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

/** \brief Simon Says game implementation for EDU-CIAA
 **
 ** This is the Simon Says game. The light sequence must be 
 ** replicated by the user pressing the buttons.
 **/

/** \addtogroup CIAA_Firmware CIAA Firmware
 ** @{ */
/** \addtogroup Examples CIAA Firmware Examples
 ** @{ */
/** \addtogroup Games Simon example source file
 ** @{ */

/*
 * Initials     Name
 * ---------------------------
 * AA         Alvaro Alonso
 */

 /*
 * modification history (new versions first)
 * -----------------------------------------------------------
 * 20160607 v0.0.1   AA   first functional version
 */

/*==================[inclusions]=============================================*/
#include "SimonSays.h"

/*==================[macros and definitions]=================================*/
#define FLANCO(TECLA) ((inputs_now ^ inputs_old) & (inputs_now & (TECLA)))
/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/
/** \brief File descriptor for digital output ports
 *
 * Device path /dev/dio/out/0
 */
static int32_t leds_fd;
/** \brief File descriptor for digital input ports
 *
 * Device path /dev/dio/in/0
 */
static int32_t buttons_fd;
/** \brief Current state of the States Machine
 * Initial State or state after reset is IDLE
 * IDLE: The machine is waiting for an input 
 */
static states state = IDLE;
/** \brief Array of the sequence to be displayed by the leds
 * Initial State or state after reset is IDLE
 * IDLE: The machine is waiting for an input 
 */
static uint8_t sequence[]={1, 2, 0, 1, 3, 3, 1, 2, 1, 0, 1, 1, 0, 1, 2, 2, 3, 3, 1, 1, 2, 1, 1, 3, 3, 3, 1, 0, 2, 1, 0, 0, 3, 0, 2, 1, 3, 2, 0, 3, 2, 0, 2, 2, 0, 2, 2, 1, 0, 0, 3, 2, 2, 2, 2, 1, 2, 2, 2, 0, 2, 2, 3, 2, 2, 1, 3, 2, 3, 2, 3, 3, 1, 1, 2, 0, 0, 0, 2, 1, 1, 2, 1, 3, 2, 0, 3, 3, 3, 2, 3, 3, 0, 0, 0, 0, 2, 3, 0, 1};
static uint8_t leds[] = {LED_B, LED_1, LED_2, LED_3};
static int level=1;

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/
/** \brief Main function
 *
 * This is the main entry point of the software.
 *
 * \returns 0
 *
 * \remarks This function never returns. Return value is only to avoid compiler
 *          warnings or errors.
 */
int main(void)
{
   /* Starts the operating system in the Application Mode 1 */
   /* This example has only one Application Mode */
   StartOS(Normal);

   /* StartOs shall never returns, but to avoid compiler warnings or errors
    * 0 is returned */
   return 0;
}

/** \brief Error Hook function
 *
 * This fucntion is called from the os if an os interface (API) returns an
 * error. Is for debugging proposes. If called this function triggers a
 * ShutdownOs which ends in a while(1).
 *
 * The values:
 *    OSErrorGetServiceId
 *    OSErrorGetParam1
 *    OSErrorGetParam2
 *    OSErrorGetParam3
 *    OSErrorGetRet
 *
 * will provide you the interface, the input parameters and the returned value.
 * For more details see the OSEK specification:
 * http://portal.osek-vdx.org/files/pdf/specs/os223.pdf
 *
 */
void ErrorHook(void)
{
   ShutdownOS(0);
}

/** \brief Initial task
 *
 * This task is started automatically in the application mode Normal.
 */
TASK(InitTask)
{
   /* init CIAA kernel and devices */
   ciaak_start();
   /* open CIAA digital outputs & inputs */
   leds_fd = ciaaPOSIX_open(SALIDAS, ciaaPOSIX_O_RDWR);
   buttons_fd = ciaaPOSIX_open(ENTRADAS, ciaaPOSIX_O_RDONLY);
   /* activate periodic task:
    *  - InputTask: it updates the user inputs.
    *  - OutputTask: it updates the game outputs.
    */
   SetRelAlarm(ActivateInputTask, START_DELAY_MS, REFRESH_RATE_INPUT_MS);
   SetRelAlarm(ActivateOutputTask, START_DELAY_MS, REFRESH_RATE_OUTPUT_MS);
   /* terminate task */
   TerminateTask();
}

/** \brief Input Task
 *
 * This task is started automatically every time that the alarm
 * ActivateInputTask expires: every REFRESH_RATE_INPUT_MS.
 * This is a polling task for monitoring buttons states
 *
 */
TASK(InputTask)
{
   uint8_t inputs_now, outputs;
   static uint8_t inputs_old;
   static uint8_t index=0;

   /* refresh buttons states */
   ciaaPOSIX_read(buttons_fd, &inputs_now, 1);
   /* inverse button logic */
   inputs_now = ~inputs_now;
   /* light led pressed */
   outputs = (inputs_now<<2);
   ciaaPOSIX_write(leds_fd, &outputs, 1);
   /* States Machine */
   switch(state)
      {
      /* IDLE State: it is waiting for an user inputs*/
      case IDLE:
         if(FLANCO(TECLA_1|TECLA_2|TECLA_3|TECLA_4))
            state = START;
         break;
      /* Listen State: it checks if the user input is correct or not */
      case LISTEN:
         if(FLANCO(TECLA_1|TECLA_2|TECLA_3|TECLA_4))
         {
            if(((sequence[index]==0) && FLANCO(TECLA_1)) || //
            ((sequence[index]==1) && FLANCO(TECLA_2)) || //
            ((sequence[index]==2) && FLANCO(TECLA_3)) || //
            ((sequence[index]==3) && FLANCO(TECLA_4)))
            {
               index++;
               state = (index == level) ? INCREASE : LISTEN;
            }
            else
            {
               index = 0;
               state = FAIL;
            }
         }
         break;
      /* Increase State: it moves the game to the next level */
      case INCREASE:
         index = 0;
         level = (level < sizeof(sequence)) ? (level+1) : 1;
         state = SEQUENCE;
        break;
      default:
      break;
   }
   /* store the buttons state */
   inputs_old = inputs_now;
   /* terminate task */
   TerminateTask();
}

/** \brief Output Task
 *
 * This task is started automatically every time that the alarm
 * ActivateInputTask expires: every REFRESH_RATE_OUTPUT_MS.
 * This is a refresh task for updating leds states
 *
 */
TASK(OutputTask)
{
   uint8_t outputs=0x00;
   static uint8_t index=0;

   /* States Machine */
   switch(state)
      {
      /* Start State: It flash all the leds to show the game has started */
      case START:
         outputs = (leds[0]|leds[1]|leds[2]|leds[3]);
         state = SEQUENCE;
         break;
      /* Sequence State: Flash a single led that correspond according to 
       *                 the sequence */
      case SEQUENCE:
         outputs = leds[sequence[index]];
         // or
         //outputs = LED_RGB_AZUL<<sequence[index];
         index ++;
         if(index == level){
            state =LISTEN;
            index = 0;
         }
        break;
      /* Fail State: It flash all the leds showing the game has ended */
      case FAIL:
         outputs = (leds[0]|leds[1]|leds[2]|leds[3]);
         state = IDLE;
         level = 0;
         index = 0; 
         break;
      default:
      break;
   }
   /* update leds states */
   ciaaPOSIX_write(leds_fd, &outputs, 1);
   /* terminate task */
   TerminateTask();
}

/** @} doxygen end group definition */
/** @} doxygen end group definition */
/** @} doxygen end group definition */
/*==================[end of file]============================================*/
