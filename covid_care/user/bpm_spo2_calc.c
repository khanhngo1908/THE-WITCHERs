/*
 * bpm_spo2_calc.c
 *
 *  Created on: 6 thg 3, 2022
 *      Author: Pham Minh Hanh
 */

#include "bpm_spo2_calc.h"
#include "sl_app_log.h"

fifo_t FIFO_data;
float t[STORAGE_SIZE]; // temp array to process signal

void BPM_SpO2_Update(BPM_SpO2_value_t *result, uint8_t n)
{

//	uint8_t i = 0;
	MAX30102_Continue();
	if (n > 1)
	{
		uint16_t j;
		for (j = 0; j < (STORAGE_SIZE - INTERVAL * THROUGHTPUT); j++)
		{
			FIFO_data.IR[j] = FIFO_data.IR[j + INTERVAL * THROUGHTPUT];
			FIFO_data.RED[j] = FIFO_data.RED[j + INTERVAL * THROUGHTPUT];
		}
		FIFO_data.cnt = STORAGE_SIZE - INTERVAL * THROUGHTPUT;
	}
	else
		FIFO_data.cnt = 0;

	MAX30102_ClearFIFO ();
	while (FIFO_data.cnt < STORAGE_SIZE)
	{
		MAX30102_ReadFIFO (&FIFO_data);
	}
	sl_app_log("---------------- %d ----------------- \n", n);

	/****************** Calc BPM, SpO2 (1)**********************/
	//		// DC removal
	//		float alpha = 0.95;
	//		DC_removal (FIFO_data.IR, STORAGE_SIZE, alpha);
	//		DC_removal(FIFO_data.RED, STORAGE_SIZE, alpha);
	//
	//		// Get DC components of ir signal and red signal
	//		float DC_ir = max (FIFO_data.IR, STORAGE_SIZE);
	//		float DC_red = max (FIFO_data.RED, STORAGE_SIZE);
	//
	//		PPG_t ppg_ir;
	//		PPG_t ppg_red;
	//
	//		ppg_ir.DC = DC_ir;
	//		ppg_red.DC = DC_red;
	//
	//		// applying median filter
	//		int filter_size = 5;
	//		median_filter (FIFO_data.IR, STORAGE_SIZE, filter_size);
	//		median_filter (FIFO_data.RED, STORAGE_SIZE, filter_size);
	//
	//		float thresh = 0.0;
	//		float sample_rate = THROUGHTPUT;
	//		BPM_estimator (FIFO_data.IR, &ppg_ir, STORAGE_SIZE, thresh, sample_rate);
	//		BPM_estimator(FIFO_data.RED, &ppg_red, STORAGE_SIZE, thresh, sample_rate);
	//
	//		// Calculate Spo2
	//		float R = (ppg_ir.AC / DC_ir) / (ppg_red.AC / DC_red);
	//		float spo2 = SpO2_estimator (R);
	//
	//		sl_app_log("ir BPM: %d AC: %d \n", (int) ppg_ir.BPM, (int) ppg_ir.AC);
	//		sl_app_log("red BPM: %d AC: %d\n", (int ) ppg_red.BPM, (int ) ppg_red.AC);
	//		sl_app_log("----------\n");
	/********************************************************/

	/****************** Calc BPM, SpO2 (2)**********************/
	PPG_t ppg_ir;
	PPG_t ppg_red;

	ppg_ir.BPM = 0.0;
	ppg_red.BPM = 0.0;

	float spo2 = 0.0;

	float alpha = 0.95;
	float thresh = 0.0;
	float sample_rate = THROUGHTPUT;

	float DC_ir = 0.0;
	float DC_red = 0.0;
	float R = 0.0;

	float offset = 100;
	int filter_size = 3;

	int idx;
	for (idx = 0; idx < STORAGE_SIZE; idx++)
	{
		if ((idx + 1) % (2 * THROUGHTPUT) == 0)
		{
			if ((idx + 1) != 2 * THROUGHTPUT)
			{
				/**** Calc for IR signal ****/
				assign_signal (FIFO_data.IR, t, idx);
				DC_removal (t, idx, alpha);
				DC_ir = max (t, idx);
				ppg_ir.DC = DC_ir;
				int idx_t = idx;
				//					trim (t, &idx_t, offset);
				median_filter (t, idx_t, filter_size);
				BPM_estimator (t, &ppg_ir, idx_t, thresh, sample_rate);
				/****************************/

				/**** Calc for RED signal ****/
				assign_signal (FIFO_data.RED, t, idx);
				DC_removal (t, idx, alpha);
				DC_red = max (t, idx);
				ppg_red.DC = DC_red;
				idx_t = idx;
				//					trim (t, &idx_t, offset);
				median_filter (t, idx_t, filter_size);
				BPM_estimator (t, &ppg_red, idx_t, thresh, sample_rate);
				/*****************************/

				// Calculate Spo2
				R = (ppg_ir.AC / ppg_ir.DC) / (ppg_red.AC / ppg_red.DC);
				//				R = (ppg_red.AC / ppg_red.DC) / (ppg_ir.AC / ppg_ir.DC);

				if (spo2 > 90.0)
				{
					spo2 = (spo2 + SpO2_estimator (R)) / 2;
				}
				else
				{
					spo2 = SpO2_estimator (R);
				}

//				sl_app_log("ir BPM: %d AC: %d \n", (int ) ppg_ir.BPM,
//						   (int ) ppg_ir.AC);
//				sl_app_log("red BPM: %d AC: %d\n", (int ) ppg_red.BPM,
//						   (int ) ppg_red.AC);
//				sl_app_log("spo2: %d\n", (int) spo2);
//				sl_app_log("----------\n");
			}
		}
	}
	/**********************************************************/
	result->BPM = (int) ((ppg_ir.BPM + ppg_red.BPM) / 2);
	result->SpO2 = (int) spo2;
//	sl_app_log("BPM: %d \n", result->BPM);
//	sl_app_log("Spo2: %d \n", result->SpO2);
	MAX30102_Shutdown ();
	if(ppg_ir.DC < FINGER_THRESH && ppg_red.DC < FINGER_THRESH)
	{
		result->BPM = 0;
		result->SpO2 = 0;
//		return 2;
	}
	else
	{
		if(result->BPM < BPM_MIN)
			result->BPM = BPM_MIN;
		if(result->BPM > BPM_MAX)
			result->BPM = BPM_MAX;
		if(result->SpO2 < SpO2_MIN)
			result->SpO2 = SpO2_MIN;
		if(result->SpO2 > SpO2_MAX)
			result->SpO2 = SpO2_MAX;
	}
//	return 0;
}

