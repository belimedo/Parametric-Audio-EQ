/*********************************************************************************

 Copyright(c) 2012 Analog Devices, Inc. All Rights Reserved.

 This software is proprietary and confidential.  By using this software you agree
 to the terms of the associated Analog Devices License Agreement.

 *********************************************************************************/
/*This project is a talk-through example using the onboard AD1939 to acquire
 * and output an audio stream. The digital audio data is available for processing
 * in the file SPORT1_isr.c. The block size is 256 samples per audio channel.
 * The output on OUT2 is delayed and the user can expect to hear an echo effect.
 */

#include "ADDS_21479_EzKit.h"
#include "filter.h"
#include <math.h>

volatile float * DelayLine;
volatile int Index  = 0;

/* Global variable for filters */
filter_t filters[NUM_FILTERS];

/* This ensures music reproduction will function without a flaw (-NaN values in process_AD1939_samples function) */
volatile bool ready = 0;

/* Variables for demonstration purpose*/
volatile bool change = 0;
volatile bool write_to_file = 0;

/* Global gains for real-time gain change - demonstration purpose */
volatile float change_gain_1 = 6;
volatile float change_gain_2 = 6;
volatile float change_gain_3 = 6;
volatile float change_gain_4 = 6;
volatile float change_gain_5 = 6;

volatile int count = 0;
volatile int reduce = 10;
volatile float real_time_gain = 24;

/* Around 100 cycles are reduced in filter coefficient calculation with this optimization*/
#pragma optimize_for_speed

void	main()
{
	/* Initialize managed drivers and/or services at the start of main(). */
	adi_initComponents();

	/* Initialize SHARC PLL*/
	initPLL();

	/* Initialize DDR2 SDRAM controller to access memory */
	initExternalMemory();

	/* Initialize DAI because the SPORT and SPI signals need to be routed */
    InitDAI();

    /* This function will configure the AD1939 codec on the 21469 EZ-KIT */
    init1939viaSPI();

    /* Turn on SPORT0 TX and SPORT1 RX for Multichannel Operation */
	enable_SPORT01_MCM_mode();


	/* Install and enable a handler for the SPORT1 Receiver interrupt.*/
	adi_int_InstallHandler(ADI_CID_P3I,process_AD1939_samples,0,true);


	/* Enable multichannel operation (SPORT mode and DMA in standby and ready) */
	*pSPMCTL0 |= MCEA;
	*pSPMCTL1 |= MCEA;

	SinTableInit();

	/* Set up small delay buffer */
//	DelayLine = (float *) 0x000C4000;
	DelayLine =(float *) malloc(DELAYLINESIZE * sizeof(float));

	/* 	Filter type,  Center frequency, Q-factor, Gain */
	filters[0] = GenerateFilter1(LSF,	100.0, 	1.0,	-6);
	filters[1] = GenerateFilter1(PF, 	350.0, 	0.5,	6);
	filters[2] = GenerateFilter1(PF,	1000.0,	3,		12);
	filters[3] = GenerateFilter1(PF,	1500.0,	3,		-6);
	filters[4] = GenerateFilter1(HSF,	5000.0,	0.707,	-6);
	if(filters[4].a[0] != 0)
		ready = 1;
	/* File pointer for starting values*/
	FILE *fp;
	/* File pointer for changed values*/
	FILE *cfp;

	/* Write filter coefficients to file */
	fp  = fopen("test.txt", "w");
	for(int i = 0; i < NUM_FILTERS; i++)
		fprintf(fp,"%f#%f#%f\n%f#%f#%f\n",filters[i].a[0],filters[i].a[1],filters[i].a[2],filters[i].b[0],filters[i].b[1],filters[i].b[2]);
	fclose(fp);

	for (;;)
	  {
		/* This was already present here in example, it is a good way to determine if example is working */
		if(count==10)
			puts("Talkthrough is running successfully");

		/* To test real-time parameter change, without stopping in debug mode,
		 * with sample frequency of 48kHz change is every 10 seconds */
		if((count % 500000 == 0) && reduce >=0) {

			real_time_gain-=4;
			/* At frequency of 300 Hz we will have 3dB weaker gain than wanted (filter response is that way)*/
			filters[0] = (GenerateFilter1(LSF,300.0,5,real_time_gain));
			reduce--;
		}
		/* If flag change is set to 1, every filter gain is changed. Any other parameter can be changed,
		 * but change in gain is easiest to notice then for demonstration this change is used */
		if(change) {

			filters[0] = GenerateFilter1(LSF,	100.0, 	1.0,	change_gain_1);
			filters[1] = GenerateFilter1(PF, 	350.0, 	0.5,	change_gain_2);
			filters[2] = GenerateFilter1(PF,	1000.0,	3,		change_gain_3);
			filters[3] = GenerateFilter1(PF,	1500.0,	3,		change_gain_4);
			filters[4] = GenerateFilter1(HSF,	5000.0,	0.707,	change_gain_5);
			change = 0;
		}
		/* Music playing will pause until writing to file is finished because it requires more than 9000 cycles*/
		if(write_to_file) {

			cfp = fopen("changed.txt","w");
			for(int i = 0; i < NUM_FILTERS; i++)
					fprintf(cfp,"%f#%f#%f\n%f#%f#%f\n",filters[i].a[0],filters[i].a[1],filters[i].a[2],filters[i].b[0],filters[i].b[1],filters[i].b[2]);
			fclose(fp);
			write_to_file = 0;
		}
		count++;
    	asm("idle;");
      }
}

