/*
 * bpm_spo2_calc.c
 *
 *  Created on: 6 thg 3, 2022
 *      Author: Pham Minh Hanh
 */

#include "bpm_spo2_calc.h"
#include "sl_app_log.h"


void DC_removal (uint32_t *raw_data, int32_t *data, uint16_t n_sample, float alpha)
{
	uint32_t pre_w = 0;
	uint16_t i;
	for (i = 0; i < n_sample; i++)
	{
		uint32_t now_w = raw_data[i] + (alpha * pre_w);
//		sl_app_log("%d\n", now_w);
		data[i] =  (now_w - pre_w);
		pre_w = now_w;
	}
}

void median_filter(int32_t *signal, uint16_t n_sample, uint8_t filter_size)
{
	uint8_t indexer = filter_size / 2;
	int16_t window[filter_size];
	int32_t temp[filter_size];
	uint16_t i;
	for (i = 0; i < filter_size; i++)
	{
		window[i] = i - indexer;
	}

	for (i = 0; i < n_sample; i++)
	{
		uint8_t j;
		for (j = 0; j < filter_size; j++)
		{
			if (i + window[j] < 0 || n_sample <= i + window[j])
			{
				temp[j] = 0;
			}
			else
			{
				temp[j] = signal[i + window[j]];
			}
		}
		sort (temp, filter_size);
		signal[i] = temp[indexer];
	}
}

void BPM_estimator(int32_t* signal, PPG_properties_t* PPG_properties, uint16_t n_sample, int32_t thresh, float sample_rate)
{
	int32_t count_p = 0;
	int32_t cloc = 0;
	int32_t ploc = 0;
	int32_t peaks = 0;

	int32_t interval = 0.0;
	int32_t interval_count = 0 ;

	int32_t lmax = 0;
    float nmax = 0;


    bool trace_up = false;
    bool trace_down = false;
    uint16_t i ;

    for(i = 1; i < n_sample; i++){
        if(signal[i] >= thresh && signal[i-1] < thresh){
            trace_up = true;
        }else{
            trace_up = false;
        }

        if(signal[i] <= thresh && signal[i-1] > thresh){
            trace_down = true;
        }else{
            trace_down = false;
        }

        if(trace_up == true || trace_down == true){
            count_p ++;
            if (peaks == 0) {
                peaks++;
            }
            if (count_p % 2 == 0) {
                cloc = i;
                int32_t c_interval = cloc - ploc;
                interval = interval + c_interval;
                interval_count ++;

                if (c_interval > (interval / interval_count / 2)){
                    peaks++;
                }

                if (ploc != 0){
                	int32_t max_lreg = max(&signal[ploc], cloc - ploc);
                    lmax = lmax + max_lreg;
                    nmax ++;
                }
                ploc = cloc;
            }
        }

    }

    int32_t bpm = peaks / (n_sample / sample_rate) * 60.0;
    int32_t AC = lmax / nmax;

    PPG_properties->BPM = bpm;
    PPG_properties->AC = AC;
    sl_app_log(" BPM: %d\n", bpm);
}

void sort (int32_t *array, uint8_t array_size)
{
	uint8_t i, j;
	uint8_t min_idx;
	for (i = 0; i < (array_size) - 1; i++)
	{
		min_idx = i;
		for (j = i + 1; j < (array_size); j++)
			if (array[j] < array[min_idx])
				min_idx = j;
		swap (&array[min_idx], &array[i]);
	}
}

void swap (int32_t *x, int32_t *y)
{
	int32_t temp = *x;
	*x = *y;
	*y = temp;
}

int32_t max(int32_t *array, int32_t array_size)
{
	int32_t i;
	int32_t m = array[0];
	for(i = 1; i < array_size; i++){
		if(array[i] > m)
			m = array[i];
	}
	return m;
}

