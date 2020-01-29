/*********************************************************************************

 Copyright(c) 2012 Analog Devices, Inc. All Rights Reserved.

 This software is proprietary and confidential.  By using this software you agree
 to the terms of the associated Analog Devices License Agreement.

 *********************************************************************************/
/*
 *NAME:     init1939viaSPI.c (Sample-based Talk-through)
 *PURPOSE:  Programs the control registers on the AD1939 via SPI accesses.
 *USAGE:    This file contains the subroutines for accessing the AD1939 control
 *          registers via SPI, and sets up the AD1939s for TDM serial communication.
 */

#include "ADDS_21479_EzKit.h"

//extern int fs;

/*NOTE: To select ADC/DAC Sample Rate of either 48 kHz or 96 kHz, choose one of
 * the following 2 macros to set the sample rate.  Uncomment only one at a time. */

#define USE_48_KHZ_SAMPLE_RATE
//#define USE_96_KHZ_SAMPLE_RATE

/* Setup the SPI parameters here in a buffer first */
unsigned char ConfigParam1939 [] = {
            (AD1939_ADDR), CLKCTRL0, DIS_ADC_DAC | INPUT256 | PLL_IN_MCLK | MCLK_OUT_OFF | PLL_PWR_DWN,
            (AD1939_ADDR), CLKCTRL1, DAC_CLK_PLL | ADC_CLK_PLL | DIS_VREF,
#ifdef USE_48_KHZ_SAMPLE_RATE
            (AD1939_ADDR), DACCTRL0, DAC_FMT_TDM | DAC_BCLK_DLY_1 | DAC_SR_48K,
#endif
#ifdef USE_96_KHZ_SAMPLE_RATE
            (AD1939_ADDR), DACCTRL0, DAC_FMT_TDM | DAC_BCLK_DLY_1 | DAC_SR_96K,
#endif
            (AD1939_ADDR), DACCTRL1, DAC_LRCLK_POL_INV | DAC_BCLK_POL_INV | DAC_CHANNELS_8 | DAC_LATCH_MID,
            (AD1939_ADDR), DACCTRL2, DAC_WIDTH_24,
            (AD1939_ADDR), DACVOL_L1, DACVOL_MAX, 
            (AD1939_ADDR), DACVOL_R1, DACVOL_MAX,
            (AD1939_ADDR), DACVOL_L2, DACVOL_MAX,
            (AD1939_ADDR), DACVOL_R2, DACVOL_MAX,
            (AD1939_ADDR), DACVOL_L3, DACVOL_MAX,
            (AD1939_ADDR), DACVOL_R3, DACVOL_MAX,
            (AD1939_ADDR), DACVOL_L4, DACVOL_MAX,
            (AD1939_ADDR), DACVOL_R4, DACVOL_MAX,
#ifdef USE_48_KHZ_SAMPLE_RATE            
            (AD1939_ADDR), ADCCTRL0, ADC_SR_48K,
#endif
#ifdef USE_96_KHZ_SAMPLE_RATE            
            (AD1939_ADDR), ADCCTRL0, ADC_SR_96K,
#endif           
            (AD1939_ADDR), ADCCTRL1, ADC_LATCH_MID | ADC_FMT_AUX | ADC_WIDTH_24,
            (AD1939_ADDR), ADCCTRL2, ADC_LRCLK_FMT_PULSE | ADC_BCLK_MASTER | ADC_LRCLK_MASTER | ADC_BCLK_SRC_INTERNAL | ADC_LRCLK_POL_INV | ADC_BCLK_POL_INV | ADC_CHANNELS_8,
            (AD1939_ADDR), CLKCTRL0, DIS_ADC_DAC | PLL_IN_MCLK | MCLK_OUT_OFF | INPUT256 | PLL_PWR_UP,
            (AD1939_ADDR), CLKCTRL0, ENA_ADC_DAC | PLL_IN_MCLK | MCLK_OUT_OFF | INPUT256 | PLL_PWR_UP,

            } ;
#ifdef USE_48_KHZ_SAMPLE_RATE
	int fs = 48000;
#endif

#ifdef USE_96_KHZ_SAMPLE_RATE
	int fs = 96000;
#endif


unsigned char AD1938_Regs_Read[sizeof(ConfigParam1939) / 3];

volatile int spiFlag ;

/*
 * Set up the SPI port to access the AD1939
 * Call with SPI flag to use
 */

