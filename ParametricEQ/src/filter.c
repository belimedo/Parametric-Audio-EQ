#include "filter.h"
#include <stdint.h>
#include <math.h>


/*
 * @brief This method generates desired filter depending on forwarded inputs.
 *
 * @param[in] 	filt_type 	- One of five different filter types (LPF,HPF,LSF,HSF,PF)
 * @param[in]	frequency 	- Centre frequency for filter
 * @param[in]	Q 		  	- Quality factor, used for bandwidth in LPF,HPF and PF in coefficient calculation
 * @param[in]	gain		- Gain given in dB.
 *
 * @output
 * @returnval 	1 			- Filter is generated correctly
 * @returnval 	0 			- Filter isn't generated correctly
 */
filter_t GenerateFilter(filter_type filt_type,double frequency,double Q,char gain) {

	filter_t tmp;
	tmp.frequency=frequency;
	tmp.Q=Q;
	tmp.gain=gain;
	tmp.f_type=filt_type;
	if(CalculateCoefficients(&tmp))
		return tmp;
	else
		printf("GRESKA!!!");
		return tmp;
}

/*
 * @brief This method calculates coefficients depending on filter parameters forwarded by pointer on struct filter_t
 *
 * @param[in]	wanted_filter 	- Instance of struct filter with filled frequency, gain, type and Q fields.
 *
 * @output
 * @returnval 	1 				- Coefficients calculated correctly
 * @returnval 	0 				- Coefficients calculated incorrectly
 */
int32_t CalculateCoefficients(filter_t* wanted_filter) {

	// If frequency is negative or greater than our sample frequency exit calculation
	if((wanted_filter->frequency < 0) || (wanted_filter->frequency > fs)) {

		return 0;
	}


	float V0 = pow(10, (wanted_filter->gain) / 20);
	float A  = pow(10, wanted_filter->gain / 40);
	float w0 = 2* PI * (wanted_filter->frequency / fs);
	float K = tan(w0/ 2);
	float alfa = sin(w0)/(2*wanted_filter->Q);

	float tempa1 = 1 + K*K*V0;
	float tempa2 = sqrt(2*V0)*K;

	float tempb1 = 1 + K*K;
	float tempb2 = sqrt(2)*K;

	float tempc1 = V0 + K*K;
	float tempc2 = tempb1 - (V0 + 1);

	float tempd1 = 1 + (K*K)/V0;
	float tempd2 = sqrt(2/V0)*K;

	if(wanted_filter->f_type == LPF) {

		wanted_filter->a[0] = 1 + alfa;
		wanted_filter->a[1] = -2*cos(w0);
		wanted_filter->a[2] = 1 - alfa;

		wanted_filter->b[1] = 1 - cos(w0);
		wanted_filter->b[0] = wanted_filter->b[1]/2;
		wanted_filter->b[2] = wanted_filter->b[0];
		return 1;
	}

	if(wanted_filter->f_type == HPF) {

		wanted_filter->a[0] = 1 + alfa;
		wanted_filter->a[1] = -2*cos(w0);
		wanted_filter->a[2] = 1 - alfa;

		wanted_filter->b[0] = 1 + cos(w0)/2;
		wanted_filter->b[1] = -1 - cos(w0);
		wanted_filter->b[2] = wanted_filter->b[0];
		return 1;
	}

	if(wanted_filter->f_type == LSF) {

		if(wanted_filter->gain == 0) {

			wanted_filter->a[0] = 1;
			wanted_filter->a[1] = 0;
			wanted_filter->a[2] = 0;

			wanted_filter->b[0] = 1;
			wanted_filter->b[1] = 0;
			wanted_filter->b[2] = 0;
			return 1;
		}

		if(wanted_filter->gain > 0) {

			wanted_filter->a[0] = 1;
			wanted_filter->a[1] = 2*(K*K-1)/(1 + sqrt(2)*K + K*K);
			wanted_filter->a[2] = (1-sqrt(2)*K+K*K)/(1 + sqrt(2)*K + K*K);

			wanted_filter->b[0] = (1 + sqrt(2*V0)*K + V0*K*K)/(1 + sqrt(2)*K + K*K);
			wanted_filter->b[1] = 2*(V0*K*K - 1)/(1 + sqrt(2)*K + K*K);
			wanted_filter->b[2] = (1 - sqrt(2*V0)*K + V0*K*K)/(1 + sqrt(2)*K + K*K);
			return 1;
		}

		else {

			V0 = pow(10, (-wanted_filter->gain) / 20);

			wanted_filter->a[0] = 1;
			wanted_filter->a[1] = 2*(V0*K*K - 1)/(1 + sqrt(2*V0)*K + V0*K*K);
			wanted_filter->a[2] = (1 - sqrt(2*V0)*K + V0*K*K)/(1 + sqrt(2*V0)*K + V0*K*K);

			wanted_filter->b[0] = (1 + sqrt(2)*K + K*K)/(1 + sqrt(2*V0)*K + V0*K*K);
			wanted_filter->b[1] =  2*(K*K-1)/(1 + sqrt(2*V0)*K + V0*K*K);
			wanted_filter->b[2] = (1-sqrt(2)*K+K*K)/(1 + sqrt(2*V0)*K + V0*K*K);
			return 1;
		}
		return 1;
	}

	if(wanted_filter->f_type == HSF) {

		if(wanted_filter->gain == 0) {

			wanted_filter->a[0] = 1;
			wanted_filter->a[1] = 0;
			wanted_filter->a[2] = 0;

			wanted_filter->b[0] = 1;
			wanted_filter->b[1] = 0;
			wanted_filter->b[2] = 0;
			return 1;
		}

		if (wanted_filter->gain > 0) {

			wanted_filter->a[0] = 1;
			wanted_filter->a[1] = 2*(K*K-1)/(1 + sqrt(2)*K + K*K);
			wanted_filter->a[2] = (1-sqrt(2)*K+K*K)/(1 + sqrt(2)*K + K*K);

			wanted_filter->b[0] = (V0 + sqrt(2*V0)*K + K*K)/(1 + sqrt(2)*K + K*K);
			wanted_filter->b[1] = 2*(K*K - V0)/(1 + sqrt(2)*K + K*K);
			wanted_filter->b[2] = (V0 - sqrt(2*V0)*K + K*K)/(1 + sqrt(2)*K + K*K);
			return 1;
		}

		else {

			V0 = pow(10, (-wanted_filter->gain) / 20);

			wanted_filter->a[0] = 1;
			wanted_filter->a[1] = 2*(K*K/V0 - 1)/(1 + sqrt(2/V0)*K + K*K/V0);
			wanted_filter->a[2] = (1 - sqrt(2/V0)*K + K*K/V0)/(1 + sqrt(2/V0)*K + K*K/V0);

			wanted_filter->b[0] = (1 + sqrt(2)*K + K*K)/(V0 + sqrt(2*V0)*K + K*K);
			wanted_filter->b[1] =  2*(K*K-1)/(V0 + sqrt(2*V0)*K + K*K);
			wanted_filter->b[2] = (1-sqrt(2)*K+K*K)/(V0 + sqrt(2*V0)*K + K*K);
			return 1;
		}
	}

	if(wanted_filter->f_type == PF) {

		if(wanted_filter->gain == 0) {

			wanted_filter->a[0] = 1;
			wanted_filter->a[1] = 0;
			wanted_filter->a[2] = 0;

			wanted_filter->b[0] = 1;
			wanted_filter->b[1] = 0;
			wanted_filter->b[2] = 0;
			return 1;
		}
		else {
			wanted_filter->a[0] = 1 + alfa/A;
			wanted_filter->a[1] = -2 * cos(w0);
			wanted_filter->a[2] = 1 - alfa/A;

			wanted_filter->b[0] = 1 + alfa * A;
			wanted_filter->b[1] = -2 * cos(w0);
			wanted_filter->b[2] = 1 - alfa * A;
			return 1;
		}
	}
	return 0;
}

