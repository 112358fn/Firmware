/* Copyright 2014, ACSE & CADIEEL
 *    ACSE   : http://www.sase.com.ar/asociacion-civil-sistemas-embebidos/ciaa/
 *    CADIEEL: http://www.cadieel.org.ar
 * All rights reserved.
 *
 *    or
 *
 * Copyright 2014, Your Name <youremail@domain.com>
 * All rights reserved.
 *
 *    or
 *
 * Copyright 2014, ACSE & CADIEEL & Your Name <youremail@domain.com
 *    ACSE   : http://www.sase.com.ar/asociacion-civil-sistemas-embebidos/ciaa/
 *    CADIEEL: http://www.cadieel.org.ar
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

/** \brief Short description of this file
 **
 ** Long description of this file
 **
 **/

/** \addtogroup CIAA_Firmware CIAA Firmware
 ** @{ */
/** \addtogroup Template Template to start a new module
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
#include "os.h"						/* <= operating system header */
#include "speechrec_ssp_internal.h"	/* <= ssp internal header */

/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

/** \brief SSP configuration format variable
 *
 */
static SSP_ConfigFormat ssp_format;

/** \brief GPDMA channel number for transmision
 *
 */
static uint8_t dmaChSSPTx;

/** \brief GPDMA channel number for reception
 *
 */
static uint8_t dmaChSSPRx;

/** \brief Current memory buffer indicator
 * Indicates which memory buffer is currently being written by the DMA
 */
static uint8_t currentDMA;

/** \brief Dummy transmission data used to generate the SSP transmission
 *
 */
static uint8_t memDMATx;


/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/** \brief First memory buffer for window 1
 * Maximum DMA transference size: 4096 =>
 * => Two memory buffers used for each window
 */
uint8_t memDest1ADMA[SPEECHREC_MEMDMASIZE];

/** \brief Second memory buffer for window 1
 * Maximum DMA transference size: 4096 =>
 * => Two memory buffers used for each window
 */
uint8_t memDest1BDMA[SPEECHREC_MEMDMASIZE];

/** \brief First memory buffer for window 2
 * Maximum DMA transference size: 4096 =>
 * => Two memory buffers used for each window
 */
uint8_t memDest2ADMA[SPEECHREC_MEMDMASIZE];

/** \brief Second memory buffer for window 2
 * Maximum DMA transference size: 4096 =>
 * => Two memory buffers used for each window
 */
uint8_t memDest2BDMA[SPEECHREC_MEMDMASIZE];

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/

/** \brief Initialize and start SPI transference with DMA
 *
 */
extern void speechrec_spi_dma_start(void)
{
   /* SSP DMA Read and Write: fixed on 8bits */

   /* Initialize CIAA Pins for the SSP interface */

   if (SPEECHREC_SSPn == LPC_SSP1) {
      Chip_SCU_PinMuxSet(0x1, 5, (SCU_PINIO_FAST | SCU_MODE_FUNC5));  /* P1.5 => SSEL1 (not connected to microphone) */
      Chip_SCU_PinMuxSet(0xF, 4, (SCU_PINIO_FAST | SCU_MODE_FUNC0));  /* PF.4 => SCK1 */

   	  Chip_SCU_PinMuxSet(0x1, 4, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC5)); /* P1.4 => MOSI1 (not connected to microphone) */
	  Chip_SCU_PinMuxSet(0x1, 3, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC5)); /* P1.3 => MISO1 */
   }
   else {
      while(1){}
   }


   /* Initialize the SSP interface */

   Chip_SSP_Init(SPEECHREC_SSPn);
   Chip_SSP_SetBitRate(SPEECHREC_SSPn, SPEECHREC_BITRATE*1000);

   /* Configure SSP Format */
   ssp_format.frameFormat = SSP_FRAMEFORMAT_SPI;
   ssp_format.bits = SPEECHREC_SSP_DATA_BITS;
   ssp_format.clockMode = SSP_CLOCK_MODE3;
   Chip_SSP_SetFormat(SPEECHREC_SSPn, ssp_format.bits, ssp_format.frameFormat, ssp_format.clockMode);

   Chip_SSP_Enable(SPEECHREC_SSPn);

   /* Initialize GPDMA controller */
   Chip_GPDMA_Init(LPC_GPDMA);

   /* Setting GPDMA interrupt */
   NVIC_DisableIRQ(DMA_IRQn);
   NVIC_SetPriority(DMA_IRQn, ((0x01 << 3) | 0x01));
   NVIC_EnableIRQ(DMA_IRQn);

   /* Set the SSP in master mode */
   Chip_SSP_SetMaster(SPEECHREC_SSPn, 1);

   /* Get a free GPDMA channel for one DMA connection for transmission */
   dmaChSSPTx = Chip_GPDMA_GetFreeChannel(LPC_GPDMA, SPEECHREC_GPDMA_SSPn_TX);

   /* Get a free GPDMA channel for one DMA connection for reception */
   dmaChSSPRx = Chip_GPDMA_GetFreeChannel(LPC_GPDMA, SPEECHREC_GPDMA_SSPn_RX);

   Chip_SSP_DMA_Enable(SPEECHREC_SSPn);

   /* Do a DMA transfer P2M: data SSP --> memDest1ADMA */
   Chip_GPDMA_Transfer(LPC_GPDMA, dmaChSSPRx,
		   	   	  SPEECHREC_GPDMA_SSPn_RX, /* source */
				  (uint32_t) &memDest1ADMA[0], /* destination */
				  GPDMA_TRANSFERTYPE_P2M_CONTROLLER_DMA,
				  SPEECHREC_MEMDMASIZE);

   /* Do a DMA transfer P2M: memDMATx --> SSP */
   Chip_GPDMA_Transfer(LPC_GPDMA, dmaChSSPTx,
				  (uint32_t) &memDMATx, /* source */
				  SPEECHREC_GPDMA_SSPn_TX, /* destination */
				  GPDMA_TRANSFERTYPE_M2P_CONTROLLER_DMA,
				  SPEECHREC_MEMDMASIZE);

   currentDMA = 1;
}

