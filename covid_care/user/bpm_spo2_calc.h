/*
 * hr_spo2_calc.h
 *
 *  Created on: 6 thg 3, 2022
 *      Author: Pham Minh Hanh
 */
#include "stdint.h"

#ifndef USER_HR_SPO2_CALC_H_
#define USER_HR_SPO2_CALC_H_

typedef struct PPG{
  uint32_t BPM;
  uint32_t AC;
  uint32_t DC;
} PPG;

extern PPG ppg_ir;
extern PPG ppg_red;

uint32_t max(uint32_t *array, uint32_t array_size);
uint32_t sum(uint32_t *array, uint32_t array_size);
void swap(uint32_t* x, uint32_t* y);
void sort(uint32_t* array, uint32_t array_size);
void trim(uint32_t* array, uint32_t* array_size, uint32_t offset);
void DC_removal(uint32_t* signal, uint32_t n_sample, float alpha);
void median_filter(uint32_t* signal, uint32_t n_sample, uint32_t filter_size);
void BPM_estimator(uint32_t* signal, PPG* ppg, uint32_t n_sample, uint32_t thresh, float sample_rate);
uint32_t SpO2_estimator(uint32_t R);
void BPM_SpO2_estimator();
void PPG_update();

#endif /* USER_HR_SPO2_CALC_H_ */
