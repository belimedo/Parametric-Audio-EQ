/*********************************************************************************

 Copyright(c) 2012 Analog Devices, Inc. All Rights Reserved.

 This software is proprietary and confidential.  By using this software you agree
 to the terms of the associated Analog Devices License Agreement.

 *********************************************************************************/

/******************************************************************************************************
/                                                                                                     /
/                            AD1939 - SPORT1 RX INTERRUPT SERVICE ROUTINE                             /
/                                                                                                     /
/    Receives input data from the AD1939 ADCs via SPORT1 and transmits processed audio data 	      /
/    back out to the four AD1939 Stereo DACs/Line Outputs through SPORT0.                             /
/                                                                                                     /
*******************************************************************************************************
/                                                                                                     /
/   This Serial Port 1 Recieve Interrupt Service Routine performs arithmetic computations on          /
/   the SPORT1 receive DMA buffer (rx1a_buf) and places results to SPORT0 transmit                    /
/   DMA buffer (tx0a_buf)                                                                             /
/                                                                                                     /
/  rx1a_buf[8] - DSP SPORT recieve buffer - AD1939                                                    /
/  Slot # Description                             DSP Data Memory Address                             /
/  ------ --------------------------------------  --------------------------------------------------  /
/  0      Internal ADC 0 Left Channel             DM(_rx1a_buf + 0) = DM(_rx1a_buf + Internal_ADC_L1) /
/  1      Internal ADC 0 Right Channel            DM(_rx1a_buf + 1) = DM(_rx1a_buf + Internal_ADC_R1) /
/  2      Internal ADC 1 Left Channel             DM(_rx1a_buf + 2) = DM(_rx1a_buf + Internal_ADC_L2) /
/  3      Internal ADC 1 Right Channel            DM(_rx1a_buf + 3) = DM(_rx1a_buf + Internal_ADC_R2) /
/  4      External Auxilliary ADC 1 Left Chan.    DM(_rx1a_buf + 4) = DM(_rx1a_buf + AUX_DAC_L1)      /
/  5      External Auxilliary ADC 1 Right Chan.   DM(_rx1a_buf + 5) = DM(_rx1a_buf + AUX_DAC_R1)      /
/  6      External Auxilliary ADC 2 Left Chan.    DM(_rx1a_buf + 6) = DM(_rx1a_buf + AUX_DAC_L2)      /
/  7      External Auxilliary ADC 2 Right Chan.   DM(_rx1a_buf + 7) = DM(_rx1a_buf + AUX_DAC_R2)      /
/                                                                                                     /
/  tx0a_buf[8] - DSP SPORT transmit buffer - AD1939                                                   /
/  Slot # Description                             DSP Data Memory Address                             /
/  ------ --------------------------------------  --------------------------------------------------  /
/  0      Internal DAC 1 Left Channel             DM(_tx0a_buf + 0) = DM(_tx0a_buf + Internal_DAC_L1) /
/  1      Internal DAC 1 Right Channel            DM(_tx0a_buf + 1) = DM(_tx0a_buf + Internal_DAC_R1) /
/  2      Internal DAC 2 Left Channel             DM(_tx0a_buf + 2) = DM(_tx0a_buf + Internal_DAC_L2) /
/  3      Internal DAC 2 Right Channel            DM(_tx0a_buf + 3) = DM(_tx0a_buf + Internal_DAC_R2) /
/  4      Internal DAC 3 Left Channel             DM(_tx0a_buf + 4) = DM(_tx0a_buf + Internal_DAC_L3) /
/  5      Internal DAC 3 Right Channel            DM(_tx0a_buf + 5) = DM(_tx0a_buf + Internal_DAC_R3) /
/  6      Internal DAC 4 Left Channel             DM(_tx0a_buf + 6) = DM(_tx0a_buf + Internal_DAC_L4) /
/  7      Internal DAC 4 Right Channel            DM(_tx0a_buf + 7) = DM(_tx0a_buf + Internal_DAC_R4) /
/                                                                                                     /
******************************************************************************************************/

#include "adds_21479_ezkit.h"
#include <asm_sprt.h>
#include "SPORT1_Macros.h"

/*  The following macro def should be uncommented to test the DACs only with some generated pure tones... */
/*  Running at 96 KHz vs 48 KHz, you should hear a 1-octave difference in the tones */
/*  Tones are generated a 4K sine wave lookup table (integer divisor of 48K/96K/192K */

