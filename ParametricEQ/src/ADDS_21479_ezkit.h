/*********************************************************************************

 Copyright(c) 2012 Analog Devices, Inc. All Rights Reserved.

 This software is proprietary and confidential.  By using this software you agree
 to the terms of the associated Analog Devices License Agreement.

 *********************************************************************************/

/*
 * NAME:     ADDS_21479_EzKit.h
 * PURPOSE:  Header file with definitions use in the C-based talk-through examples
 */

#ifdef __ECC__


/* include files */

#include <stdio.h>     /* Get declaration of puts and definition of NULL. */
#include <stdint.h>    /* Get definition of uint32_t. */
#include <stdlib.h>    /* Get definition of malloc. */
#include <assert.h>    /* Get the definition of support for standard C asserts. */
#include <builtins.h>  /* Get definitions of compiler builtin functions */
#include <platform_include.h>      /* System and IOP register bit and address definitions. */
#include <processor_include.h>
#include <services/int/adi_int.h>  /* Interrupt Handler API header. */
#include "adi_initialize.h"
#include <sru.h>
#include "ad1939.h"
#include "SPORT1_Macros.h"


#include <cycle_count.h>
#include <cycles.h>

/* Insert C Definitions here.... */




/* Function prototypes for this talkthrough code */
void	Setup_ADSP21479(void);
void	initPLL(void);
void    initExternalMemory(void);
void 	InitDAI(void);
static void 	clearDAIpins(void);
void 	init1939viaSPI(void);
void 	enable_SPORT01_MCM_mode(void);
void 	enable_SPORT01_DMA_channels(void);
void 	enable_SPORT23_MCM_mode(void);
void 	enable_SPORT23_DMA_channels(void);
void	process_AD1939_samples(uint32_t , void *);
void	Receive_ADC_Samples(void);
void	Transmit_DAC_Samples(void);

static void 	SetupSPI1939(unsigned int);
static void 	DisableSPI1939();
static void 	Configure1939Register(unsigned char,unsigned char,unsigned char,unsigned int);
static unsigned char Get1939Register(unsigned char,unsigned int);

static void 	Delay(int i);
void	SinTableInit(void);

/* Insert global variable definitions here */

extern void Delay (int i);
extern float Sine_Table[SINETABLESIZE];
extern int Sine_Index;

extern int tx0a_buf[8];
extern int rx1a_buf[8];

extern volatile float * DelayLine;
extern volatile int Index;

/* Sampling frequency definition */
extern int fs;
extern volatile bool ready;

/* input audio channels */
extern 	float			Left_Channel_In1;       /* Input values from the AD1939 internal stereo ADCs */
extern 	float 			Right_Channel_In1;
extern	float			Left_Channel_In2;
extern  float			Right_Channel_In2;

/*output audio channels */
extern 	float			Left_Channel_Out1;      /* Output values for the 4 AD1939 internal stereo DACs */
extern 	float 			Left_Channel_Out2;
extern 	float 			Left_Channel_Out3;
extern 	float 			Left_Channel_Out4;
extern 	float			Right_Channel_Out1;
extern 	float			Right_Channel_Out2;
extern 	float			Right_Channel_Out3;
extern 	float			Right_Channel_Out4;



#else
/* Insert Assembly Definitions here.... */

#endif


#define SPIB_MODE (CPHASE | CLKPL)
#define AD1939_CS DS0EN
//#define AD1939_CS DS1EN

#define CLEAR_DSXEN_BITS 0xF00


#define SELECT_SPI_SLAVE(select) (*pSPIFLG &= ~(spiselect<<8))
#define DESELECT_SPI_SLAVE(select) (*pSPIFLG |= (spiselect<<8))

/* AD1939 TDM Timeslot Definitions */
#define     Internal_ADC_L1    0
#define     Internal_ADC_R1    1
#define     Internal_ADC_L2    2
#define     Internal_ADC_R2    3
#define     AUX_ADC_L1         4
#define     AUX_ADC_R1         5
#define     AUX_ADC_L2         6
#define     AUX_ADC_R2         7

#define     Internal_DAC_L1    0
#define     Internal_DAC_R1    1
#define     Internal_DAC_L2    2
#define     Internal_DAC_R2    3
#define     Internal_DAC_L3    4
#define     Internal_DAC_R3    5
#define     Internal_DAC_L4    6
#define     Internal_DAC_R4    7


