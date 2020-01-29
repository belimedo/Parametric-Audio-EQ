/*********************************************************************************

 Copyright(c) 2012 Analog Devices, Inc. All Rights Reserved.

 This software is proprietary and confidential.  By using this software you agree
 to the terms of the associated Analog Devices License Agreement.

 *********************************************************************************/

/*
 * NAME:     initSRU.c (Sample-based Talkthrough)
 * PURPOSE:  Talkthrough framework for sending and receiving audio samples on the
 *           21469 EZ-KIT Lite AD1939 Codec usin SPORTs 0/1
 * USAGE:    This file initializes the DAI for accessing the ADC and DACs.
 */


/*-----------------------------------------------------------------------------

   *** 21469 EZ-KIT TDM MODE AD1939 Single Codec ADC/DAC ROUTING OVERVIEW ***
   (AD1939 AUX TDM mode implementation uses 4 DAI pins and 2 internal SPORTs)

                          ..............................................
 AD1939                   :SHARC ADSP-21469    
                          :
   ADC  12.288 MHz DAI_P07:  o-DAI_PB07_O--------------*-----o-SPORT0_CLK_I
   BCLK >---------------->#--o-                  +---- | ----o-SPORT0_FS_I
   OUT                    :  o-PBEN07_I=LOW      |     |  +--o-SPORT0_DA_O
                          :                      |     |  |
   ADC   48/96 kHz DAI_P08:  o-DAI_PB08_O--------*     |  |
   FS   >---------------->#--o-                  |     |  |
   OUT                    :  o-PBEN08_I=LOW      |     |  |
                          :                      |     |  | 
   ADC1            DAI_P05:  o-DAI_PB05_O-----+  |     |  |
   DATA >---------------->#--o-               |  |     |  |
   OUT                    :  o PBEN05_I=LOW   |  |     |  |
                          :                   |  |     |  |
   ADC2                   :  o-DAI_PB06_O     |  |     |  |
   DATA >                >#--o-               |  |     |  |
   OUT                    :  o PBEN06_I=LOW   |  |     |  |
                          :                   |  |     |  |    
   DAC1                   :  o-               |  |     |  |
   BCLK <                <#--o-DAI_PB13_I     |  |     |  | 
   IN                     :  o-PBEN13_I=HIGH  |  |     |  |
                          :                   |  |     |  |
   DAC1                   :  o-               |  |     +---->o-SPORT1_CLK_I
   FS   <                <#--o-DAI_PB14_I     |  +---------->o-SPORT1_FS_I
   IN                     :  o PBEN14_I=HIGH  +------------->o-SPORT1_DA_I
                          :                               |  o-SPORT1_DB_I
   DAC1            DAI_P12:  o                            |   
   DATA <----------------<#--o-DAI_PB12_I<----------------+   
   IN                     :  o PBEN12_I=HIGH               
                          :                                
   DAC2                   :  o-                            
   DATA <                <#--o-DAI_PB11_I<---
   IN                     :  o PBEN11_I=HIGH         
                          :                                  o-SPORT2_CLK_I
   DAC3                   :  o-                              o-SPORT2_FS_I
   DATA <                <#--o-DAI_PB10_I<---                o-SPORT2_DA_O
   IN                     :  o PBEN10_I=HIGH                 o-SPORT2_DB_O
                          :                                
   DAC4                   :  o-                            
   DATA <                <#--o-DAI_PB09_I<---
   IN                     :  o PBEN09_I=HIGH
                          :
                          ..............................................

-----------------------------------------------------------------------------*/
                          
/* The following definition allows the SRU macro to check for errors. Once the routings have
 * been verified, this definition can be removed to save some program memory space.
 * The preprocessor will issue a warning stating this when using the SRU macro without this
 * definition
 */
#define SRUDEBUG  /* Check SRU Routings for errors. */


#include "ADDS_21479_EzKit.h"
#include <sru.h>
#include <def21469.h>