//#define GENERATE_DAC_PURE_TONES_TEST

.section /dm seg_dmda;

/* AD1939 stereo-channel data holders - used for DSP processing of audio data received from codec */
// input channels
.var 			_Left_Channel_In1;          /* Input values from the AD1939 internal stereo ADCs */			
.var 			_Right_Channel_In1;
.var 			_Left_Channel_In2;						
.var 			_Right_Channel_In2;

/*output channels */
.var			_Left_Channel_Out1;         /* Output values for the 4 AD1939 internal stereo DACs */
.var 			_Left_Channel_Out2;        
.var 			_Left_Channel_Out3;
.var 			_Left_Channel_Out4;
.var			_Right_Channel_Out1;
.var			_Right_Channel_Out2;
.var			_Right_Channel_Out3;
.var			_Right_Channel_Out4;

.var			_Left_Channel;              /* Can use these variables as intermediate results to next filtering stage */
.var            _Right_Channel;

.var			AD1939_audio_frame_timer;

.var 			sine4000[SINETABLESIZE] = {
									#include INPUTFILE
									};  
.var			TEMP_L4;
.var			Sine1_B_reg, Sine1_I_reg, Sine1_M_reg, Sine1_L_reg;
.var			Sine2_B_reg, Sine2_I_reg, Sine2_M_reg, Sine2_L_reg;
.var			Sine3_B_reg, Sine3_I_reg, Sine3_M_reg, Sine3_L_reg;
.var			Sine4_B_reg, Sine4_I_reg, Sine4_M_reg, Sine4_L_reg;

.global			_Left_Channel_In1;
.global			_Right_Channel_In1;
.global			_Left_Channel_In2;
.global			_Right_Channel_In2;

.global			_Left_Channel_Out1;
.global			_Right_Channel_Out1;
.global			_Left_Channel_Out2;
.global			_Right_Channel_Out2;
.global			_Left_Channel_Out3;
.global			_Right_Channel_Out3;
.global			_Left_Channel_Out4;
.global			_Right_Channel_Out4;

.extern			_rx1a_buf;
.extern			_tx0a_buf;
.extern			_rx1b_buf;
.extern			_tx0b_buf;


.section/pm seg_pmco;
//.section/pm seg_swco;

_Receive_ADC_Samples:
.global _Receive_ADC_Samples;
//void	Receive_ADC_Samples();
	/* get AD1939 left channel input samples, save to data holders for processing */
	r1 = -31;
	r0 = dm(_rx1a_buf + Internal_ADC_L1);	f0 = float r0 by r1;	dm(_Left_Channel_In1) = r0;	
	r0 = dm(_rx1a_buf + Internal_ADC_L2);	f0 = float r0 by r1;	dm(_Left_Channel_In2) = r0;		

	/* get AD1939 right channel input samples, save to data holders for processing */
	r0 = dm(_rx1a_buf + Internal_ADC_R1);	f0 = float r0 by r1;	dm(_Right_Channel_In1) = r0;	
	r0 = dm(_rx1a_buf + Internal_ADC_R2);	f0 = float r0 by r1;	dm(_Right_Channel_In2) = r0;			

	r0 = DM(AD1939_audio_frame_timer);
	r0 = r0 + 1;
	DM(AD1939_audio_frame_timer) = r0;
	
	leaf_exit;
_Receive_ADC_Samples.end:


_Transmit_DAC_Samples:
.global _Transmit_DAC_Samples;

	r1 = 31;

	/* output processed left ch audio samples to AD1939 */
	r0 = dm(_Left_Channel_Out1);	r0 = trunc f0 by r1;	dm(_tx0a_buf + Internal_DAC_L1) = r0;
	r0 = dm(_Left_Channel_Out2);	r0 = trunc f0 by r1;	dm(_tx0a_buf + Internal_DAC_L2) = r0;
	r0 = dm(_Left_Channel_Out3);	r0 = trunc f0 by r1;	dm(_tx0a_buf + Internal_DAC_L3) = r0;
	r0 = dm(_Left_Channel_Out4);	r0 = trunc f0 by r1;	dm(_tx0a_buf + Internal_DAC_L4) = r0;

	/* output processed right ch audio samples to AD1939 */
	r0 = dm(_Right_Channel_Out1); 	r0 = trunc f0 by r1;	dm(_tx0a_buf + Internal_DAC_R1) = r0;
	r0 = dm(_Right_Channel_Out2);	r0 = trunc f0 by r1;	dm(_tx0a_buf + Internal_DAC_R2) = r0;
	r0 = dm(_Right_Channel_Out3);	r0 = trunc f0 by r1;	dm(_tx0a_buf + Internal_DAC_R3) = r0;
	r0 = dm(_Right_Channel_Out4);	r0 = trunc f0 by r1;	dm(_tx0a_buf + Internal_DAC_R4) = r0;

