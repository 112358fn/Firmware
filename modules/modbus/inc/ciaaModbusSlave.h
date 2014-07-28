/* Copyright 2014, Mariano Cerdeiro
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

#ifndef _CIAAMODBUS_H_
#define _CIAAMODBUS_H_
/** \brief Modbus Slave Header File
 **
 ** This files shall be included by moodules using the interfaces provided by
 ** the Modbus Slave
 **
 **/

/** \addtogroup CIAA_Firmware CIAA Firmware
 ** @{ */
/** \addtogroup Modbus CIAA Modbus
 ** @{ */

/*
 * Initials     Name
 * ---------------------------
 * MaCe         Mariano Cerdeiro
 * GMuro        Gustavo Muro
 *
 */

/*
 * modification history (new versions first)
 * -----------------------------------------------------------
 * 20140623 v0.0.1 initials initial
 */

/*==================[inclusions]=============================================*/
#include "ciaaPOSIX_stdint.h"
#include "ciaaModbus_Cfg.h"

/*==================[cplusplus]==============================================*/
#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/
/** \brief No error */
#define MODBUS_E_OK                       0x00

/** \brief Function not supported error */
#define MODBUS_E_FUNC_NOT_SUPPORTED       0x01

/** \brief Invalid address error */
#define MODBUS_E_INV_ADDRESS              0x02

/** \brief Invalid length error */
#define MODBUS_E_INV_LENGHT               0x03

/** \brief Function internal error */
#define MODBUS_E_FUNCTION_ERROR           0x04

/****** error codes ******/
#define CIAAMODBUS_E_FNC_NOT_SUPPORTED    0x01
#define CIAAMODBUS_E_WRONG_STR_ADDR       0x02
#define CIAAMODBUS_E_WRONG_REG_QTY        0x03
#define CIAAMODBUS_E_FNC_ERROR            0x04

/****** functions ******/
#define CIAAMODBUS_FCN_READHOLDINGREGISTERS     0x03
#define CIAAMODBUS_FCN_READINPUTREGISTERS       0x04
#define CIAAMODBUS_FCN_WRITESINGLEREGISTER      0x06
#define CIAAMODBUS_FCN_WRITEMULTIPLEREGISTERS   0x10



/** \brief Lenght of a modbus pdu for function 0x04 Read Input Registers */
#define CIAAMODBUS_MSG_READ_INPUT_REGISTERS_LENGTH             0x05

/*==================[typedef]================================================*/
/** \brief Modbus return type */
typedef uint8_t Modbus_returnType;

/*==================[external data declaration]==============================*/
/** \brief Modbus initialization
 **
 ** \param[in] dev device used for the modbus reception/transmission
 **/
extern void ciaaModbus_init(char * dev);

/** \brief Modbus deinitialization
 **/
extern void ciaaModbus_deinit(void);

/** \brief Process modbus request
 **
 ** \param[inout] buf buffer with the modbus data
 ** \param[in] len length of the buffer
 ** \return length of data on the buffer
 **/
extern int32_t ciaaModbus_process(uint8_t * buf, int32_t len);

/** \brief Modbus slave main function
 **/
extern void ciaaModbus_slaveMainTask(void);

/** \brief Command Read Holding Registers
 **
 ** \param[inout] buf buffer with the modbus pdu
 ** \param[in] len lenght of the buffer
 ** \returns count of bytes to be answered
 **/
int32_t ciaaModbus_readHoldingRegisters(uint8_t * buf, int32_t len);

/** \brief Command Read Input Registers
 **
 ** \param[inout] buf buffer with the modbus pdu
 ** \param[in] len lenght of the buffer
 ** \returns count of bytes to be answered
 **/
int32_t ciaaModbus_readInputRegisters(uint8_t * buf, int32_t len);

/** \brief Command Write Single Register
 **
 ** \param[inout] buf buffer with the modbus pdu
 ** \param[in] len lenght of the buffer
 ** \returns count of bytes to be answered
 **/
int32_t ciaaModbus_writeSingleRegisters(uint8_t * buf, int32_t len);

/** \brief Command Write Multiple Register
 **
 ** \param[inout] buf buffer with the modbus pdu
 ** \param[in] len lenght of the buffer
 ** \returns count of bytes to be answered
 **/
int32_t ciaaModbus_writeMultipleRegisters(uint8_t * buf, int32_t len);

/*==================[external functions declaration]=========================*/

/** \brief Read Integer from modbus
 **
 ** As described in modbus specification, the modbus uses a bigendian format to
 ** transmit integers. This function shall be used to access integers.
 **
 ** \param[in] add address of the first byte of the integer to be read.
 **
 **/
#define ciaaModbus_readInt(add) \
   ( (((uint8_t*)(add))[0] << 8) | (((uint8_t*)(add))[1]))


/** \brief Write Integer in modbus buffer
 **
 ** As described in modbus specification, the modbus uses a bigendian format to
 ** transmit integers. This function shall be used to access integers.
 **
 ** \param[in] add address of the first byte of the integer to be write.
 ** \param[in] num integer to be write.
 **
 **/
#define ciaaModbus_writeInt(add,num) \
   do {                                                        \
      (((uint8_t*)(add))[0] = ((uint16_t)(num) >> 8) & 0XFF);  \
      (((uint8_t*)(add))[1] = ((uint16_t)(num) >> 0) & 0XFF);  \
   }while (0)

/*==================[cplusplus]==============================================*/
#ifdef __cplusplus
}
#endif
/** @} doxygen end group definition */
/** @} doxygen end group definition */
/*==================[end of file]============================================*/
#endif /* #ifndef _CIAAMODBUS_H_ */

