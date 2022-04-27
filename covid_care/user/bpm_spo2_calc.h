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

#define INTERVAL 	4
#define BPM_MAX		230
#define BPM_MIN		30
#define SpO2_MAX	100
#define SpO2_MIN	95
#define FINGER_THRESH 20000

typedef struct __attribute__((packed))
{
	float AC;
	float DC;
	float BPM;
} PPG_t;

typedef struct BPM_SpO2_value_t
{
	uint8_t BPM;
	uint8_t SpO2;
} BPM_SpO2_value_t;

float max(float *array, int array_size);
float sum(float *array, int array_size);
void swap(float* x, float* y);
void sort(float* array, int array_size);
void trim(float* array, int* array_size, int offset);
void DC_removal(float* signal, int n_sample, float alpha);
void median_filter(float* signal, int n_sample, int filter_size);
void BPM_estimator(float* signal, PPG_t* ppg, int n_sample, float thresh, float sample_rate);
float SpO2_estimator(float R);
void assign_signal(float* ori, float* des, int n_sample);
uint8_t BPM_SpO2_Update(BPM_SpO2_value_t *result, uint8_t n);

#endif /* USER_BPM_SPO2_CALC_H_ */