#ifdef GENERATE_DAC_PURE_TONES_TEST
	Call make_DAC_Pure_Tones;
#endif

	leaf_exit;
_Transmit_DAC_Samples.end:


_SinTableInit:
.global _SinTableInit;

	/* use Compiler Scratch Registers for generating sine tones in assembly */
	DM(TEMP_L4) = L4;
	
	B4=sine4000;
	I4=sine4000;
	L4 = SINETABLESIZE;
	
	M4 = 40;

	DM(Sine1_B_reg) = B4;
	DM(Sine1_I_reg) = I4;
	DM(Sine1_L_reg) = L4;
	DM(Sine1_M_reg) = M4;
	
	M4 = 50;

	DM(Sine2_B_reg) = B4;
	DM(Sine2_I_reg) = I4;
	DM(Sine2_L_reg) = L4;
	DM(Sine2_M_reg) = M4;
	
	M4 = 60;

	DM(Sine3_B_reg) = B4;
	DM(Sine3_I_reg) = I4;
	DM(Sine3_L_reg) = L4;
	DM(Sine3_M_reg) = M4;

	M4 = 80;

	DM(Sine4_B_reg) = B4;
	DM(Sine4_I_reg) = I4;
	DM(Sine4_L_reg) = L4;
	DM(Sine4_M_reg) = M4;
	
	L4 = DM(TEMP_L4);

	leaf_exit;
_SinTableInit.end:


make_DAC_Pure_Tones:
	/* use Compiler Scratch Registers for generating sine tones in assembly */
	DM(TEMP_L4) = L4;
	
	/* generate sine1 tone from lookup table */
	B4 = DM(Sine1_B_reg);
	I4 = DM(Sine1_I_reg);
	L4 = DM(Sine1_L_reg);
	M4 = DM(Sine1_M_reg);
	
	r4 = dm(i4, m4);		
	dm(_tx0a_buf + Internal_DAC_L1) = r4;
	dm(_tx0a_buf + Internal_DAC_R1) = r4;
	
	DM(Sine1_I_reg) = I4;
	
	/* generate sine2 tone from lookup table */
	B4 = DM(Sine2_B_reg);
	I4 = DM(Sine2_I_reg);
	L4 = DM(Sine2_L_reg);
	M4 = DM(Sine2_M_reg);
	
	r4 = dm(i4, m4);		/* generate sine1 tone from lookup table */
	dm(_tx0a_buf + Internal_DAC_L2) = r4;
	dm(_tx0a_buf + Internal_DAC_R2) = r4;
	
	DM(Sine2_I_reg) = I4;
	
	/* generate sine3 tone from lookup table */
	B4 = DM(Sine3_B_reg);
	I4 = DM(Sine3_I_reg);
	L4 = DM(Sine3_L_reg);
	M4 = DM(Sine3_M_reg);
	
	r4 = dm(i4, m4);		
	dm(_tx0a_buf + Internal_DAC_L3) = r4;
	dm(_tx0a_buf + Internal_DAC_R3) = r4;
	
	DM(Sine3_I_reg) = I4;
	
	/* generate sine4 tone from lookup table */
	B4 = DM(Sine4_B_reg);
	I4 = DM(Sine4_I_reg);
	L4 = DM(Sine4_L_reg);
	M4 = DM(Sine4_M_reg);
	
	r4 = dm(i4, m4);		/* generate sine1 tone from lookup table */
	dm(_tx0a_buf + Internal_DAC_L4) = r4;
	dm(_tx0a_buf + Internal_DAC_R4) = r4;
	
	DM(Sine4_I_reg) = I4;
	
	L4 = DM(TEMP_L4);
	
	RTS;

Make_DAC_Pure_Tones.end:
