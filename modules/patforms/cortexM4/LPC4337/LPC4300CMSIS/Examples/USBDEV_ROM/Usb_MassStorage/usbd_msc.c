/***********************************************************************
 * $Id: usbd_msc.c 8097 2011-09-15 19:08:51Z nxp21346 $
 *
 * Project: LPC18xx Validation
 *
 * Description: USB mass storage example project.
 *
 ***********************************************************************
* Software that is described herein is for illustrative purposes only
* which provides customers with programming information regarding the
* products. This software is supplied "AS IS" without any warranties.
* NXP Semiconductors assumes no responsibility or liability for the
* use of the software, conveys no license or title under any patent,
* copyright, or mask work right to the product. NXP Semiconductors
* reserves the right to make changes in the software without
* notification. NXP Semiconductors also make no representation or
* warranty that such application will be suitable for the specified
* use without further testing or modification.
* Permission to use, copy, modify, and distribute this software and its
* documentation is hereby granted, under NXP Semiconductors'
* relevant copyright in the software, without fee, provided that it
* is used in conjunction with NXP Semiconductors microcontrollers.  This
* copyright, permission, and disclaimer notice must appear in all copies of
* this code.
 **********************************************************************/              
#include <string.h>
#include "LPC18xx.h"
#include "config.h"
#include "scu.h"
#include "usbd_config.h"
#include "sdio.h"

#include "usbd/usbd_mscuser.h"
#include "usbd/usbd_core.h"
#include "usbd/usbd_hw.h"
#include "usbd/usbd_mscuser.h"
#include "usbd/usbd_rom_api.h"


#ifndef _BIT
#define _BIT(n) (((uint32_t)(1ul)) << (n))
#endif

#define CFG_SDCARD

/**********************************************************************
 ** Extern Function prototyping 
**********************************************************************/
int usbd_msc_sdio_init (void); 
void usbd_msc_sdio_SysTick_Handler (void);
 					
extern ErrorCode_t usb_msc_mem_init(USBD_HANDLE_T hUsb, 
  USB_INTERFACE_DESCRIPTOR* pIntfDesc, uint32_t* mem_base, 
  uint32_t* mem_size);
extern uint32_t copy_descriptors(USB_CORE_DESCS_T* pDesc, 
  uint32_t mem_base);
extern ErrorCode_t USB_Configure_Event (USBD_HANDLE_T hUsb);

/**********************************************************************
 ** Function prototypes
 **********************************************************************/
static void vIOInit(void);
static void ClockInit(void);
static void vCatchError(uint8_t u8Error);

/**********************************************************************
 ** Global data 
**********************************************************************/
const uint8_t InquiryStr[] = {'N','X','P',' ',' ',' ',' ',' ',     \
                           'L','P','C',' ','M','e','m',' ',     \
                           'D','i','s','k',' ',' ',' ',' ',     \
                           '1','.','0',' ',};

static volatile uint32_t u32Milliseconds = 0;
/* local data */
static USBD_HANDLE_T hUsb;


//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
#define CACHE_SIZE (64 * 1024)
#define CACHE_SECTORS (CACHE_SIZE/MMC_SECTOR_SIZE)
static uint8_t* Memory = (uint8_t*)0x20000000;
int32_t cached_blocknum = -1;
int32_t cache_invalidated = 0;

//----------------------------------------------------------------------------
void MSC_cache_flush(void)
{
	if (cache_invalidated)
	{
		cache_invalidated = 0;
		sdio_write_blocks((void *) Memory, cached_blocknum, cached_blocknum + CACHE_SECTORS - 1);
	}
}

//----------------------------------------------------------------------------
void MSC_cache_read( uint32_t offset, uint8_t** buff_adr, uint32_t length)
{
	uint32_t blocknum = offset/CACHE_SIZE * CACHE_SECTORS;

	if (cached_blocknum != blocknum)
	{
		MSC_cache_flush();
		cached_blocknum = blocknum;
		sdio_read_blocks((void *) Memory, cached_blocknum, cached_blocknum + CACHE_SECTORS - 1);
	}
	memcpy((void*)*buff_adr, (void*)(Memory + (offset%CACHE_SIZE)), length);
}

//----------------------------------------------------------------------------
void MSC_cache_write( uint32_t offset, uint8_t** buff_adr, uint32_t length)
{
	uint32_t blocknum = offset/CACHE_SIZE * CACHE_SECTORS;

	if (cached_blocknum != blocknum)
	{
		MSC_cache_flush();
		MSC_cache_read(offset, buff_adr, length);
	}

	memcpy((void*)(Memory + (offset%CACHE_SIZE)), (void*)*buff_adr, length);
   	cache_invalidated = 1;
}
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////


