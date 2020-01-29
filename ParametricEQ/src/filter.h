#include "ADDS_21479_EzKit.h"

#define FILTER_ORDER	2
#define NUM_FILTERS		5
#define PI 				3.14159265


/* @brief
 * This Enum type tells us what filter is in use, we can have Low-pass filter (0),
 * High-pass filter(1), Low-shelf filter (2), High-shelf filter (3) and Peaking filter (4)
 */
typedef enum {

	LPF,
	HPF,
	LSF,
	HSF,
	PF,
} filter_type;

/* @brief
 * Struct for filters, that holds every information for filters. This DSP does not support uint8_t it only supports
 * uint32_t and higher
 */
typedef struct filter {

	float 		frequency; // Desired frequency for filter, cut-off frequency for shelf and pass filters, center frequency for peaking
	float		Q; 		   // Quality factor, determines how narrow is peaking filter
	float		a[FILTER_ORDER + 1]; // Coefficients for transfer function denominator
	float		b[FILTER_ORDER + 1]; // Coefficients for transfer function numerator
	filter_type f_type; // This defines filter type
	float		gain; // Gain for filter in dB

} filter_t;

extern filter_t filters[5];

filter_t GenerateFilter(filter_type filt_type,double frequency,double Q,char gain);

filter_t GenerateFilter1(filter_type filt_type,double frequency,double Q,float gain);

int32_t CalculateCoefficients(filter_t* wanted_filter);

