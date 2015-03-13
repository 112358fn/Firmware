/* Copyright 2014, Daniel Cohen
 * Copyright 2014, Esteban Volentini
 * Copyright 2014, Matias Giori
 * Copyright 2014, Franco Salinas
 * Copyright 2015, Mariano Cerdeiro
 * All rights reserved.
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

/** \brief CIAA Flash Posix Driver
 **
 ** Simulated Flash Driver for Posix for testing proposes
 **
 **/

/** \addtogroup CIAA_Firmware CIAA Firmware
 ** @{ */
/** \addtogroup Drivers CIAA Drivers
 ** @{ */
/** \addtogroup Flash Flash Drivers
 ** @{ */

/*
 * Initials     Name
 * ---------------------------
 * DC           Daniel Cohen
 * EV           Esteban Volentini
 * MG           Matias Giori
 * FS           Franco Salinas
 * MaCe         Mariano Cerdeiro
 */

/*
 * modification history (new versions first)
 * -----------------------------------------------------------
 * 20150201 v0.0.2 MaCe first implementation of flash simulation
 * 20141006 v0.0.1 EV   first initial version
 */

/*==================[inclusions]=============================================*/
#include "ciaaDriverFlash.h"
#include "ciaaDriverFlash_Internal.h"
#include "ciaaBlockDevices.h"
#include "ciaaPOSIX_string.h"
#include "ciaaPOSIX_stddef.h"
#include "ciaaPOSIX_ioctl_block.h"
#include "ciaaBlockDevices.h"
#include <stdio.h>

/*==================[macros and definitions]=================================*/
#define CIAADRVFLASH_PAGESIZE       512
#define CIAADRVFLASH_PAGESCOUNT     64

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/
/** \brief Function to erase a defined number of blocks of flash
 *  \param start First block number to erase
 *  \param end Last block number to erase
 *  \return result of erase operation
 */
int32_t ciaaDriverFlash_blockErase(uint32_t start, uint32_t end);

/** \brief Function to write a block of flash
 * @param address Number of block to write
 * @param buffer Buffer with data to write
 * @param size Size of data buffer
 * @return Result of write operation
 */
int32_t ciaaDriverFlash_blockWrite(uint32_t address, uint8_t const * const data, uint32_t size);

/*==================[internal data definition]===============================*/
/* Constant with filename of storage file maped to Flash */
static const char ciaaDriverFlash_filename[] = CIAADRVFLASH_FILENAME;

static ciaaDevices_deviceType ciaaDriverFlash_device = {
   "hd/0",                          /** <= driver name */
   ciaaDriverFlash_open,            /** <= open function */
   ciaaDriverFlash_close,           /** <= close function */
   ciaaDriverFlash_read,            /** <= read function */
   ciaaDriverFlash_write,           /** <= write function */
   ciaaDriverFlash_ioctl,           /** <= ioctl function */
   ciaaDriverFlash_seek,            /** <= seek function is not provided */
   NULL,                            /** <= upper layer */
   (void*)&ciaaDriverFlash_flash,  /** <= layer */
   NULL                             /** <= NULL no lower layer */
};

/*==================[external data definition]===============================*/
/** \brief Flash 0 */
ciaaDriverFlash_flashType ciaaDriverFlash_flash;

/*==================[internal functions definition]==========================*/
int32_t ciaaDriverFlash_blockErase(uint32_t start, uint32_t end) {
   int32_t ret = -1;
   int32_t index;
   uint8_t buffer[CIAADRVFLASH_BLOCK_SIZE];
   ciaaDriverFlash_flashType * flash = &ciaaDriverFlash_flash;

   if ((start <= end) && (end < CIAADRVFLASH_BLOCK_CANT))
   {
      ciaaPOSIX_memset(buffer, 0xFF, CIAADRVFLASH_BLOCK_SIZE);
      fseek(flash->storage, start * CIAADRVFLASH_BLOCK_SIZE, SEEK_SET);
      for(index = 0; index < CIAADRVFLASH_BLOCK_CANT; index++)
      {
         fwrite(buffer, CIAADRVFLASH_BLOCK_SIZE, 1, flash->storage);
      }
      ret = 0;
   }
   return ret;
}