/*==================[interrupt handlers]=====================================*/

/** \brief DMA interrupt handler sub-routine
 *
 */
ISR(DMA_IRQHandler)
{

   if(Chip_GPDMA_Interrupt(LPC_GPDMA, dmaChSSPRx) == SUCCESS){

      if (currentDMA == 1){
         /* Do a DMA transfer P2M: data SSP --> memDest1BDMA */
         Chip_GPDMA_Transfer(LPC_GPDMA, dmaChSSPRx,
				   	   	  SPEECHREC_GPDMA_SSPn_RX, /* source */
		   				  (uint32_t) &memDest1BDMA[0], /* destination */
		   				  GPDMA_TRANSFERTYPE_P2M_CONTROLLER_DMA,
		   				  SPEECHREC_MEMDMASIZE);

         currentDMA = 2;
      }
      else if (currentDMA == 2){

         /* Do a DMA transfer P2M: data SSP --> memDest2ADMA */
         Chip_GPDMA_Transfer(LPC_GPDMA, dmaChSSPRx,
				   	   	  SPEECHREC_GPDMA_SSPn_RX, // source
		   				  (uint32_t) &memDest2ADMA[0], /* destination */
		   				  GPDMA_TRANSFERTYPE_P2M_CONTROLLER_DMA,
		   				  SPEECHREC_MEMDMASIZE);


         currentDMA = 3;
      }
      else if (currentDMA == 3){

         /* Do a DMA transfer P2M: data SSP --> memDest2BDMA */
         Chip_GPDMA_Transfer(LPC_GPDMA, dmaChSSPRx,
				   	   	  SPEECHREC_GPDMA_SSPn_RX, /* source */
		   				  (uint32_t) &memDest2BDMA[0], /* destination */
		   				  GPDMA_TRANSFERTYPE_P2M_CONTROLLER_DMA,
		   				  SPEECHREC_MEMDMASIZE);

         currentDMA = 4;

      }
      else{ /* currentDMA = 4 */

         /* Do a DMA transfer P2M: data SSP --> memDest1ADMA */
         Chip_GPDMA_Transfer(LPC_GPDMA, dmaChSSPRx,
				   	   	  SPEECHREC_GPDMA_SSPn_RX, /* source */
		   				  (uint32_t) &memDest1ADMA[0], /* destination */
		   				  GPDMA_TRANSFERTYPE_P2M_CONTROLLER_DMA,
		   				  SPEECHREC_MEMDMASIZE);

         currentDMA = 1;
      }

   }
   else if (Chip_GPDMA_Interrupt(LPC_GPDMA, dmaChSSPTx) == SUCCESS){

      /* Do a DMA transfer P2M: memDMATx --> SSP */
      Chip_GPDMA_Transfer(LPC_GPDMA, dmaChSSPTx,
	   				  (uint32_t) &memDMATx, /* source */
	   				  SPEECHREC_GPDMA_SSPn_TX, /* destination */
	   				  GPDMA_TRANSFERTYPE_M2P_CONTROLLER_DMA,
	   				  SPEECHREC_MEMDMASIZE);

   }

}

/** @} doxygen end group definition */
/** @} doxygen end group definition */
/*==================[end of file]============================================*/
