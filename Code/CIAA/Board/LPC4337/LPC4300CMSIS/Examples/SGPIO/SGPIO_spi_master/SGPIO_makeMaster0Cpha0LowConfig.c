#include <stdint.h>

#include "sgpio_spi.h"

extern CsPattern cs0Waveform;
extern CsPattern cs0upWaveform;
extern const CsPattern ChipSelectWaveform[NUM_CS_IDX];

extern const SgpioSliceCfg CHIP_SELECT_CFG0;
extern const SgpioSliceCfg CHIP_SELECT_CFG0_INTERNAL;
extern const SgpioSliceCfg DATA_OUT_CFG0;
extern const SgpioSliceCfg CLOCK_CFG0;
extern const SgpioSliceCfg DATA_IN_CFG0;

void SGPIO_makeMaster0Cpha0LowConfig(SpiParam const *config) {

	SGPIO_SliceMuxConfig 	sliceMuxCfg;
	SGPIO_MuxConfig 		muxCfg;
	SGPIO_OutMuxConfig		outMuxCfg;

	cs0Waveform = ChipSelectWaveform[config->chipSelect];

	if(config->wordLenght == DBIT_30) {
		
		// for the 30-bit case
		cs0upWaveform = 0x1;				
		
	} else if (config->wordLenght == DBIT_32) {
			
		// for the 32-bit case
		cs0upWaveform = 0x4;
	};
	
	/**********************************************************************
	* make the static configuration for the Chip Select, slice A + I		
	*
	**********************************************************************/
	SGPIO_disableSlice(CHIP_SELECT_CFG0.sliceId);
	
	sliceMuxCfg = SGPIO_makeSliceMuxConfig(
		SMC_MATCH_DATA,
		SMC_CLKCAP_DONTCARE,
		SMC_CLKGEN_COUNTER,
		SMC_INVOUT_DONTCARE,
		SMC_DATACAP_DONTCARE,
		SMC_PAR_1BPCK,
		SMC_INVQUAL_INVERTED);
	
	SGPIO_configSliceMuxReg(CHIP_SELECT_CFG0.sliceId, sliceMuxCfg);
	
	muxCfg = SGPIO_makeMuxConfig(
		MC_CLK_INTERNAL,
		MC_CLKSRC_PIN_DONTCARE,
		MC_CLKSRC_SLICE_DONTCARE,
		MC_QUALMODE_SLICE,
		MC_QUALPIN_DONTCARE,
		MC_QUALSLICE_A_D,
		MC_CONCAT_DATA,
		MC_CONCAT_TWO_SLICES);
	
	SGPIO_configMuxReg(CHIP_SELECT_CFG0.sliceId, muxCfg);
	
	outMuxCfg = SGPIO_makeOutMuxConfig(OMC_DOUTM1, OMC_GPIO_OE);

	SGPIO_configOutMuxReg(CHIP_SELECT_CFG0.pinId, outMuxCfg);

	if(CHIP_SELECT_CFG0.sliceFunc == SGPIO_OUTPUT_PIN)
		SGPIO_setOeReg(CHIP_SELECT_CFG0.pinId, outMuxCfg);

	SGPIO_setBitCountReg(CHIP_SELECT_CFG0.sliceId, (config->wordLenght)+2);
	
	SGPIO_setCountReloadReg(CHIP_SELECT_CFG0.sliceId, SGPIO_IP_CLOCK/(2*(config->bitRateHz)));
	
	SGPIO_writeDataReg(CHIP_SELECT_CFG0.sliceId, cs0Waveform);
	
	// Slice I is part of the Chip Select
	SGPIO_disableSlice(CHIP_SELECT_CFG0_INTERNAL.sliceId);
	
	sliceMuxCfg = SGPIO_makeSliceMuxConfig(
		SMC_MATCH_DATA,
		SMC_CLKCAP_DONTCARE,
		SMC_CLKGEN_COUNTER,
		SMC_INVOUT_DONTCARE,
		SMC_DATACAP_DONTCARE,
		SMC_PAR_1BPCK,
		SMC_INVQUAL_INVERTED);

	SGPIO_configSliceMuxReg(CHIP_SELECT_CFG0_INTERNAL.sliceId, sliceMuxCfg);
	
	muxCfg = SGPIO_makeMuxConfig(
		MC_CLK_INTERNAL,
		MC_CLKSRC_PIN_DONTCARE,
		MC_CLKSRC_SLICE_DONTCARE,
		MC_QUALMODE_SLICE,
		MC_QUALPIN_DONTCARE,
		MC_QUALSLICE_I_D,
		MC_CONCAT_DATA,
		MC_CONCAT_TWO_SLICES);

	SGPIO_configMuxReg(CHIP_SELECT_CFG0_INTERNAL.sliceId, muxCfg);

	outMuxCfg = SGPIO_makeOutMuxConfig(OMC_DOUTM1, OMC_GPIO_OE);

	SGPIO_configOutMuxReg(CHIP_SELECT_CFG0_INTERNAL.pinId, outMuxCfg);

	if(CHIP_SELECT_CFG0_INTERNAL.sliceFunc == SGPIO_OUTPUT_PIN)
		SGPIO_setOeReg(CHIP_SELECT_CFG0_INTERNAL.pinId, outMuxCfg);

	SGPIO_setBitCountReg(CHIP_SELECT_CFG0_INTERNAL.sliceId, (config->wordLenght)+2);

	SGPIO_setCountReloadReg(CHIP_SELECT_CFG0_INTERNAL.sliceId, SGPIO_IP_CLOCK/(2*(config->bitRateHz)));

	SGPIO_writeDataReg(CHIP_SELECT_CFG0_INTERNAL.sliceId, cs0upWaveform);
	
	/**********************************************************************
	* make the static configuration for the Clock, slice D		
	*
	**********************************************************************/
	SGPIO_disableSlice(CLOCK_CFG0.sliceId);
	
	sliceMuxCfg = SGPIO_makeSliceMuxConfig(
		SMC_MATCH_DATA,
		SMC_CLKCAP_DONTCARE,
		SMC_CLKGEN_COUNTER,
		SMC_INVOUT_DONTCARE,
		SMC_DATACAP_DONTCARE,
		SMC_PAR_1BPCK,
		SMC_INVQUAL_INVERTED);
	
	SGPIO_configSliceMuxReg(CLOCK_CFG0.sliceId, sliceMuxCfg);
	
	muxCfg = SGPIO_makeMuxConfig(
		MC_CLK_INTERNAL,
		MC_CLKSRC_PIN_DONTCARE,
		MC_CLKSRC_SLICE_DONTCARE,
		MC_QUALMODE_SLICE,
		MC_QUALPIN_DONTCARE,
		MC_QUALSLICE_A_D,
		MC_CONCAT_DATA,
		MC_CONCAT_SELF_LOOP);
	
	SGPIO_configMuxReg(CLOCK_CFG0.sliceId, muxCfg);
	
	outMuxCfg = SGPIO_makeOutMuxConfig(OMC_DOUTM1, OMC_GPIO_OE);

	SGPIO_configOutMuxReg(CLOCK_CFG0.pinId, outMuxCfg);

	if(CLOCK_CFG0.sliceFunc == SGPIO_OUTPUT_PIN)
		SGPIO_setOeReg(CLOCK_CFG0.pinId, outMuxCfg);

	SGPIO_setBitCountReg(CLOCK_CFG0.sliceId, (config->wordLenght)*2);

	SGPIO_setCountReloadReg(CLOCK_CFG0.sliceId, SGPIO_IP_CLOCK/(2*(config->bitRateHz)));

	SGPIO_writeDataReg(CLOCK_CFG0.sliceId, 0xAAAAAAAA);
	
	/**********************************************************************
	* make the static configuration for the Data out pin, slice E		
	*
	**********************************************************************/
	SGPIO_disableSlice(DATA_OUT_CFG0.sliceId);

	sliceMuxCfg = SGPIO_makeSliceMuxConfig(
		SMC_MATCH_DATA,
		SMC_CLKCAP_RISING, 
		SMC_CLKGEN_COUNTER,
		SMC_INVOUT_DONTCARE,
		SMC_DATACAP_DONTCARE,
		SMC_PAR_1BPCK,
		SMC_INVQUAL_INVERTED);

	SGPIO_configSliceMuxReg(DATA_OUT_CFG0.sliceId, sliceMuxCfg);
	
	muxCfg = SGPIO_makeMuxConfig(
		MC_CLK_INTERNAL,
		MC_CLKSRC_PIN_DONTCARE,
		MC_CLKSRC_SLICE_DONTCARE,
		MC_QUALMODE_SLICE,
		MC_QUALPIN_DONTCARE,
		MC_QUALSLICE_A_D,
		MC_CONCATEN_DONTCARE,
		MC_CONCAT_ORDER_DONTCARE);

	SGPIO_configMuxReg(DATA_OUT_CFG0.sliceId, muxCfg);
	
	outMuxCfg = SGPIO_makeOutMuxConfig(OMC_DOUTM1, OMC_GPIO_OE);
	// changed for testing to clk output
	// outMuxCfg = SGPIO_makeOutMuxConfig(OMC_CLKOUT, OMC_GPIO_OE);
	
	SGPIO_configOutMuxReg(DATA_OUT_CFG0.pinId, outMuxCfg);

	if(DATA_OUT_CFG0.sliceFunc == SGPIO_OUTPUT_PIN)
		SGPIO_setOeReg(DATA_OUT_CFG0.pinId, outMuxCfg);
			
	SGPIO_setBitCountReg(DATA_OUT_CFG0.sliceId, (config->wordLenght));

	SGPIO_setCountReloadReg(DATA_OUT_CFG0.sliceId, SGPIO_IP_CLOCK/(config->bitRateHz));

	SGPIO_writeDataReg(DATA_OUT_CFG0.sliceId, 0x0);

	SGPIO_writeDataShadowReg(DATA_OUT_CFG0.sliceId, 0x0);	
	
	/**********************************************************************
	* make the static configuration for the Data in pin, slice F		
	*
	**********************************************************************/
	SGPIO_disableSlice(DATA_IN_CFG0.sliceId);
	
	sliceMuxCfg = SGPIO_makeSliceMuxConfig(
		SMC_MATCH_DATA,
		SMC_CLKCAP_FALLING, 
		SMC_CLKGEN_COUNTER,
		SMC_INVOUT_DONTCARE,
		SMC_DATACAP_RISING,
		SMC_PAR_1BPCK,
		SMC_INVQUAL_DONTCARE);

	SGPIO_configSliceMuxReg(DATA_IN_CFG0.sliceId, sliceMuxCfg);
	
	muxCfg = SGPIO_makeMuxConfig(
		MC_CLK_INTERNAL,
		MC_CLKSRC_PIN_DONTCARE,
		MC_CLKSRC_SLICE_DONTCARE,
		MC_QUALMODE_DONTCARE,
		MC_QUALPIN_DONTCARE,
		MC_QUALSLICE_DONTCARE,
		MC_EXT_DATA_PIN,
		MC_CONCAT_ORDER_DONTCARE);

	SGPIO_configMuxReg(DATA_IN_CFG0.sliceId, muxCfg);
		
	outMuxCfg = SGPIO_makeOutMuxConfig(OMC_DOUTM1, OMC_GPIO_OE);
	SGPIO_configOutMuxReg(DATA_IN_CFG0.pinId, outMuxCfg);	
	//  can be modified to test the clk output
	// 	outMuxCfg = SGPIO_makeOutMuxConfig(OMC_CLKOUT, OMC_GPIO_OE);
	// 	SGPIO_configOutMuxReg((SGPIO_Pin)5, outMuxCfg);
	
	if(DATA_IN_CFG0.sliceFunc == SGPIO_OUTPUT_PIN)
		SGPIO_setOeReg(DATA_IN_CFG0.pinId, outMuxCfg);
	//  can be modified to test the clk output
	// SGPIO_setOeReg((SGPIO_Pin)5, outMuxCfg);
	
	SGPIO_setBitCountReg(DATA_IN_CFG0.sliceId, (config->wordLenght));

	SGPIO_setCountReloadReg(DATA_IN_CFG0.sliceId, SGPIO_IP_CLOCK/(config->bitRateHz));

	SGPIO_writeDataReg(DATA_IN_CFG0.sliceId, 0x0);

	SGPIO_writeDataShadowReg(DATA_IN_CFG0.sliceId, 0x0);	
	
	// enable input bit match interrupt for slice DATA IN
	LPC_SGPIO->SET_EN_3 = (1<<DATA_IN_CFG0.pinId);		

	
}
