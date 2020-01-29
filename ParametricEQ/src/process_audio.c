/*********************************************************************************

 Copyright(c) 2012 Analog Devices, Inc. All Rights Reserved.

 This software is proprietary and confidential.  By using this software you agree
 to the terms of the associated Analog Devices License Agreement.

 *********************************************************************************/

/*
 * NAME:     process_audio.c (sample-based Talkthrough)
 * PURPOSE:  Function process_audio() for AD1939/ADSP-21364 Talkthrough framework.
 * USAGE:    This file contains the function to process AD1939 samples.
 */

/*
 *--------------------------------------------------------------------------------------
 *
 * DMA Buffer relationship to C Audio Variables
 *
 * Timeslot #  SPORT DMA Buffer Timeslot Data   <->   AD1939 ADC/DAC C Variables (floats)
 * ======================================================================================
 *
 * 0           DM(_rx1a_bufa + Internal_ADC_L1) -->  Left_Channel_In1
 * 1           DM(_rx1a_bufa + Internal_ADC_R1) -->  Right_Channel_In1
 * 2           DM(_rx1a_bufa + Internal_ADC_L2) -->  Left_Channel_In2
 * 3           DM(_rx1a_bufa + Internal_ADC_R2) -->  Right_Channel_In2
 * 4           DM(_rx1a_bufa + AUX_ADC_L1)      -->  X (not used in EZ-KIT Lite)
 * 5           DM(_rx1a_bufa + AUX_ADC_R1)      -->  X (not used in EZ-KIT Lite)
 * 6           DM(_rx1a_bufa + AUX_ADC_L2)      -->  X (not used in EZ-KIT Lite)
 * 7           DM(_rx1a_bufa + AUX_ADC_R2)      -->  X (not used in EZ-KIT Lite)
 *
 * 0           DM(_tx0a_buf + Internal_DAC_L1)  <--  Left_Channel_Out1
 * 1           DM(_tx0a_buf + Internal_DAC_R1)  <--  Right_Channel_Out1
 * 2           DM(_tx0a_buf + Internal_DAC_L2)  <--  Left_Channel_Out2
 * 3           DM(_tx0a_buf + Internal_DAC_R2)  <--  Right_Channel_Out2
 * 4           DM(_tx0a_buf + Internal_DAC_L3)  <--  Left_Channel_Out3
 * 5           DM(_tx0a_buf + Internal_DAC_R3)  <--  Right_Channel_Out3
 * 6           DM(_tx0a_buf + Internal_DAC_L4)  <--  Left_Channel_Out4
 * 7           DM(_tx0a_buf + Internal_DAC_R4)  <--  Right_Channel_Out4
 */



#include "ADDS_21479_EzKit.h"
#include "filter.h"


/* Inputs for left and right channel */
float dm x_l[FILTER_ORDER + 1] 	= {0};
float dm x_r[FILTER_ORDER + 1] 	= {0};

/* Outputs for left channel */
float dm y1_l[FILTER_ORDER + 1] 	= {0};
float dm y2_l[FILTER_ORDER + 1] 	= {0};
float dm y3_l[FILTER_ORDER + 1] 	= {0};
float dm y4_l[FILTER_ORDER + 1] 	= {0};
float dm y5_l[FILTER_ORDER + 1] 	= {0};

/* Outputs for right channel */
float dm y1_r[FILTER_ORDER + 1] 	= {0};
float dm y2_r[FILTER_ORDER + 1] 	= {0};
float dm y3_r[FILTER_ORDER + 1] 	= {0};
float dm y4_r[FILTER_ORDER + 1] 	= {0};
float dm y5_r[FILTER_ORDER + 1] 	= {0};