float max(float *array, int array_size)
{
	int i;
	float m = array[0];
	for(i = 1; i < array_size; ++ i){
		if(array[i] > m)
			m = array[i];
	}
	return m;
}

float sum(float *array, int array_size)
{
    float s = 0;
    int i ;
    for( i = 0; i < array_size; ++ i){
        s = s + array[i];
    }
    return s;
}

void swap(float* x, float* y)
{
    float temp = *x;
    *x = *y;
    *y = temp;
}

void sort(float* array, int array_size)
{
    int i, j;
    int min_idx;
    for (i = 0; i < (array_size) - 1; ++ i) {
        min_idx = i;
        for (j = i + 1; j < (array_size); j++)
            if (array[j] < array[min_idx])
                min_idx = j;
        swap(&array[min_idx], &array[i]);
    }
}

void trim(float* array, int* array_size, int offset)
{
    int i ;
    for( i = 0; i < (*array_size - offset) ; ++ i){
        array[i] = array[i + offset];
    }
    *array_size = *array_size - offset;
}

void DC_removal(float* signal, int n_sample, float alpha)
{
    float pre_w = 0.0;

    int now_w = 0;
    int i ;
    for(i = 0; i < n_sample; ++ i){
        float now_w = signal[i] + (alpha * pre_w);
        signal[i] = now_w - pre_w;
        pre_w = now_w;
    }
}

void median_filter(float* signal, int n_sample, int filter_size)
{
    int indexer = filter_size / 2;
    int window[filter_size];
    float temp[filter_size];
    int i;
    for(i = 0; i < filter_size; i ++){
        window[i] = i - indexer;
    }

    for(i =0; i < n_sample; ++ i){
        int j;
        for(j = 0; j < filter_size; ++ j){
            if(i + window[j] < 0 || n_sample <= i + window[j]){
                temp[j] = 0;
            } else{
                temp[j] = signal[i + window[j]];
            }
        }
        sort(temp, filter_size);
        signal[i] = temp[indexer];
    }
}

void BPM_estimator(float* signal, PPG_t* ppg, int n_sample, float thresh, float sample_rate)
{
    int count_p = 0;
    int cloc = 0;
    int ploc = 0;
    int peaks = 0;

    float interval = 0.0;
    int interval_count = 0 ;

    float lmax = 0;
    float nmax = 0;


    bool trace_up = false;
    bool trace_down = false;
    int i ;

    for(i = 0; i < n_sample; ++i){
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
                int c_interval = cloc - ploc;
                interval = interval + c_interval;
                interval_count ++;

                if (c_interval > (interval / interval_count / 2)){
                    peaks++;
                }

                if (ploc != 0){
                    float max_lreg = max(&signal[ploc], cloc - ploc);
                    lmax = lmax + max_lreg;
                    nmax ++;
                }
                ploc = cloc;
            }
        }

    }

//    sl_app_log("peaks: %d   n_sample: %d\n", peaks, n_sample);
//    printf("peaks: %d   n_sample: %d\n", peaks, n_sample);

    float bpm = (peaks + 1) / (n_sample / sample_rate) * 60.0;
    float AC = lmax / nmax;

    if((ppg->BPM) > 60.0){
		ppg->BPM = (ppg->BPM + bpm) / 2;
	}
	else
	{
		ppg->BPM = bpm;
	}
//    ppg->BPM = bpm;
    ppg->AC = AC;
}

float SpO2_estimator(float R)
{
    float spo2 = 0.0;
    if(0.4 <= R && R <= 1){
        spo2 = 110.0 - 25.0 * R;
    } else if(R <= 2.0){
        spo2 = 120.0 - 35.0 * R;
    } else if(R <= 3.5){
        spo2 = 350.0 / 3.0 - 100.0 / 3.0 * R;
    }
    if(spo2 > 100.0){
        spo2 = 100.0;
    } else if (spo2 < 80.0){
        spo2 = 80.0;
    }
    return spo2;
}

void assign_signal(float* ori, float* des, int n_sample)
{
    int i;
    for(i = 0; i < n_sample; ++i){
        des[i] = ori[i];
    }
}
