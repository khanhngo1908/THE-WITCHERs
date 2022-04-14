/*
 * msc.c
 *
 *  Created on: 6 thg 4, 2022
 *      Author: Admin
 */
#include "msc.h"
#include "em_cmu.h"
#include "sl_app_log.h"


void MSC_init (void)
{
	// Enable MSC Clock
	CMU_ClockEnable (cmuClock_MSC, true);
}

/** msc_DataCount phai la so chan */
uint8_t MSC_write (uint8_t *data, uint8_t *msc_DataPointer)
{
	if( (*msc_DataPointer) % 2 != 0 )
		return 0;
	if( data[0] % 2 != 0)
		return 0;

	data[0] = data[0] / 2;

	uint8_t tmp1 = (data[0] << 2) | (data[1] >> 6);
	uint8_t tmp2 = ((data[1] & 0x0f) << 4) | data[2];

	uint32_t word1 = 0x01;
	uint32_t word2 = 0x02;

	word1 = (tmp1 << 24) | (tmp2 << 16) | (data[3] << 8) | data[4];
	word2 = (data[5] << 24) | (data[6] << 16) | (data[7] << 8) | data[8];

//	sl_app_log(" w1: %x \n", word1);
//	sl_app_log(" w2: %x \n", word2);

	sl_app_log("1 \n");
	MSC_Init ();
	MSC_WriteWord ((USERDATA + (*msc_DataPointer) ), &word1, 4);
	(*msc_DataPointer) += 1;
	sl_app_log("2 \n");
	MSC_WriteWord ((USERDATA + (*msc_DataPointer) ), &word2, 4);
	(*msc_DataPointer) += 1;
	sl_app_log("3 \n");
	MSC_Deinit ();
	return 1;
}

/** msc_DataCount phai la so chan */
void MSC_read (uint8_t *data, uint8_t msc_DataPointer)
{
	if( (msc_DataPointer) % 2 != 0 )
		return;

	uint32_t word1;
	uint32_t word2;

	word1 = USERDATA[msc_DataPointer];
	word2 = USERDATA[msc_DataPointer + 1];

	data[0] = (word1 >> 26) * 2;            	// un-read counter
	data[1] = (word1 >> 20) & 0x3F;				// ngay
	data[2] = (word1 >> 16) & 0x0f;				// thang
	data[3] = (word1 >> 8) & 0xff;				// nam
	data[4] = word1 & 0xff;						// gio
	data[5] = word2 >> 24;						// phut
	data[8] = (word2 >> 16) & 0xff;				// BPM
	data[7] = (word2 >> 8) & 0xff;				// SpO2
	data[8] = word2 & 0xff;						// Nhiet do

	sl_app_log(" %d \n", data[0]);
	sl_app_log(" %d %d %d %d \n", data[1], data[2], data[3], data[4]);
	sl_app_log(" %d %d %d %d \n", data[5], data[6], data[7], data[8]);
}

void MSC_CheckUnRead(uint8_t *unReadCounter, uint8_t *dataCounter)
{
	*unReadCounter = 0;
	*dataCounter = 0;
	uint32_t check = 0;
	uint8_t i = 0;
	while(i < MSC_MAX_COUNTER)
	{
		check = USERDATA[i];
//		sl_app_log(" i: %d - data: %x \n", i, check);
		if(check == 0xffffffff)
		{
			break;
		}
		i+=2;
	}

	*dataCounter = i;
	if(i == 0)
	{
		*unReadCounter = 0;
	}
	else
	{
		check = USERDATA[i-2];
		*unReadCounter = (check >> 26) * 2;
	}
}

void MSC_CheckPage()
{
	uint8_t i;
	for(i = 0; i < MSC_MAX_COUNTER; i+=2 )
	{
		sl_app_log(" CNT: %d - Data: %x \n", i, USERDATA[i]);
	}
}
