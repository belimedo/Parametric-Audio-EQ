/*********************************************************************************

 Copyright(c) 2012 Analog Devices, Inc. All Rights Reserved.

 This software is proprietary and confidential.  By using this software you agree
 to the terms of the associated Analog Devices License Agreement.

 *********************************************************************************/
/******************************************************************************************************
/                                                                                                     /
/                            AD1939 - SPORT0 & SPORT1 Initialization Routines                         /
/                                                                                                     /
/    Configures SPORT1 RX and SPORT0 TX control and DMA registers for multichannel operation          /
/                                                                                                     /
/ *************************************************************************************************** /
/                                                                                                     /
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


#include "ADDS_21479_EzKit.h"
//#include <def21469.h>
//asm("#include <def21469.h>");


int	 	rx1a_buf[8];   /* receive buffer A (DMA)*/
int 	tx0a_buf[8];   /* transmit buffer A (DMA)*/	  		
    
				  		
/* TCB = "Transfer Control Block" */
/* TCB format:    ECx (length of destination buffer),
				  EMx (destination buffer step size),
				  EIx (destination buffer index (initialized to start address)),
				  GPx ("general purpose"),
				  CPx ("Chain Point register"; points to last address (IIx) of
			   								   next TCB to jump to
				                               upon completion of this TCB.),
				  Cx  (length of source buffer),
				  IMx (source buffer step size),
				  IIx (source buffer index (initialized to start address))       */

int 	rcv1a_tcb[4]  = {0, 8, 1, (int) rx1a_buf};	/* SPORT1 receive tcb */
int 	xmit0a_tcb[4] = {0, 8, 1, (int) tx0a_buf};  /* SPORT0 transmit tcb */



void 	enable_SPORT01_MCM_mode()
{
	/* Clear out SPORT 0/1 registers */
	*pSPMCTL0 = 0;
	*pSPMCTL1 = 0;
    *pSPCTL0 = 0;
    *pSPCTL1 = 0;
	
    /* External clock and frame syncs generated by AD1939 */
   	*pDIV0 = 0x00000000;  // Transmitter (SPORT0)
    *pDIV1 = 0x00000000;  // Receiver (SPORT1) at 12.288 MHz SCLK and 48 kHz sample rate


	/* SPORT0 and SPORT1 are being operated in "multichannel" mode.
	This is synonymous with TDM mode which is the operating mode for the AD1939 */					

	/* SPORT 0&1  Miscellaneous Control Bits Registers */
	/* SP01MCTL = 0x000000E2,  Hold off on MCM enable, and number of TDM slots to 8 active channels */
	/* Multichannel Frame Delay=1, Number of Channels = 8, LB disabled */
	//*pSPMCTL01 = NCH_8 | MFD1;
	*pSPMCTL0 = NCH7 | MFD1;
	*pSPMCTL1 = NCH7 | MFD1;
	
	/* Enable chain pointer registers before enabling DMA */
	xmit0a_tcb[0] = *pCPSP0A = (((int) xmit0a_tcb + 3) & 0x7FFFF) | (1<<19);
	rcv1a_tcb[0]  = *pCPSP1A = (((int) rcv1a_tcb  + 3) & 0x7FFFF )| (1<<19);

	/* sport1 control register set up as a receiver in MCM */
	/* sport 1 control register SPCTL1 = 0x000C01F0 */
	/* externally generated SCLK1 and RFS1 */
	*pSPCTL1 = 	SCHEN_A | SDEN_A | SLEN32; 
	
	/* sport0 control register set up as a transmitter in MCM */
	/* sport 0 control register, SPCTL2 = 0x000C01F0 */
	//*pSPCTL0 = 	SCHEN_A | SDEN_A | SPTRAN | FSR | SLEN32;
	*pSPCTL0 = 	SCHEN_A | SDEN_A | SPTRAN | SLEN32;

	/* sport1 receive & sport0 transmit multichannel word enable registers */
	/* enable receive channels 0-7 */
	/* enable transmit channels 0-7 */
	*pMR1CS0 = *pMT0CS0 =0x000000FF;   

	/* sport1 & sport0 receive & transmit multichannel companding enable registers */
	/* no companding for our 8 active timeslots*/
	/* no companding on SPORT1 receive */
	/* no companding on SPORT0 transmit */
	*pMR1CCS0 = *pMT0CCS0 = 0;
	
	
}	




#ifdef DEBUG
/* TDM audio frame/ISR counter, for debug purposes */
int			audio_frame_timer = 0;
#endif


