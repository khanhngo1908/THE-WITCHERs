/*
 * rtc.h
 *
 *  Created on: 22 thg 3, 2022
 *      Author: Ngo Minh Khanh
 */

#ifndef USER_RTC_H_
#define USER_RTC_H_

#include "rtcdriver.h"
#include <stddef.h>

typedef struct rtc_value_t
{
    uint8_t hour;
    uint8_t minute;
} rtc_value_t;

void rtc_init(void);
void myCallback( RTCDRV_TimerID_t id, void * user );
void get_rtc_value(void);

#endif /* USER_RTC_H_ */