static void clearDAIpins()
{
/*------------------------------------------------------------------------
 *  Tie the pin buffer inputs LOW for all DAI pins.  Even though
 *  these pins are inputs to the SHARC, tying unused pin buffer inputs
 *  LOW is "good coding style" to eliminate the possibility of
 *  termination artifacts internal to the IC.  Note that signal
 *  integrity is degraded only with a few specific SRU combinations.
 *  In practice, this occurs VERY rarely, and these connections are
 *  typically unnecessary.  This is GROUP D
 */
    SRU(LOW, DAI_PB01_I);
    SRU(LOW, DAI_PB02_I);
    SRU(LOW, DAI_PB03_I);
    SRU(LOW, DAI_PB04_I);
    SRU(LOW, DAI_PB05_I);
    SRU(LOW, DAI_PB06_I);
    SRU(LOW, DAI_PB07_I);
    SRU(LOW, DAI_PB08_I);
    SRU(LOW, DAI_PB09_I);
    SRU(LOW, DAI_PB10_I);
    SRU(LOW, DAI_PB11_I);
    SRU(LOW, DAI_PB12_I);
    SRU(LOW, DAI_PB13_I);
    SRU(LOW, DAI_PB14_I);
    SRU(LOW, DAI_PB15_I);
    SRU(LOW, DAI_PB16_I);
    SRU(LOW, DAI_PB17_I);
    SRU(LOW, DAI_PB18_I);
    SRU(LOW, DAI_PB19_I);
    SRU(LOW, DAI_PB20_I);


/*  Tie the pin buffer enable inputs LOW for all DAI pins so
 *   that they are always input pins.  This is GROUP F.
 */
    SRU(LOW, PBEN01_I);
    SRU(LOW, PBEN02_I);
    SRU(LOW, PBEN03_I);
    SRU(LOW, PBEN04_I);
    SRU(LOW, PBEN05_I);
    SRU(LOW, PBEN06_I);
    SRU(LOW, PBEN07_I);
    SRU(LOW, PBEN08_I);
    SRU(LOW, PBEN09_I);
    SRU(LOW, PBEN10_I);
    SRU(LOW, PBEN11_I);
    SRU(LOW, PBEN12_I);
    SRU(LOW, PBEN13_I);
    SRU(LOW, PBEN14_I);
    SRU(LOW, PBEN15_I);
    SRU(LOW, PBEN16_I);
    SRU(LOW, PBEN17_I);
    SRU(LOW, PBEN18_I);
    SRU(LOW, PBEN19_I);
    SRU(LOW, PBEN20_I);
}


void InitDAI()
{
    clearDAIpins();


/*  Connect the AD1939 ADCs: The AD1939 drives a BCLK output to DAI pin 7,
 *  a frame sync to DAI pin 8 and TDM rx data to DAI pins 5
 *
 *  Connect the TDM ADC stream to SPORT1, using data input A
 *  All four lines are always inputs to the SHARC so tie the pin
 *  buffer inputs and pin buffer enable inputs all low.
 */

    SRU(DAI_PB07_O, SPORT1_CLK_I);   /* DAIP17 (RSCLK1) to SPORT1 CLK (CLK) */
    SRU(DAI_PB07_O, SPORT0_CLK_I);   /* DAIP7 (RSCLK1) to SPORT0 CLK (CLK)*/

    SRU(DAI_PB08_O, SPORT1_FS_I);    /* DAIP8 (RFS1) to SPORT1 FS (FS)*/
    SRU(DAI_PB08_O, SPORT0_FS_I);    /* DAIP8 (RFS1) to SPORT0 FS (FS)*/

    SRU(DAI_PB05_O, SPORT1_DA_I);    /* DAIP5 (DR1PRI) to SPORT1 DA (RX) */

    

/*  Connect the AD1939 DACs in TDM mode to SPORT0:
 *  The clock and frame sync inputs are provided by the ADCs
 *
 *  All DAC connections are always outputs from the SHARC
 *  so tie the pin buffer enable inputs all high.
 *  Connect the TDM DAC stream to SPORT0 A via DAI pin 12
 */

    SRU(HIGH, PBEN12_I);
    SRU(SPORT0_DA_O, DAI_PB12_I);    // DAIP 13 (DT1PRI)to SPORT0 DA (TX)
    


/* Route SPI signals to AD1939 Control Port. */

    SRU(SPI_MOSI_O, DPI_PB01_I);     /*Connect MOSI to DPI PB1. */
    SRU(DPI_PB02_O, SPI_MISO_I);     /*Connect DPI PB2 to MISO. */
    SRU(SPI_CLK_O, DPI_PB03_I);      /*Connect SPI CLK to DPI PB3.*/
    SRU(SPI_FLG0_O, DPI_PB04_I);     /*Connect SPI FLAG0 to DPI PB4.*/
    
    

/* Tie pin buffer enable from SPI peipheral to determine whether they are
 *  inputs or outputs
 */
    SRU(SPI_MOSI_PBEN_O, DPI_PBEN01_I);
    SRU(SPI_MISO_PBEN_O, DPI_PBEN02_I);
    SRU(SPI_CLK_PBEN_O, DPI_PBEN03_I);
    SRU(SPI_FLG0_PBEN_O, DPI_PBEN04_I);

}