void  process_AD1939_samples(  uint32_t iid, void *handlerarg)
{
	/* Perform AD1939 Audio Processing Here Using 32-bit Floating-point Math
	 *  (Note: The ADC/DAC variables below are used to "double-buffer" the audio
	 *  samples with temporary variables, allowing active DMA completion of
	 *  rx1a_buf[] and tx0a_buf[])
	 */

	Receive_ADC_Samples();

	/* This is to ensure that there will be no divison by zero */
	if(ready == 1) {

		/* Move inputs by one place */
		for(int i = FILTER_ORDER; i > 0; i--) {

			x_l[i] = x_l[i-1];
			x_r[i] = x_r[i-1];
			y1_l[i] = y1_l[i-1];
			y1_r[i] = y1_r[i-1];
		}
		x_l[0] = Left_Channel_In1;
		x_r[0] = Right_Channel_In1;

		int i = 0; // current filter

		float sum_l = 0.0; // temporary variable for summation
		float sum_r = 0.0;
		float tmp = filters[i].a[0]; // to reduce execution time TODO: evaluate by how much

		for(int j = 1;j <= FILTER_ORDER; j++) {
			sum_l -= filters[i].a[j]*y1_l[j]/tmp;
			sum_r -= filters[i].a[j]*y1_r[j]/tmp;
		}

		for(int j = 0; j <= FILTER_ORDER; j++) {
			sum_l += filters[i].b[j]*x_l[j]/tmp;
			sum_r += filters[i].b[j]*x_l[j]/tmp;
		}

		y1_l[0] = sum_l;
		y1_r[0] = sum_r;

		sum_l = 0.0; // temporary variable for summation
		sum_r = 0.0;
		tmp = filters[++i].a[0]; // to reduce execution time TODO: evaluate by how much

		for(int j = FILTER_ORDER; j > 0; j--) {
			y2_l[j] = y2_l[j-1];
			y2_r[j] = y2_r[j-1];
		}

		for(int j = 1;j <= FILTER_ORDER; j++) {
			sum_l -= filters[i].a[j]*y2_l[j]/tmp;
			sum_r -= filters[i].a[j]*y2_r[j]/tmp;
		}

		for(int j = 0; j <= FILTER_ORDER; j++) {
			sum_l += filters[i].b[j]*y1_l[j]/tmp;
			sum_r += filters[i].b[j]*y1_r[j]/tmp;
		}

		y2_l[0] = sum_l;
		y2_r[0] = sum_r;

		sum_l = 0.0; // temporary variable for summation
		sum_r = 0.0;
		tmp = filters[++i].a[0]; // to reduce execution time TODO: evaluate by how much

		for(int j = FILTER_ORDER; j > 0; j--) {
			y3_l[j] = y3_l[j-1];
			y3_r[j] = y3_r[j-1];
		}

		for(int j = 1;j <= FILTER_ORDER; j++) {
			sum_l -= filters[i].a[j]*y3_l[j]/tmp;
			sum_r -= filters[i].a[j]*y3_r[j]/tmp;
		}

		for(int j = 0; j <= FILTER_ORDER; j++) {
			sum_l += filters[i].b[j]*y2_l[j]/tmp;
			sum_r += filters[i].b[j]*y2_r[j]/tmp;
		}

		y3_l[0] = sum_l;
		y3_r[0] = sum_r;

		sum_l = 0.0; // temporary variable for summation
		sum_r = 0.0;
		tmp = filters[++i].a[0]; // to reduce execution time TODO: evaluate by how much

		for(int j = FILTER_ORDER; j > 0; j--) {
			y4_l[j] = y4_l[j-1];
			y4_r[j] = y4_r[j-1];
		}

		for(int j = 1;j <= FILTER_ORDER; j++) {
			sum_l -= filters[i].a[j]*y4_l[j]/tmp;
			sum_r -= filters[i].a[j]*y4_r[j]/tmp;
		}

		for(int j = 0; j <= FILTER_ORDER; j++) {
			sum_l += filters[i].b[j]*y3_l[j]/tmp;
			sum_r += filters[i].b[j]*y3_r[j]/tmp;
		}

		y4_l[0] = sum_l;
		y4_r[0] = sum_r;

		sum_l = 0.0; // temporary variable for summation
		sum_r = 0.0;
		tmp = filters[++i].a[0]; // to reduce execution time TODO: evaluate by how much

		for(int j = FILTER_ORDER; j > 0; j--) {
			y5_l[j] = y5_l[j-1];
			y5_r[j] = y5_r[j-1];
		}

		for(int j = 1;j <= FILTER_ORDER; j++) {
			sum_l -= filters[i].a[j]*y5_l[j]/tmp;
			sum_r -= filters[i].a[j]*y5_r[j]/tmp;
		}

		for(int j = 0; j <= FILTER_ORDER; j++) {
			sum_l += filters[i].b[j]*y4_l[j]/tmp;
			sum_r += filters[i].b[j]*y4_r[j]/tmp;
		}

		y5_l[0] = sum_l;
		y5_r[0] = sum_r;

		Left_Channel_Out3 = sum_l;
		Right_Channel_Out3 = sum_r;
	}

	Transmit_DAC_Samples();
}