/*----------------------------------------------------------------------------
  USB device mass storage class read callback routine
 *----------------------------------------------------------------------------*/
static void translate_rd( uint32_t offset, uint8_t** buff_adr, uint32_t length)
{
	{
		volatile int i = length;
	
		if (i != MSC_BlockSize)
			i++;
	}

#ifdef CFG_SDCARD

	MSC_cache_read(offset, buff_adr, length);
	
#else

  *buff_adr =  &Memory[offset];

#endif
}

/*----------------------------------------------------------------------------
  USB device mass storage class write callback routine
 *----------------------------------------------------------------------------*/
static void translate_wr( uint32_t offset, uint8_t** buff_adr, uint32_t length)
{
	{
		volatile int i = length;
	
		if (i != MSC_BlockSize)
			i++;
	}

#ifdef CFG_SDCARD

	MSC_cache_write(offset, buff_adr, length);
	
#else

  *buff_adr =  &Memory[offset + length];

#endif
}

/*----------------------------------------------------------------------------
  USB device mass storage class get write buffer callback routine
 *----------------------------------------------------------------------------*/
static void translate_GetWrBuf( uint32_t offset, uint8_t** buff_adr, uint32_t length)
{
  //memcpy((void*)&Memory[offset], *src, length);
  *buff_adr =  &Memory[offset];
}

/*----------------------------------------------------------------------------
  USB device mass storage class verify callback routine
 *----------------------------------------------------------------------------*/
static ErrorCode_t translate_verify( uint32_t offset, uint8_t* buff_adr, uint32_t length)
{
	{
		volatile int i = length;
	
		if (i != MSC_BlockSize)
			i++;
	}

#ifdef CFG_SDCARD
	uint32_t blocknum = offset/MSC_BlockSize;

	sdio_read_blocks((void *) Memory, blocknum, blocknum);
	memcpy((void*)*buff_adr, (void*)Memory, length);
	
#endif

  if (memcmp((void*)Memory, buff_adr, length))
    return ERR_FAILED;

  return LPC_OK;
}

/*----------------------------------------------------------------------------
  USB device mass storage class init routine
 *----------------------------------------------------------------------------*/
static ErrorCode_t usb_msc_mem_init(USBD_HANDLE_T hUsb, USB_INTERFACE_DESCRIPTOR* pIntfDesc, uint32_t* mem_base, uint32_t* mem_size)
{
    USBD_MSC_INIT_PARAM_T msc_param;
    ErrorCode_t ret = LPC_OK;
    
    memset((void*)&msc_param, 0, sizeof(USBD_MSC_INIT_PARAM_T));
    msc_param.mem_base = *mem_base;
    msc_param.mem_size = *mem_size;
    /* mass storage paramas */
    msc_param.InquiryStr = (uint8_t*)InquiryStr; 
#ifdef CFG_SDCARD
    msc_param.MemorySize = sdio_get_device_size();
#else
    msc_param.MemorySize = MSC_MemorySize;
#endif
    msc_param.BlockSize = MSC_BlockSize;
    msc_param.BlockCount = msc_param.MemorySize / msc_param.BlockSize;
    
    if ((pIntfDesc == 0) ||
    (pIntfDesc->bInterfaceClass != USB_DEVICE_CLASS_STORAGE) ||
        (pIntfDesc->bInterfaceSubClass != MSC_SUBCLASS_SCSI) )
    return ERR_FAILED;
    
    msc_param.intf_desc = (uint8_t*)pIntfDesc;
    /* user defined functions */
    msc_param.MSC_Write = translate_wr; 
    msc_param.MSC_Read = translate_rd;
    msc_param.MSC_Verify = translate_verify;
#ifdef CFG_SDCARD
    msc_param.MSC_GetWriteBuf = NULL;
#else
    msc_param.MSC_GetWriteBuf = translate_GetWrBuf;
#endif    
    
    ret = USBD_API->msc->init(hUsb, &msc_param);
    /* update memory variables */
    *mem_base = msc_param.mem_base;
    *mem_size = msc_param.mem_size;
    
    return ret;
}