static void SetupSPI1939(unsigned int SPI_Flag)
{
/* Configure the SPI Control registers */
/* First clear a few registers */
    *pSPICTL = (TXFLSH | RXFLSH) ;
    *pSPIFLG = 0;

/* Setup the baud rate to 1MHz */
    //*pSPIBAUD = 100;
    *pSPIBAUD = 25;

/* Setup the SPI Flag register using the Flag specified in the call */
    *pSPIFLG = (CLEAR_DSXEN_BITS|SPI_Flag);

/* Now setup the SPI Control register */
    *pSPICTL = (SPIEN | SPIMS | WL8 | MSBF | TIMOD1 | CPHASE | CLKPL | SMLS|GM);
   
  
}


/* Disable the SPI Port */


static void DisableSPI1939(void)
{
    *pSPICTL = (TXFLSH | RXFLSH);
}



/*  Send a word to the AD1939 via SPI
 *  Call with the AD1939 address, register address, register data,
 *  and SPI flag to use
 */


static void Configure1939Register (unsigned char rwaddr,unsigned char regaddr,unsigned char regdata, unsigned int spiselect)
{
    int i,j;
    unsigned char val[3];

    SELECT_SPI_SLAVE(spiselect);

    val[0]=WR(rwaddr);
    val[1]=regaddr;
    val[2]=regdata;
    
   


    for(i=0;i<3;i++)
        {

            *pTXSPI = val[i] ;
           // Delay(136);
    /* Wait for the SPI to indicate that it has finished. */
            while ((*pSPISTAT & TXS))
            {NOP();}
        }

    /* Wait for the SPI to indicate that it has finished. */
        while (!(*pSPISTAT & SPIFE))
        {NOP();}
    
    Delay(10);

    DESELECT_SPI_SLAVE(spiselect);

}


/*
 * Receive a register setting from the AD1939
 * Call with the register address, and SPI flag to use; returns register data
 */


static unsigned char Get1939Register (unsigned char regaddr, unsigned int spiselect)
{
    int i,j;
    unsigned char val[3];
    unsigned int tmp;
    
    SELECT_SPI_SLAVE(spiselect);

    val[0]=RD(AD1939_ADDR);
    val[1]=regaddr;
    val[2]=0;
    
    
    for(i=0;i<3;i++)
        {
            *pTXSPI = val[i] ;


    /* Wait for the SPI to indicate that it has finished. */
           while ((*pSPISTAT & TXS))
            {NOP();}


        }


    /*Wait for the SPI to indicate that it has finished.*/
    
    	while (!(*pSPISTAT & SPIF))
       {NOP();}
       while (!(*pSPISTAT & SPIFE))
       {NOP();}
    
    Delay(10);
    
    tmp = *pRXSPI;
    




    DESELECT_SPI_SLAVE(spiselect);




    return tmp;
}

int LockCount = 0;
unsigned int LockTest;

/* Set up all AD1939 registers via SPI */

void init1939viaSPI(void)
{
    int configSize = sizeof(ConfigParam1939);
    int i,j=0 ;
    unsigned char tmpA[sizeof(ConfigParam1939) / 3];
    

    /* Set up AD1939*/
    SetupSPI1939(AD1939_CS);

    /* Write register settings*/
    for(i = 0; i < configSize-3; i+=3)
    {
        Configure1939Register(ConfigParam1939[i], ConfigParam1939[i+1], ConfigParam1939[i+2], AD1939_CS);
        Delay(272);
        
        /* Read back register settings for debugging*/
        AD1938_Regs_Read[j++] = Get1939Register(ConfigParam1939[i+1], AD1939_CS);
        Delay(272);
    }
    
    /* Make sure the PLL is locked before enabling the CODEC.*/
    
    LockTest = Get1939Register(0x1, AD1939_CS);
    while (!(LockTest & AD1938_PLL_LOCK))
    {
    	LockTest = Get1939Register(CLKCTRL1, AD1939_CS);
    LockCount++;
    }
    
    for(i = configSize-3; i < configSize; i+=3)
    {
        Configure1939Register(ConfigParam1939[i], ConfigParam1939[i+1], ConfigParam1939[i+2], AD1939_CS);
        Delay(272);
        
        /* Read back register settings for debugging*/
        AD1938_Regs_Read[j++] = Get1939Register(ConfigParam1939[i+1], AD1939_CS);
        Delay(272);
    }






    DisableSPI1939();
}



/* Delay loop */
static void Delay(int i)
{
	 int j;
	    for(j=i ; j > 0; --j)
	        NOP();
}