/*
 * @brief Optimised method for generating filters depending on filter type, frequency, quality factor and gain.
 *
 * @param[in] 	filt_type 	- One of five different filter types (LPF,HPF,LSF,HSF,PF)
 * @param[in]	frequency 	- Centre frequency for filter
 * @param[in]	Q 		  	- Quality factor, used for bandwidth in LPF,HPF and PF in coefficient calculation
 * @param[in]	gain		- Gain given in dB.
 *
 * @output	-	Generated filter struct with calculated parameters
 */
filter_t GenerateFilter1(filter_type filt_type,double frequency,double Q,float gain) {

	filter_t tmp;
	filter_t c;
	tmp.frequency=frequency;
	tmp.Q=Q;
	tmp.gain=gain;
	tmp.f_type=filt_type;

	if((frequency < 0) || (frequency > fs)) {
		tmp.a[0] = 0;
		tmp.a[1] = 0;
		tmp.a[2] = 0;

		tmp.b[0] = 0;
		tmp.b[1] = 0;
		tmp.b[2] = 0;
		return tmp;
	}


	float V0 = pow(10, (gain) / 20.0);
	float A  = pow(10, gain / 40.0);
	float w0 = 2* PI * (frequency / fs);
	float K = tan(w0/ 2);
	float alfa = sin(w0)/(2*Q);

	float tempa1 = 1 + K*K*V0;
	float tempa2 = sqrt(2*V0)*K;

	float tempb1 = 1 + K*K;
	float tempb2 = sqrt(2)*K;

	float tempc1 = V0 + K*K;
	float tempc2 = K*K - V0;

	float tempd1 = (K*K)/V0 -1;
	float tempd2 = sqrt(2/V0)*K;

	if(filt_type == LPF) {

		tmp.a[0] = 1 + alfa;
		tmp.a[1] = -2*cos(w0);
		tmp.a[2] = 1 - alfa;

		tmp.b[1] = 1 - cos(w0);
		tmp.b[0] = tmp.b[1]/2;
		tmp.b[2] = tmp.b[0];
		return tmp;
	}

	if(filt_type == HPF) {

		tmp.a[0] = 1 + alfa;
		tmp.a[1] = -2*cos(w0);
		tmp.a[2] = 1 - alfa;

		tmp.b[0] = 1 + cos(w0)/2;
		tmp.b[1] = -1 - cos(w0);
		tmp.b[2] = tmp.b[0];
		return tmp;
	}

	if(filt_type == LSF) {

		if(gain == 0) {

			tmp.a[0] = 1;
			tmp.a[1] = 0;
			tmp.a[2] = 0;

			tmp.b[0] = 1;
			tmp.b[1] = 0;
			tmp.b[2] = 0;
			return tmp;
		}

		if(gain > 0) {

			tmp.a[0] = 1;
			tmp.a[1] = 2*(K*K-1)/(1 + sqrt(2)*K + K*K);
			tmp.a[2] = (1-sqrt(2)*K+K*K)/(1 + sqrt(2)*K + K*K);

			tmp.b[0] = (1 + sqrt(2*V0)*K + V0*K*K)/(1 + sqrt(2)*K + K*K);
			tmp.b[1] = 2*(V0*K*K - 1)/(1 + sqrt(2)*K + K*K);
			tmp.b[2] = (1 - sqrt(2*V0)*K + V0*K*K)/(1 + sqrt(2)*K + K*K);
			return tmp;
		}

		else {

			V0 = pow(10, (-gain) / 20);

			tmp.a[0] = 1;
			tmp.a[1] = 2*(V0*K*K - 1)/(1 + sqrt(2*V0)*K + V0*K*K);
			tmp.a[2] = (1 - sqrt(2*V0)*K + V0*K*K)/(1 + sqrt(2*V0)*K + V0*K*K);

			tmp.b[0] = (1 + sqrt(2)*K + K*K)/(1 + sqrt(2*V0)*K + V0*K*K);
			tmp.b[1] =  2*(K*K-1)/(1 + sqrt(2*V0)*K + V0*K*K);
			tmp.b[2] = (1-sqrt(2)*K+K*K)/(1 + sqrt(2*V0)*K + V0*K*K);
			return tmp;

		}
	}

	if(filt_type == HSF) {

		if(gain == 0) {

			tmp.a[0] = 1;
			tmp.a[1] = 0;
			tmp.a[2] = 0;

			tmp.b[0] = 1;
			tmp.b[1] = 0;
			tmp.b[2] = 0;
			return tmp;
		}

		if (gain > 0) {

			tmp.a[0] = 1;
			tmp.a[1] = 2*(K*K-1)/(1 + sqrt(2)*K + K*K);
			tmp.a[2] = (1-sqrt(2)*K+K*K)/(1 + sqrt(2)*K + K*K);

			tmp.b[0] = (V0 + sqrt(2*V0)*K + K*K)/(1 + sqrt(2)*K + K*K);
			tmp.b[1] = 2*(K*K - V0)/(1 + sqrt(2)*K + K*K);
			tmp.b[2] = (V0 - sqrt(2*V0)*K + K*K)/(1 + sqrt(2)*K + K*K);
			return tmp;
		}

		else {

			V0 = pow(10, (-gain) / 20);

			tmp.a[0] = 1;
			tmp.a[1] = 2*(K*K/V0 - 1)/(1 + sqrt(2/V0)*K + K*K/V0);
			tmp.a[2] = (1 - sqrt(2/V0)*K + K*K/V0)/(1 + sqrt(2/V0)*K + K*K/V0);

			tmp.b[0] = (1 + sqrt(2)*K + K*K)/(V0 + sqrt(2*V0)*K + K*K);
			tmp.b[1] =  2*(K*K-1)/(V0 + sqrt(2*V0)*K + K*K);
			tmp.b[2] = (1-sqrt(2)*K+K*K)/(V0 + sqrt(2*V0)*K + K*K);
			return tmp;
		}
	}

	if(filt_type == PF) {

		if(gain == 0) {

			tmp.a[0] = 1;
			tmp.a[1] = 0;
			tmp.a[2] = 0;

			tmp.b[0] = 1;
			tmp.b[1] = 0;
			tmp.b[2] = 0;
			return tmp;
		}
		else {

			tmp.a[0] = 1 + alfa/A;
			tmp.a[1] = -2 * cos(w0);
			tmp.a[2] = 1 - alfa/A;

			tmp.b[0] = 1 + alfa * A;
			tmp.b[1] = -2 * cos(w0);
			tmp.b[2] = 1 - alfa * A;
			return tmp;
		}
	}
	return tmp;
}