#if defined(USB1_ULPI_PHY)
static void SetUsb1ClockPinmux( void )
{
    volatile int32_t tmo = 1000;
    uint32_t  portsc;

    /* disable USB1_CLOCK */
    LPC_CCU1->CLK_USB1_CFG = 0;

    /* set the processor in turbo mode to meet ULPI timings. */
    //value = LPC_CREG->PMUCON;
    //value &= ~(3 << 20);
    //value |= (7 << 6) | (2 << 20);
    //LPC_CREG->PMUCON = value;

#ifdef USE_ADC_VAL
    /* for ULPI
    P1_4 reset 
    P9_0 cs
    P8_8 = USB1_ULPI_CLK, FUNC1, I, ULPI link CLK signal. 60 MHz clock generated by the PHY.
    P8_6 = USB1_ULPI_NXT, FUNC1, I, ULPI link NXT signal. Data flow control signal from the PHY.
    P8_7 = USB1_ULPI_STP, FUNC1, O, ULPI link STP signal. Asserted to end or interrupt transfers to the PHY
    PC_11 = USB1_ULPI_DIR, FUNC1, I, ULPI link DIR signal. Controls the ULP data line direction.
    P8_5 = USB1_ULPI_D0, FUNC1, IO
    P8_4 = USB1_ULPI_D1, FUNC1, IO
    P8_3 = USB1_ULPI_D2, FUNC1, IO
    PC_5 = USB1_ULPI_D3, FUNC1, IO
    PC_4 = USB1_ULPI_D4, FUNC1, IO
    PC_3 = USB1_ULPI_D5, FUNC0, IO
    PC_2 = USB1_ULPI_D6, FUNC0, IO
    PC_1 = USB1_ULPI_D7, FUNC0, IO
    */
#if 0
    scu_pinmux(0x1, 4, MD_PDN | MD_EZI | MD_ZI , FUNC0);
    /* force chip select low since we have only one device on this bus*/
    scu_pinmux(0x9, 0, MD_PDN | MD_EZI | MD_ZI , FUNC0);
    scu_pinmux(0x8, 7, MD_PLN | MD_EZI | MD_ZI | MD_EHS, FUNC0);
    scu_pinmux(0x8, 5, MD_PLN | MD_EZI | MD_ZI | MD_EHS , FUNC0);
    scu_pinmux(0x8, 4, MD_PLN | MD_EZI | MD_ZI | MD_EHS , FUNC0);
    scu_pinmux(0x8, 3, MD_PLN | MD_EZI | MD_ZI | MD_EHS , FUNC0);
    scu_pinmux(0xC, 5, MD_PLN | MD_EZI | MD_ZI | MD_EHS , FUNC4);
    scu_pinmux(0xC, 4, MD_PLN | MD_EZI | MD_ZI | MD_EHS , FUNC4);
    scu_pinmux(0xC, 3, MD_PLN | MD_EZI | MD_ZI | MD_EHS , FUNC4);
    scu_pinmux(0xC, 2, MD_PLN | MD_EZI | MD_ZI | MD_EHS , FUNC4);
    scu_pinmux(0xC, 1, MD_PLN | MD_EZI | MD_ZI | MD_EHS , FUNC4);
    /* PC_1 to PC_5 */
    LPC_GPIO6->DIR = 0x1F;
    LPC_GPIO6->CLR = 0x1F;
    /* P8_3 to P8_5,  P8_7, P9_0*/
    LPC_GPIO4->DIR = 0x1B8;
    LPC_GPIO4->CLR = 0x1B8;
    /* reset the chip*/
    LPC_GPIO0->DIR = _BIT(11);
    LPC_GPIO0->CLR = _BIT(11);
#endif

    scu_pinmux(0x8, 8, MD_PLN | MD_EZI | MD_ZI | MD_EHS, FUNC1);
    scu_pinmux(0x8, 6, MD_PLN | MD_EZI | MD_ZI | MD_EHS, FUNC1);
    scu_pinmux(0x8, 7, MD_PLN | MD_EZI | MD_ZI | MD_EHS, FUNC1);
    scu_pinmux(0xC, 11, MD_PLN | MD_EZI | MD_ZI | MD_EHS, FUNC1);
    scu_pinmux(0x8, 5, MD_PLN | MD_EZI | MD_ZI | MD_EHS , FUNC1);
    scu_pinmux(0x8, 4, MD_PLN | MD_EZI | MD_ZI | MD_EHS , FUNC1);
    scu_pinmux(0x8, 3, MD_PLN | MD_EZI | MD_ZI | MD_EHS , FUNC1);
    scu_pinmux(0xC, 5, MD_PLN | MD_EZI | MD_ZI | MD_EHS , FUNC1);
    scu_pinmux(0xC, 4, MD_PLN | MD_EZI | MD_ZI | MD_EHS , FUNC1);
    scu_pinmux(0xC, 3, MD_PLN | MD_EZI | MD_ZI | MD_EHS , FUNC0);
    scu_pinmux(0xC, 2, MD_PLN | MD_EZI | MD_ZI | MD_EHS , FUNC0);
    scu_pinmux(0xC, 1, MD_PLN | MD_EZI | MD_ZI | MD_EHS , FUNC0);


#else

#if 0
    /* force chip select low since we have only one device on this bus*/
    scu_pinmux(0x1, 5, MD_PDN | MD_EZI | MD_ZI , FUNC0);
    scu_pinmux(0xC, 10, MD_PLN | MD_EZI | MD_ZI | MD_EHS, FUNC4);
    scu_pinmux(0xC, 8, MD_PLN | MD_EZI | MD_ZI | MD_EHS , FUNC4);
    scu_pinmux(0xC, 7, MD_PLN | MD_EZI | MD_ZI | MD_EHS , FUNC4);
    scu_pinmux(0xC, 6, MD_PLN | MD_EZI | MD_ZI | MD_EHS , FUNC4);
    scu_pinmux(0xC, 5, MD_PLN | MD_EZI | MD_ZI | MD_EHS , FUNC4);
    scu_pinmux(0xC, 4, MD_PLN | MD_EZI | MD_ZI | MD_EHS , FUNC4);
    scu_pinmux(0xC, 3, MD_PLN | MD_EZI | MD_ZI | MD_EHS , FUNC4);
    scu_pinmux(0xC, 2, MD_PLN | MD_EZI | MD_ZI | MD_EHS , FUNC4);
    scu_pinmux(0xC, 1, MD_PLN | MD_EZI | MD_ZI | MD_EHS , FUNC4);

    LPC_GPIO6->DIR = 0x2FF;
    LPC_GPIO6->CLR = 0x2FF;

    LPC_GPIO1->DIR = (1UL << 8);
    LPC_GPIO1->CLR = _BIT(8);

    while(tmo-- > 0);

    LPC_GPIO1->SET = (1UL << 8);
#endif
    /* for ULPI 
    P1_5
    P8_8 = USB1_ULPI_CLK, FUNC1, I, ULPI link CLK signal. 60 MHz clock generated by the PHY.
    PC_9 = USB1_ULPI_NXT, FUNC1, I, ULPI link NXT signal. Data flow control signal from the PHY.
    PC_10 = USB1_ULPI_STP, FUNC1, O, ULPI link STP signal. Asserted to end or interrupt transfers to the PHY
    PC_11 = USB1_ULPI_DIR, FUNC1, I, ULPI link DIR signal. Controls the ULP data line direction.
    PC_8 = USB1_ULPI_D0, FUNC1, IO
    PC_7 = USB1_ULPI_D1, FUNC1, IO
    PC_6 = USB1_ULPI_D2, FUNC1, IO
    PC_5 = USB1_ULPI_D3, FUNC1, IO
    PC_4 = USB1_ULPI_D4, FUNC1, IO
    PC_3 = USB1_ULPI_D5, FUNC0, IO
    PC_2 = USB1_ULPI_D6, FUNC0, IO
    PC_1 = USB1_ULPI_D7, FUNC0, IO
    */
    scu_pinmux(0x8, 8, MD_PLN | MD_EZI | MD_ZI | MD_EHS, FUNC1);
    scu_pinmux(0xC, 9, MD_PLN | MD_EZI | MD_ZI | MD_EHS, FUNC1);
    scu_pinmux(0xC, 10, MD_PLN | MD_EZI | MD_ZI | MD_EHS, FUNC1);
    scu_pinmux(0xC, 11, MD_PLN | MD_EZI | MD_ZI | MD_EHS, FUNC1);
    scu_pinmux(0xC, 8, MD_PLN | MD_EZI | MD_ZI | MD_EHS , FUNC1);
    scu_pinmux(0xC, 7, MD_PLN | MD_EZI | MD_ZI | MD_EHS , FUNC1);
    scu_pinmux(0xC, 6, MD_PLN | MD_EZI | MD_ZI | MD_EHS , FUNC1);
    scu_pinmux(0xC, 5, MD_PLN | MD_EZI | MD_ZI | MD_EHS , FUNC1);
    scu_pinmux(0xC, 4, MD_PLN | MD_EZI | MD_ZI | MD_EHS , FUNC1);
    scu_pinmux(0xC, 3, MD_PLN | MD_EZI | MD_ZI | MD_EHS , FUNC0);
    scu_pinmux(0xC, 2, MD_PLN | MD_EZI | MD_ZI | MD_EHS , FUNC0);
    scu_pinmux(0xC, 1, MD_PLN | MD_EZI | MD_ZI | MD_EHS , FUNC0);

#endif

    /* switch to ulpi phy and turn on the power to phy*/
    portsc = LPC_USB1->PORTSC1_D & 0x00FFFFFF;
    portsc |= 0x80000000;
    LPC_USB1->PORTSC1_D = portsc;
    /* reset the controller */
    LPC_USB1->USBCMD_D = _BIT(1);
    /* wait for reset to complete */
    while (LPC_USB1->USBCMD_D & _BIT(1));
    
    /* Program the controller to be the USB device mode */
    LPC_USB1->USBMODE_D =   0x02 | _BIT(3);

}
#else
static void SetUsb1ClockPinmux( void )
{
    /* enable USB phy */
    LPC_CREG->CREG0 &= ~(1 << 5);
    //LPC_CREG->RESERVED1[4] |= (1 << 17);

    /* enable USB1_DP and USB1_DN on chip FS phy */
    LPC_SCU->SFSUSB = 0x12;
    /* enable USB1_VBUS */
    scu_pinmux(0x2, 5, MD_PLN | MD_EZI | MD_ZI, FUNC2);

    /* connect 60MHz CLK_USB1 clock */
    /* Run base BASE_USB1_CLK clock from PL160M, no division */
    SetClock(BASE_USB1_CLK, SRC_PL160M_0, DIV2);        
    SetClock(BASE_OUT_CLK, SRC_PL160M_0, DIV2);         
}
#endif

