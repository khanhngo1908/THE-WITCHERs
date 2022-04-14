/*
 * msc.h
 *
 *  Created on: 6 thg 4, 2022
 *      Author: Ngo Minh Khanh
 */

#ifndef USER_MSC_H_
#define USER_MSC_H_

#include "em_msc.h"
#include <inttypes.h>

#define USERDATA ((uint32_t*)USERDATA_BASE)
#define MSC_MAX_COUNTER 128

void MSC_init(void);
uint8_t MSC_write (uint8_t *data, uint8_t *msc_DataCount);
void MSC_read (uint8_t *data, uint8_t msc_DataCount);
void MSC_CheckUnRead(uint8_t *unReadCounter, uint8_t *dataCounter);
void MSC_CheckPage();

#endif /* USER_MSC_H_ */
