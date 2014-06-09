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

/** \brief CIAA POSIX source file
 **
 ** This file contains the POSIX implementation
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
 * MaCe         Mariano Cerdeiro
 */

/*
 * modification history (new versions first)
 * -----------------------------------------------------------
 * 20140528 v0.0.2 MaCe implement printf
 * 20140420 v0.0.1 EzEs initial version
 */

/*==================[inclusions]=============================================*/
#include "ciaak.h"
#include "ciaaPlatforms.h"
#include "ciaaPOSIX_stdio.h"
#include "ciaaPOSIX_string.h"
#include "ciaaPOSIX_stdlib.h"

/* in windows and posix also include posix interfaces */
#if ( (ARCH == win) || (ARCH == posix) )
#include "stdio.h"
#include "stdarg.h"
#include "Os_Internal.h"
#endif

/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/
/** \brief Filedescriptor type */
typedef struct {
	ciaaDevices_deviceType const * device;
} ciaaPOSIX_stdio_fildesType;

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/
/** \brief List of files descriptors */
ciaaPOSIX_stdio_fildesType ciaaPOSIX_stdio_fildes[ciaaPOSIX_stdio_MAXFILDES];

/** \brief device prefix */
char const * const ciaaPOSIX_stdio_devPrefix = "/dev";

/** \brief ciaa POSIX stdio sempahore */
static sem_t ciaaPOSIX_stdio_sem;

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/
void ciaaPOSIX_init(void)
{
   uint32_t loopi;

   /* init all posix devices */
   for (loopi = 0; loopi < ciaaPOSIX_stdio_MAXFILDES; loopi++) {
      ciaaPOSIX_stdio_fildes[loopi].device = NULL;
   }

   /* init semaphore */
   ciaaPOSIX_sem_init(&ciaaPOSIX_stdio_sem);
}

extern int32_t ciaaPOSIX_open(char const * const path, uint8_t const oflag)
{
   ciaaDevices_deviceType const * device;
   int32_t ret = -1;
   int8_t loopi;

   /* check if device */
   if (strncmp(path, ciaaPOSIX_stdio_devPrefix, strlen(ciaaPOSIX_stdio_devPrefix)) == 0)
   {
      /* get device */
      device = ciaaDevices_getDevice(path);

      /* if a device has been found */
      if (NULL != device)
      {
         /* search a file descriptor */
         for(loopi = 0; (loopi < ciaaPOSIX_stdio_MAXFILDES) && (-1 == ret); loopi++)
         {
            /* enter critical section */
            ciaaPOSIX_sem_wait(&ciaaPOSIX_stdio_sem);

            /* if file descriptor not used, use it */
            if (NULL != ciaaPOSIX_stdio_fildes[loopi].device)
            {
               /* load device in descriptor */
               ciaaPOSIX_stdio_fildes[loopi].device = device;

               /* return file descriptor */
               ret = loopi;
            }

            /* exit critical section */
            ciaaPOSIX_sem_post(&ciaaPOSIX_stdio_sem);
         }

         /* if a file descriptor has been found */
         if (-1 != ret)
         {
            /* open device */
            if (ciaaPOSIX_stdio_fildes[ret].device->open(
                     ciaaPOSIX_stdio_fildes[ret].device,
                     oflag) > 0)
            {
               /* open device successfull */
               /* nothing to do */
            }
            else
            {
               /* device could not be opened */
               
               /* enter critical section */
               ciaaPOSIX_sem_wait(&ciaaPOSIX_stdio_sem);

               /* remove device from file descriptor */
               ciaaPOSIX_stdio_fildes[ret].device = NULL;

               /* exit critical section */
               ciaaPOSIX_sem_post(&ciaaPOSIX_stdio_sem);

               /* return an error */
               ret = -1;
            }
         }

      }
   }
   else
   {
      /* TODO add ASSERT */
      /* TODO implement file handler */
   }

   return ret;
} /* end ciaaPOSIX_open */

int32_t ciaaPOSIX_close(int32_t fildes)
{
   int32_t ret = -1;

   if ( (fildes >= 0) && (fildes < ciaaPOSIX_stdio_MAXFILDES) )
   {
      ciaaPOSIX_sem_wait(&ciaaPOSIX_stdio_sem);

      if (NULL != ciaaPOSIX_stdio_fildes[fildes].device)
      {
         ret = ciaaPOSIX_stdio_fildes[fildes].device->close(ciaaPOSIX_stdio_fildes[fildes].device);
         if (0 == ret)
         {
            /* free file descriptor, file has been closed */
            ciaaPOSIX_stdio_fildes[fildes].device = NULL;
         }
      }
   }

   return ret;
}

int32_t ciaaPOSIX_ioctl (int32_t fildes, int32_t request, void* param)
{
   int32_t ret = -1;

   /* check that file descriptor is on range */
   if ( (fildes >= 0) && (fildes < ciaaPOSIX_stdio_MAXFILDES) )
   {
      /* check that file descriptor is beeing used */
      if (NULL != ciaaPOSIX_stdio_fildes[fildes].device)
      {
         /* call ioctl function */
         ret = ciaaPOSIX_stdio_fildes[fildes].device->ioctl(
               ciaaPOSIX_stdio_fildes[fildes].device,
               request,
               param);
      }
   }

   return ret;
}

int32_t ciaaPOSIX_read (int32_t const fildes, uint8_t * buffer, uint32_t nbyte)
{
   int32_t ret = -1;

   /* check that file descriptor is on range */
   if ( (fildes >= 0) && (fildes < ciaaPOSIX_stdio_MAXFILDES) )
   {
      /* check that file descriptor is beeing used */
      if (NULL != ciaaPOSIX_stdio_fildes[fildes].device)
      {
         /* call read function */
         ret = ciaaPOSIX_stdio_fildes[fildes].device->read(
               ciaaPOSIX_stdio_fildes[fildes].device,
               buffer,
               nbyte);
      }
   }

   return ret;
}

extern int32_t ciaaPOSIX_write (int32_t const fildes, uint8_t const * const buf, uint32_t nbyte)
{
   int32_t ret = -1;

   /* check that file descriptor is on range */
   if ( (fildes >= 0) && (fildes < ciaaPOSIX_stdio_MAXFILDES) )
   {
      /* check that file descriptor is beeing used */
      if (NULL != ciaaPOSIX_stdio_fildes[fildes].device)
      {
         /* call write function */
         ret = ciaaPOSIX_stdio_fildes[fildes].device->write(
               ciaaPOSIX_stdio_fildes[fildes].device,
               buf,
               nbyte);
      }
   }

   return ret;
}

extern int32_t ciaaPOSIX_printf(const char * format, ...)
{
   int32_t ret;

#if ( (ARCH == win) || (ARCH == posix) )
   /* OS pre call service, changes stack to system stack */
   /* #36 TODO */
   PreCallService();

   /* call vprintf passing all received parameters */
   va_list args;
   va_start(args, format);
   ret = vprintf(format, args);
   va_end(args);
   /* Fixes a Bug in Eclipse (173732) print to the console */
   /* See issue CIAA Firmware issue #35: https://github.com/ciaa/Firmware/issues/35 */
   fflush(stdout);

   /* OS post call service, changes stack to RTOS stack */
   /* #36 TODO */
   PostCallService();
#else
   /* parameter format is not used in no win nor posix arch, casted to void to
    * avoid compiler warning */
   (void)format;
   /* this interface is not supported in no windows nor posix system */
   ret = -1;
#endif

   return ret;
}

/** @} doxygen end group definition */
/** @} doxygen end group definition */
/*==================[end of file]============================================*/