/**********************************************************************
 ** Function name:      
 **
 ** Description:        
 **                     
 ** Parameters:         
 **
 ** Returned value:     
 **********************************************************************/

int main (void) 
{
    
    USBD_API_INIT_PARAM_T usb_param;
    USB_CORE_DESCS_T desc;
    ErrorCode_t ret;
    USB_INTERFACE_DESCRIPTOR* pIntfDesc;
    
   
    SystemInit();

    ClockInit();

    /* Configure the IO's for the LEDs */ 
    vIOInit();          
    
    /* Generate interrupt @ 1000 Hz */
    SysTick_Config(M3Frequency/1000);               

    // Setup PLL550 to generate 480MHz from 12 MHz crystal
    SetPLLUSB(SRC_XTAL, 1);

    /* enable clocks and pinmux for usb0 */
#if (CURR_USB_PORT == LPC_USB1_BASE)
    /* for USB1 set the clocks and pin muxing based on the board */
    SetUsb1ClockPinmux();
#endif

#ifdef CFG_SDCARD
	usbd_msc_sdio_init();
#endif
    /* initilize call back structures */
    memset((void*)&usb_param, 0, sizeof(USBD_API_INIT_PARAM_T));
    usb_param.usb_reg_base = CURR_USB_PORT;
    usb_param.max_num_ep = 4;
    usb_param.mem_base = 0x10010000;
    usb_param.mem_size = 0x2000;

    /* for eagle/raptor the local SRAM is not accesable to USB
    * so copy the descriptors to USB accessable memory
    */
    copy_descriptors(&desc, usb_param.mem_base + usb_param.mem_size);

#if (CURR_USB_PORT == LPC_USB0_BASE)
	/* Turn on the phy */
	LPC_CREG->CREG0 &= ~(1<<5);
#endif

    /* USB Initialization */
    ret = USBD_API->hw->Init(&hUsb, &desc, &usb_param);  
    if (ret == LPC_OK) {

#if (CURR_USB_PORT == LPC_USB1_BASE)

       /* force full speed for USB1 */
       LPC_USB1->PORTSC1_D |= (1<<24);
#endif

        pIntfDesc = (USB_INTERFACE_DESCRIPTOR*)((uint32_t)desc.high_speed_desc + USB_CONFIGUARTION_DESC_SIZE);
        ret = usb_msc_mem_init(hUsb, pIntfDesc, &usb_param.mem_base, &usb_param.mem_size);
        if (ret != LPC_OK) {
            vCatchError(0); //"usb_msc_mem_init error!!!"
        }


        if (ret == LPC_OK) {

#if (CURR_USB_PORT != LPC_USB1_BASE)
            NVIC_EnableIRQ(USB0_IRQn); //  enable USB0 interrrupts 
#else
            NVIC_EnableIRQ(USB1_IRQn); //  enable USB1 interrrupts
#endif
            /* now connect */
            USBD_API->hw->Connect(hUsb, 1); 
        }

    }                       
    else {
        vCatchError(1); //"\r\nhwUSB_Init error!!!"
    }

    while (1) 
    {                                           
        u32Milliseconds = 100;
        
        /* Wait... */
        while(u32Milliseconds);

		MSC_cache_flush();

        /* Toggle LED state */
#if defined(USE_BGA504) 
            LPC_GPIO3->NOT = (1UL << 6);
#elif defined(USE_ADC_VAL)
            LPC_GPIO1->NOT = (1UL << 1);
#else
            LPC_GPIO4->NOT = (1UL << 14);
#endif
    }
}

