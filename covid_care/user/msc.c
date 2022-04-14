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

void MSC_write (uint8_t *data, uint8_t *msc_wordCount)
{
	uint32_t word1 = 0;
	uint32_t word2 = 0;
	word1 = (uint32_t) (data[0] << 24) | (uint32_t) (data[1] << 16) | (uint32_t) (data[2] << 8) | (uint32_t) data[3];
	word2 = (uint32_t) (data[4] << 24) | (uint32_t) (data[5] << 16) | (uint32_t) (data[6] << 8) | (uint32_t) data[7];

	MSC_Init ();
	MSC_WriteWord ((USERDATA + (*msc_wordCount) ), &word1, 4);
	(*msc_wordCount) += 1;
	MSC_WriteWord ((USERDATA + (*msc_wordCount) ), &word2, 4);
	(*msc_wordCount) += 1;
	MSC_Deinit ();
}

void MSC_read (uint8_t *data, uint8_t msc_wordCount)
{
	uint32_t word1;
	uint32_t word2;

	word1 = USERDATA[msc_wordCount];
	word2 = USERDATA[msc_wordCount + 1];

	data[0] = word1 >> 24;
	data[1] = (word1 >> 16) & 0xff;
	data[2] = (word1 >> 8) & 0xff;
	data[3] = word1 & 0xff;
	data[4] = word2 >> 24;
	data[5] = (word2 >> 16) & 0xff;
	data[6] = (word2 >> 8) & 0xff;
	data[7] = word2 & 0xff;

//	sl_app_log (" %d %d %d %d \n", data[0], data[1], data[2], data[3]);
//	sl_app_log (" %d %d %d %d \n", data[4], data[5], data[6], data[7]);
}