int32_t ciaaDriverFlash_blockWrite(uint32_t address, uint8_t const * const data, uint32_t size) {
   int32_t ret = -1;
   uint8_t buffer[CIAADRVFLASH_BLOCK_SIZE * CIAADRVFLASH_BLOCK_CANT];
   ciaaDriverFlash_flashType * flash = &ciaaDriverFlash_flash;
   uint32_t index;

   if (address <= CIAADRVFLASH_BLOCK_SIZE * CIAADRVFLASH_BLOCK_CANT)
   {
      rewind(flash->storage);
      fseek(flash->storage, address, SEEK_SET);
      fread(buffer, size, 1, flash->storage);
      for(index = 0; index < size; index++)
      {
         buffer[index] &= data[index];
      }
      rewind(flash->storage);
      fwrite(buffer, size, 1, flash->storage);
      //fflush(flash->storage);
      ret = 0;
   }
   return ret;
}
/*==================[external functions definition]==========================*/
extern ciaaDevices_deviceType * ciaaDriverFlash_open(char const * path, ciaaDevices_deviceType * device, uint8_t const oflag)
{
   ciaaDriverFlash_flashType * flash = device->layer;

   flash->storage = fopen(flash->filename,"r+b");
   if (flash->storage == NULL)
   {
      perror("Flash emulation file not exists: ");
      flash->storage = fopen(flash->filename,"w+b");
      if (flash->storage == NULL)
      {
         perror("Error creating flash emulation file: ");
         device = NULL;
      }
      else
      {
         ciaaDriverFlash_blockErase(0, CIAADRVFLASH_BLOCK_CANT - 1);
         rewind( flash->storage );
         flash->position = 0;
      }
   }
   return device;
}

extern int32_t ciaaDriverFlash_close(ciaaDevices_deviceType const * const device)
{
   int32_t ret = -1;
   ciaaDriverFlash_flashType * flash = device->layer;

   if(flash == &ciaaDriverFlash_flash)
   {
      fclose(flash->storage);
      ret = 0;
   }
   return ret;
}

extern int32_t ciaaDriverFlash_ioctl(ciaaDevices_deviceType const * const device, int32_t const request, void * param)
{
   int32_t ret = -1;

   ciaaDevices_blockType * block = param;
   ciaaDriverFlash_flashType * flash = device->layer;

   if(flash == &ciaaDriverFlash_flash)
   {
      switch(request)
      {
         case ciaaPOSIX_IOCTL_BLOCK_GETINFO:
            block->minRead = 0;
            block->maxRead = 0;
            block->minWrite = 0;
            block->maxWrite = 0;
            block->blockSize = CIAADRVFLASH_BLOCK_SIZE;
            block->lastPosition = CIAADRVFLASH_BLOCK_SIZE * CIAADRVFLASH_BLOCK_CANT;
            block->flags.eraseBeforeWrite = 1;
            ret = 1;
            break;

         case ciaaPOSIX_IOCTL_BLOCK_ERASE:
            ciaaDriverFlash_blockErase((uint32_t) (flash->position/CIAADRVFLASH_BLOCK_SIZE), (uint32_t) (flash->position/CIAADRVFLASH_BLOCK_SIZE));
            ret = 1;
            break;
      }
   }
   return ret;
}

extern ssize_t ciaaDriverFlash_read(ciaaDevices_deviceType const * const device, uint8_t* buffer, size_t size)
{
   ssize_t ret = -1;
   ciaaDriverFlash_flashType * flash = device->layer;

   if(flash == &ciaaDriverFlash_flash)
   {
      if (flash->storage)
      {
         rewind(flash->storage);
         fseek(flash->storage, flash->position, SEEK_SET);
         ret = fread(buffer, 1, size, flash->storage);
      }
   }

   return ret;
}

extern ssize_t ciaaDriverFlash_write(ciaaDevices_deviceType const * const device, uint8_t const * const buffer, size_t const size)
{
   ssize_t ret = -1;
   ciaaDriverFlash_flashType * flash = device->layer;

   if(flash == &ciaaDriverFlash_flash)
   {
      ciaaDriverFlash_blockWrite(flash->position, buffer, size);
      flash->position += size;
      ret = size;
   }
   return ret;
}

extern int32_t ciaaDriverFlash_seek(ciaaDevices_deviceType const * const device, int32_t const offset, uint8_t const whence)
{
   int32_t ret = -1;
   int32_t destination;
   ciaaDriverFlash_flashType * flash = device->layer;

   if(flash == &ciaaDriverFlash_flash)
   {
      switch(whence)
      {
         case SEEK_END:
            destination = CIAADRVFLASH_SIZE + offset;
            break;
         case SEEK_CUR:
            destination = flash->position + offset;
            break;
         default:
            destination = offset;
      }

      if ((destination >= 0) && (destination < CIAADRVFLASH_SIZE))
      {
         flash->position = destination;
         ret = 0;
      }
   }
   return ret;
}

void ciaaDriverFlash_init(void)
{
   ciaaDriverFlash_flashType * flash = ciaaDriverFlash_device.layer;
   flash->filename = ciaaDriverFlash_filename;
   ciaaBlockDevices_addDriver(&ciaaDriverFlash_device);
}

/*==================[interrupt handlers]=====================================*/
/** @} doxygen end group definition */
/** @} doxygen end group definition */
/** @} doxygen end group definition */
/*==================[end of file]============================================*/