/*----------------------------------------------------------------------------
  System tick handler
 *----------------------------------------------------------------------------*/
void SysTick_Handler (void)                     
{           
    if(u32Milliseconds > 0)
    {
        u32Milliseconds--; 
    }

#ifdef CFG_SDCARD
	usbd_msc_sdio_SysTick_Handler();
#endif

}

/*----------------------------------------------------------------------------
  Initialize /FalconEagle validation board specific IO
 *----------------------------------------------------------------------------*/
static void vIOInit(void)
{
#if defined(USE_BGA504)  /* validation board with 504BGA */
    scu_pinmux (0x6, 10, MD_PDN, FUNC0);    // P6.10 : GPIO3_6: Via jumper cable from SW4 to LD11 (LED)
    LPC_GPIO3->DIR = (1UL << 6);            // GPIO3_6 Output
#elif defined(USE_ADC_VAL)  /* ADC validation board with 256BGA */
    scu_pinmux(0x1 , 8, MD_PDN, FUNC0);     // P1.8 : GPIO1_1: D5(LED)
    LPC_GPIO1->DIR  = (1UL << 1);
#elif defined(NGX_BOARD)  /* validation board with NGX_BAORD */
	scu_pinmux(0x2 ,11 , MD_PDN, FUNC0); 	/* P2.11 : GPIO1_11: LD11 (LED) */
	LPC_GPIO1->DIR  |= (1UL << 11);
	LPC_GPIO1->SET  = ~(1UL << 11);
#else /* validation board with 256BGA */
    scu_pinmux(0x9 ,2 , MD_PDN, FUNC0);     // P9.2 : GPIO4_14: LD11 (LED)
    LPC_GPIO4->DIR  = (1UL << 14);
	scu_pinmux(0xA ,4 , MD_PDN, FUNC4); 	/* P9.2 : GPIO4_14: LD11 (LED) */
	LPC_GPIO5->DIR  = (1UL << 19);
	LPC_GPIO5->SET  = (1UL << 19);
#endif
}

