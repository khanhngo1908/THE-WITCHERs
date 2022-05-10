/*
 * max30102.c
 *
 *  Created on: 23 thg 2, 2022
 *      Author: Ngo Minh Khanh
 */

#include "i2c_lib.h"
#include "max30102.h"
#include "sl_app_log.h"

/**
 * @brief      MAX30102 initial
 *
 */
void MAX30102_init ()
{
	// Reset
	i2c_writeByte (MAX30102_ADDRESS, REG_MODE_CONFIG, MAX30102_RESET);
	sl_sleeptimer_delay_millisecond (500);

	// Mode Configuration
	i2c_writeByte (MAX30102_ADDRESS, REG_MODE_CONFIG, MAX30102_MODE_SPO2_HR);

	//FIFO Configuration
	i2c_writeByte (MAX30102_ADDRESS, REG_FIFO_CONFIG, smp2 | FIFO_ROLLOVER_EN );

	// Enable FIFO Almost Full Interrupt
//    i2c_writeByte(MAX30102_ADDRESS, REG_INTR_ENABLE_1 , INT_A_FULL_EN);

// LED Pulse Amplitude Configuration
	i2c_writeByte (MAX30102_ADDRESS, REG_LED1_PA, FIX_CURRENT);
	i2c_writeByte (MAX30102_ADDRESS, REG_LED2_PA, FIX_CURRENT);

	// SpO2 Configuration
	i2c_writeByte (MAX30102_ADDRESS, REG_SPO2_CONFIG, adc16384 | sr200 | pw411);

	// Clear FIFO
	MAX30102_ClearFIFO ();

	// Shutdown
	MAX30102_Shutdown ();

	sl_sleeptimer_delay_millisecond (500);
}

/**
 * @brief     	Read data from FIFO register
 *
 * @param[out]	fifo_t	-	fifo_t data type
 */
void MAX30102_ReadFIFO (fifo_t *result)
{
	uint8_t writePointer = 0, readPointer = 0;
	i2c_read (MAX30102_ADDRESS, REG_FIFO_WR_PTR, &writePointer, 1);
	i2c_read (MAX30102_ADDRESS, REG_FIFO_RD_PTR, &readPointer, 1);

	int16_t num_samples = 0;

	if (writePointer != readPointer)
	{
		uint16_t raw_IR = 0;
		uint16_t raw_RED = 0;
		num_samples = (int16_t) writePointer - (int16_t) readPointer;
		if (num_samples < 0)
			num_samples += 32;

		int16_t bytesLeftToRead = num_samples * BYTES_PER_SAMPLE;

		while (bytesLeftToRead > 0)
		{
			int16_t toGet = bytesLeftToRead;

			if (toGet > I2C_BUFFER_LENGTH)
				toGet = I2C_BUFFER_LENGTH - (I2C_BUFFER_LENGTH % BYTES_PER_SAMPLE);

//			if (toGet > MAX30102_TOTAL_BYTES)
//				toGet = MAX30102_TOTAL_BYTES - (MAX30102_TOTAL_BYTES % BYTES_PER_SAMPLE);


			bytesLeftToRead -= toGet;

			while (toGet > 0)
			{
				if (result->cnt > (STORAGE_SIZE - 1))
				{
					result->cnt = STORAGE_SIZE;
					break;
				}

				uint8_t sample[BYTES_PER_SAMPLE];
				i2c_read (MAX30102_ADDRESS, REG_FIFO_DATA, sample,
						  BYTES_PER_SAMPLE);
				raw_RED = ((uint16_t) (sample[0] << 16)
						| (uint16_t) (sample[1] << 8) | (uint16_t) (sample[2]))
						& 0x3ffff;
				raw_IR = ((uint16_t) (sample[3] << 16)
						| (uint16_t) (sample[4] << 8) | (uint16_t) (sample[5]))
						& 0x3ffff;

				result->IR[result->cnt] = raw_IR;
				result->RED[result->cnt] = raw_RED;

//				sl_app_log("Cnt: %d - IR: %d - RED: %d \n", result->cnt, raw_IR, raw_RED);
//				sl_app_log("%d %d \n", raw_IR, raw_RED);
//				sl_app_log("%d \n", raw_IR);
//				sl_app_log("%d \n", raw_RED);

				result->cnt += 1;
				toGet -= BYTES_PER_SAMPLE;
			}
		}  //End while (bytesLeftToRead > 0)
	} //End readPtr != writePtr

}

/**
 * @brief      Set MAX30102 into shutdown mode
 *
 */
void MAX30102_Shutdown ()
{
	uint8_t config;
	i2c_read (MAX30102_ADDRESS, REG_MODE_CONFIG, &config, 1);
	config = config | 0x80;
	i2c_writeByte (MAX30102_ADDRESS, REG_MODE_CONFIG, config);
}

/**
 * @brief      Enable MAX30102
 *
 */
void MAX30102_Continue ()
{
	uint8_t config;
	i2c_read (MAX30102_ADDRESS, REG_MODE_CONFIG, &config, 1);
	config = config & 0x7F;
	i2c_writeByte (MAX30102_ADDRESS, REG_MODE_CONFIG, config);
}

/**
 * @brief      	Clear data in FIFO register
 *
 */
void MAX30102_ClearFIFO ()
{
	i2c_writeByte (MAX30102_ADDRESS, REG_FIFO_WR_PTR, 0x00);
	i2c_writeByte (MAX30102_ADDRESS, REG_OVF_COUNTER, 0x00);
	i2c_writeByte (MAX30102_ADDRESS, REG_FIFO_RD_PTR, 0x00);
}
//void MAX30102_CheckReg (void)
//{
////    i2c_read(MAX30102_ADDRESS, REG_MODE_CONFIG, &data, 1);
////    sl_app_log(" Mode Config Reg: %x\n", data);
////    i2c_read(MAX30102_ADDRESS, REG_FIFO_CONFIG, &data, 1);
////    sl_app_log(" FIFO Config Reg: %x\n", data);
////    i2c_read(MAX30102_ADDRESS, REG_SPO2_CONFIG, &data, 1);
////    sl_app_log(" SpO2 Config Reg: %x\n", data);
////    i2c_read(MAX30102_ADDRESS, REG_INTR_ENABLE_1, &data, 1);
////    sl_app_log(" Int Enable1 Reg: %x\n", data);
//	i2c_read (MAX30102_ADDRESS, REG_INTR_STATUS_1, &data, 1);
//	sl_app_log(" Int Status1 Reg: %x\n", data);
//	i2c_read (MAX30102_ADDRESS, REG_INTR_STATUS_2, &data, 1);
//	sl_app_log(" Int Status2 Reg: %x\n", data);
//	sl_app_log(" ---------------------------------\n");
//}
//
//void MAX30102_ClearIntr (void)
//{
//	i2c_read (MAX30102_ADDRESS, REG_INTR_STATUS_1, &data, 1);
//	i2c_read (MAX30102_ADDRESS, REG_INTR_STATUS_2, &data, 1);
//}
