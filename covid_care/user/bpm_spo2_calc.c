/*
 * bpm_spo2_calc.c
 *
 *  Created on: 6 thg 3, 2022
 *      Author: Pham Minh Hanh
 */

#include "bpm_spo2_calc.h"
#include "sl_app_log.h"

fifo_t FIFO_data;
//PPG_t ppg_ir;
//PPG_t ppg_red;
//int16_t IR[STORAGE_SIZE];
//int16_t RED[STORAGE_SIZE];
float t_ir[STORAGE_SIZE];
float t_red[STORAGE_SIZE];
float o_ir[STORAGE_SIZE];
float o_red[STORAGE_SIZE];

void check(int16_t *arr, uint16_t n)
{
	uint16_t i;
	for(i = 0; i < n; i++)
	{
		sl_app_log("%d \n",arr[i]);
	}
}

void check1(fifo_t *fifo, uint16_t n)
{
	uint16_t i;
	for(i = 0; i < n; i++)
	{
		sl_app_log(" %d %d \n", fifo->raw_IR[i], fifo->raw_RED[i]);
	}
}

//void check2(fifo_t *fifo, )

void BPM_SpO2_Update(BPM_SpO2_value_t *result, uint8_t n)
{

	uint8_t i = 0;
	MAX30102_Continue();
	while(i < n)
	{
		if (i > 0)
		{
			uint16_t j;
			for (j = 0; j < (STORAGE_SIZE - INTERVAL*THROUGHTPUT); j++)
			{
				FIFO_data.raw_IR[j] = FIFO_data.raw_IR[j + INTERVAL*THROUGHTPUT];
				FIFO_data.raw_RED[j] = FIFO_data.raw_RED[j + INTERVAL*THROUGHTPUT];
			}
			FIFO_data.cnt = STORAGE_SIZE - INTERVAL*THROUGHTPUT;
		}
		else
			FIFO_data.cnt = 0;

		MAX30102_ClearFIFO ();
		while (FIFO_data.cnt < STORAGE_SIZE)
		{
			MAX30102_ReadFIFO (&FIFO_data);
		}
//		check1(&FIFO_data, STORAGE_SIZE);
		sl_app_log(" ----------- End read ------------- \n");

		/****************** Calc BPM, SpO2 **********************/
		int max_sample = 450;
		sl_app_log(" 0.1 \n");
	 // temp array to process signal
	// original array to save signal
		sl_app_log(" 0.2 \n");
		int idx_ir = 0, idx_red = 0;
		sl_app_log(" 1 \n");

		PPG_t ppg_ir;
		PPG_t ppg_red;

		ppg_ir.BPM = 0.0;
		ppg_red.BPM = 0.0;
		sl_app_log(" 2 \n");

		float spo2 = 0.0;

		float alpha = 0.95;
		float thresh = 0.0;
		float sample_rate = 50.0;

		float DC_ir = 0.0;
		float DC_red = 0.0;
		float R = -1.0;
		sl_app_log(" 3 \n");

		int offset = 100;

		int i;
		for (i = 0; i < max_sample; ++i)
		{
			o_ir[i] = FIFO_data.raw_IR[i];
			o_red[i] = FIFO_data.raw_RED[i];

			if ((i + 1) % (3*THROUGHTPUT) == 0)
			{
				idx_ir = (i + 1) > max_sample ? (i + 1) - max_sample : (i + 1);
				idx_red = idx_ir;

				sl_app_log("idx %d\n", idx_ir);
//				printf ("idx %d\n", idx_ir);

				// asignal original array to temp array
				assign_signal (o_ir, t_ir, idx_ir);
				assign_signal (o_red, t_red, idx_red);

				DC_removal (t_ir, idx_ir, alpha);
				DC_removal (t_red, idx_red, alpha);

				// Get DC components of ir signal and red signal
				DC_ir = max (t_ir, idx_ir);
				DC_red = max (t_red, idx_red);

				if (DC_ir < 10000.0 && DC_red < 10000.0)
				{
					sl_app_log("Please put your finger in to measure Spo2 & BPM!\n");
//					printf ("Please put your finger in to measure Spo2 & BPM!\n");
					continue;
				}

				ppg_ir.DC = DC_ir;
				ppg_red.DC = DC_red;

				sl_app_log("ir DC component: %d\n", (int) DC_ir);
				sl_app_log("red DC component: %d \n", (int) DC_red);
//				printf ("ir DC component: %lf\n", DC_ir);
//				printf ("red DC component: %lf\n", DC_red);

				// Cut off 100 first samples to remove redundant DC components in signal
				trim (t_ir, &idx_ir, offset);
				trim (t_red, &idx_red, offset);

				// Estimate BPM
				BPM_estimator (t_ir, &ppg_ir, idx_ir, thresh, sample_rate);
				BPM_estimator (t_red, &ppg_red, idx_red, thresh, sample_rate);

				sl_app_log("ir BPM: %d AC: %d\n", (int) ppg_ir.BPM, (int) ppg_ir.AC);
				sl_app_log("red BPM: %d AC: %d\n", (int) ppg_red.BPM, (int) ppg_red.AC);
//				printf ("ir BPM: %lf AC: %lf\n", ppg_ir.BPM, ppg_ir.AC);
//				printf ("red BPM: %lf AC: %lf\n", ppg_red.BPM, ppg_red.AC);

				// Calculate Spo2
				R = (ppg_ir.AC / DC_ir) / (ppg_red.AC / DC_red);

				if (spo2 > 79.0)
				{
					spo2 = (spo2 + SpO2_estimator (R)) / 2;
				}
				else
				{
					spo2 = SpO2_estimator (R);
				}
				sl_app_log("Spo2: %d \n", (int) spo2);
				sl_app_log("-----------{%d}-----------\n----------------------------\n", i);
//				printf ("Spo2: %lf\n", spo2);
//				printf ("-----------{%d}-----------\n----------------------------\n", i);
			}
		}
		/********************************************************/
		i += 1;
	}
	MAX30102_Shutdown();
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

    for(i = 1; i < n_sample; ++i){
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

    sl_app_log("peaks: %d   n_sample: %d\n", peaks, n_sample);
//    printf("peaks: %d   n_sample: %d\n", peaks, n_sample);

    float bpm = peaks / (n_sample / sample_rate) * 60.0;
    float AC = lmax / nmax;

    if((ppg->BPM) > 60.0){
        ppg->BPM = (ppg->BPM + bpm) / 2;
    } else {
        ppg->BPM = bpm;
    }
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
