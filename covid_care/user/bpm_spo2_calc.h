/*
 * BPM_spo2_calc.h
 *
 *  Created on: 6 thg 3, 2022
 *      Author: Pham Minh Hanh
 */
#include "stdint.h"
#include "max30102.h"

#ifndef USER_BPM_SPO2_CALC_H_
#define USER_BPM_SPO2_CALC_H_

typedef struct PPG_properties_t
{
	uint32_t BPM;
	uint32_t AC;
	uint32_t DC;
} PPG_properties_t;

typedef struct PPG_signal_t
{
	int32_t IR[STORAGE_SIZE];
	int32_t RED[STORAGE_SIZE];
} PPG_signal_t;

void DC_removal (uint32_t *raw_data, int32_t *data, uint16_t n_sample, float alpha);
void median_filter(int32_t *signal, uint16_t n_sample, uint8_t filter_size);
void sort (int32_t *array, uint8_t array_size);
void swap (int32_t *x, int32_t *y);
void BPM_estimator(int32_t* signal, PPG_properties_t* PPG_properties, uint16_t n_sample, int32_t thresh, float sample_rate);
int32_t max(int32_t *array, int32_t array_size);

#endif /* USER_BPM_SPO2_CALC_H_ */
