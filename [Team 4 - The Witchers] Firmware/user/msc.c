/*
 * msc.c
 *
 *  Created on: 6 thg 4, 2022
 *      Author: Admin
 */
#include "msc.h"
#include "em_cmu.h"
#include "sl_app_log.h"
#include "lm75.h"

void MSC_init (void)
{
	// Enable MSC Clock
	CMU_ClockEnable (cmuClock_MSC, true);
}

/**
 * @brief      	Write data to memory
 *
 * @param[in]  data   -   data needs to be written
 * @param[in]  msc_DataPointer - word ordinal number of the data to be written
 */
void MSC_write (uint8_t *data, uint8_t msc_DataPointer)
{
	uint8_t tmp = msc_DataPointer;

	if(tmp>127)
		return;

	uint32_t word1 = 0;
	uint32_t word2 = 0;

	word1 = (data[0] << 21) | (data[1] << 16) | (data[2] << 12) | (data[3] << 5) | data[4];
	word2 = (data[5] << 23) | (data[6] << 15) | (data[7] << 8) | data[8];

	sl_app_log("    Word1: %x - Word 2: %x \n", word1, word2);

	MSC_Init ();
	MSC_WriteWord ((USERDATA + (2*tmp) ), &word1, 4);
	MSC_WriteWord ((USERDATA + (2*tmp + 1) ), &word2, 4);
	MSC_Deinit ();
}

/**
 * @brief      	Write data to memory
 *
 * @param[out]  data   -   data is readout
 * @param[in]  msc_DataPointer - word ordinal number of the data to be read
 */
void MSC_read (uint8_t *data, uint8_t msc_DataPointer)
{
	if (msc_DataPointer > 127)
		return;

	uint32_t word1;
	uint32_t word2;

	word1 = USERDATA[2*msc_DataPointer];
	word2 = USERDATA[2*msc_DataPointer + 1];

	data[0] = word1 >> 21;          			// un-read counter
	data[1] = (word1 >> 16) & 0x1f;				// ngay
	data[2] = (word1 >> 12) & 0x0f;				// thang
	data[3] = (word1 >> 5) & 0x7f;				// nam
	data[4] = word1 & 0x1f;						// gio

	data[5] = word2 >> 23;						// phut
	data[6] = (word2 >> 15) & 0xff;				// BPM
	data[7] = (word2 >> 8) & 0x7f;				// SpO2
	data[8] = word2 & 0xff;						// Nhiet do

	float t = LM75_OneByteToFloat (data[8]);

	sl_app_log("    Word1: %x - Word 2: %x \n", word1, word2);
	sl_app_log("    %d \n", data[0]);
	sl_app_log("    %d %d %d %d %d \n", data[1], data[2], data[3], data[4], data[5]);
	sl_app_log("    %d %d %d \n", data[6], data[7], (uint32_t) (1000 * t) );
}

/**
 * @brief      	Check User Data page
 *
 * @param[out]  unReadCounter   -   unread word ordinal number
 * @param[out]  msc_DataPointer - word ordinal number of the data to be written
 * @return 	Is the page deleted?
 */
uint8_t MSC_CheckPage(uint8_t *unReadCounter, uint8_t *dataCounter)
{
	*unReadCounter = 0;
	*dataCounter = 0;
	uint32_t check = 0;
	uint8_t i = 0;
	while(i < MSC_MAX_COUNTER)
	{
		check = USERDATA[2*i];
//		sl_app_log(" i: %d - data: %x \n", i, check);
		if(check == 0xffffffff)
		{
			break;
		}
		i+=1;
	}

	if(i == MSC_MAX_COUNTER )
	{
		MSC_Clear ();
		*unReadCounter = 0;
		*dataCounter = 0;
		return 1;               // check clear page
	}

	*dataCounter = i;
	if(i == 0)
	{
		*unReadCounter = 0;
	}
	else
	{
		check = USERDATA[2*(i-1)];
		*unReadCounter = check >> 21;
	}
	return 0;
}

/**
 * @brief      	Print User Data page
 */
void MSC_PrintPage()
{
	uint32_t word1;
	uint32_t word2;
	uint8_t data[9];

	uint8_t i;
	for(i = 0; i < MSC_MAX_COUNTER; i++ )
	{
		word1 = USERDATA[2 * i];
		word2 = USERDATA[2 * i + 1];

		data[0] = word1 >> 21;          			// un-read counter
		data[1] = (word1 >> 16) & 0x1f;				// ngay
		data[2] = (word1 >> 12) & 0x0f;				// thang
		data[3] = (word1 >> 5) & 0x7f;				// nam
		data[4] = word1 & 0x1f;						// gio

		data[5] = word2 >> 23;						// phut
		data[6] = (word2 >> 15) & 0xff;				// BPM
		data[7] = (word2 >> 8) & 0x7f;				// SpO2
		data[8] = word2 & 0xff;						// Nhiet do

		float t = LM75_OneByteToFloat (data[8]);

		sl_app_log("%d \n", i);
		sl_app_log("    Word1: %x - Word 2: %x \n", word1, word2);
		sl_app_log("    %d \n", data[0]);
		sl_app_log("    %d %d %d %d %d \n", data[1], data[2], data[3], data[4],
				   data[5]);
		sl_app_log("    %d %d %d \n", data[6], data[7], (uint32_t ) (1000 * t));
	}
}


/**
 * @brief      	Clear User Data page
 */
void MSC_Clear()
{
	MSC_ErasePage(USERDATA);
}