/*----------------------------------------------------------------------------
  Initialize clocks
 *----------------------------------------------------------------------------*/
static void ClockInit(void)
{
    /* Set PL160M @ 10*12=120 MHz */
    SetPL160M(SRC_XTAL, 10);                        
    /* Run base M3 clock from PL160M, no division */
    SetClock(BASE_M3_CLK, SRC_PL160M_0, DIV1);      
    /* Show base out clock on output */
    SetClock(BASE_OUT_CLK, SRC_XTAL, DIV1);         
}
/**********************************************************************
 ** Function name:      
 **
 ** Description:        
 **                     
 ** Parameters:         
 **
 ** Returned value:     
 **********************************************************************/
#if (CURR_USB_PORT != LPC_USB1_BASE)
void USB0_IRQHandler(void)
{
    USBD_API->hw->ISR(hUsb);
}
#else
void USB1_IRQHandler(void)
{
    USBD_API->hw->ISR(hUsb);
}
#endif

/**********************************************************************
 ** Function name:      
 **
 ** Description:        
 **                     
 ** Parameters:         
 **
 ** Returned value:     
 **********************************************************************/
static void vCatchError(uint8_t u8Error)
{
    volatile uint8_t u8theError = u8Error;

    while(1);
}


/**********************************************************************
 **                            End Of File
 **********************************************************************/
