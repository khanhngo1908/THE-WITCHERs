/*
 * rtc.c
 *
 *  Created on: 22 thg 3, 2022
 *      Author: Ngo Minh Khanh
 */

#include "rtc.h"
#include <stdio.h>
#include <inttypes.h>
#include "sl_app_log.h"

uint8_t i = 0;
RTCDRV_TimerID_t idTimer;
uint32_t wallClock;
uint8_t hh, mm, ss;

void myCallback( RTCDRV_TimerID_t id, void * user )
{
  (void) user; // unused argument in this example

  i++;

  if ( i < 10 ) {
    // Restart timer
    RTCDRV_StartTimer( id, rtcdrvTimerTypeOneshot, 100, myCallback, NULL );
  }
}

void rtc_init(void)
{
    RTCDRV_Init();

    // Reserve a timer.
    RTCDRV_AllocateTimer( &idTimer );

    // Start a oneshot timer with 100 millisecond timeout.
    RTCDRV_StartTimer( idTimer, rtcdrvTimerTypeOneshot, 100, myCallback, NULL );
}

void get_rtc_value(void)
{
    wallClock = RTCDRV_GetWallClock();
    hh = wallClock/3600;
    mm = (wallClock - hh*3600)/60;
    ss = wallClock - hh*3600 - mm*60;
    sl_app_log(" %" PRIu32 "s \n", wallClock);
    sl_app_log(" %d:%d:%d \n", hh,mm,ss);
}
