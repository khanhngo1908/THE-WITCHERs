/*
 * hr_spo2_calc.c
 *
 *  Created on: 6 thg 3, 2022
 *      Author: Pham Minh Hanh
 */

#include <bpm_spo2_calc.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include "max30102.h"

void DC_removal(uint32_t* signal, uint16_t n_sample, float alpha){
    double pre_w = 0.0;

    int now_w = 0;
    int i;
    for(i = 0; i < n_sample; i++){
        double now_w = signal[i] + (alpha * pre_w);
        signal[i] = now_w - pre_w;
        pre_w = now_w;
    }
}
