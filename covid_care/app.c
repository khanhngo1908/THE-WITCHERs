/***************************************************************************//**
 * @file
 * @brief Core application logic.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
#include "em_common.h"
#include "app_assert.h"
#include "sl_bluetooth.h"
#include "gatt_db.h"
#include "app.h"
#include "sl_app_log.h"
#include "em_chip.h"
#include "sl_simple_timer.h"

#include "sl_sleeptimer.h"

/*---- Include User's Header ----*/
#include "i2c_lib.h"
#include "lm75.h"
#include "max30102.h"
#include "bpm_spo2_calc.h"
#include "gpio_intr.h"
#include "led_buzzer.h"
#include "msc.h"
#include "mpu6050.h"
#include "test_variable.h"
#include "time.h"
#include "em_rtcc.h"
#include "function.h"
uint8_t notifyEnabled = false;
uint8_t indicateEnabled = false;
uint8_t app_connection;

// The advertising set handle allocated from Bluetooth stack.
static uint8_t advertising_set_handle = 0xff;

/*----------------- Define Area -----------------*/
#define TIMER_S(s) (s * 32768)
#define TIMER_MS(ms) ((ms * 32768) / 1000)
#define TEMPERATURE 0
#define SEC 1
#define MIIN 2
#define BUTTON_PRESS_TIMER 3
/*MPU6050 */
volatile bool mpuInterrupt = false; // indicates whether MPU interrupt pin has gone high
void dmpDataReady ()
{
	mpuInterrupt = true;
}
struct MPU6050_Base mpu;
uint16_t packetSize;
uint8_t devStatus;
uint8_t mpuIntStatus;
bool dmpReady = false;
struct tm time_date;
uint32_t diff;
uint8_t check_count;
uint8_t check_fall = 0;

sl_sleeptimer_date_t date_disconnect;
sl_sleeptimer_date_t datetest;

uint8_t button_press_timerCounter = 0;
uint8_t button_press_counter = 0;
uint8_t help = 0;

uint8_t memory_data_header = 0;
uint8_t unReadCounter = 0;
uint8_t dataCounter = 0;
uint8_t dataPointer = 0;

uint8_t caution = 0;
uint8_t cautionCounter = 0;

uint8_t timerCounter = 0;

uint8_t newUnreadDataHeader = 0;

float T;
BPM_SpO2_value_t bpm_spo2_value;
/**************************************************************************//**
 * Application Init.
 *****************************************************************************/
SL_WEAK void app_init (void)
{
	/////////////////////////////////////////////////////////////////////////////
	// Put your additional application init code here!                         //
	// This is called once during start-up.                                    //
	/////////////////////////////////////////////////////////////////////////////
	sl_app_log("\nInitiation.... \n");

	// Chip init
	CHIP_Init ();
	sl_app_log(" Chip init Ok \n");

	// LED & Buzzer init
	led_buzzer_init ();
	sl_app_log(" LED & buzzer init Ok \n");

	set_LED ('W');

	// I2C init
	i2c_init ();
	sl_app_log(" I2C init Ok \n");

	// Max30102 init
	MAX30102_init ();
	sl_app_log(" MAX30102 init Ok \n");

	// GPIO Interrupt init
	gpio_INTR_init ();
	sl_app_log(" GPIO Intr init Ok \n");

	// MPU6050init
//	MPU6050_ConfigDMP (&mpu, &devStatus, &dmpReady, &mpuIntStatus, &packetSize);
//	sl_app_log(" MPU6050 init Ok \n");

	// MSC init
	MSC_init ();
//	MSC_Clear ();
	MSC_CheckPage (&unReadCounter, &dataCounter);
	memory_data_header = unReadCounter;
	sl_app_log(" MSC init Ok \n");

	set_Buzzer ();
	sl_sleeptimer_delay_millisecond (600);
	clear_Buzzer ();
	clear_all_LED ();
	sl_app_log("Init OK \n\n");

//	MSC_PrintPage ();

	sl_sleeptimer_get_datetime (&datetest);
	sl_app_log(" Runtime: %d %d %d \n ", datetest.hour, datetest.min, datetest.sec);
	sl_bt_system_set_soft_timer (TIMER_S(60 * 2), MIIN, 0);
}

/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
SL_WEAK void app_process_action (void)
{

	/*********************** Duong's Process **********************************/
//	MPU6050_GetData (&mpu, &dmpReady, &mpuInterrupt, &packetSize, &mpuIntStatus,
//					 &check_fall);
//	if (check_fall == 1)
//	{
//		sl_bt_system_set_soft_timer (TIMER_MS(5000), SEC, 1);
//	}
	if (help == 1)
	{
		set_LED ('R');
		set_Buzzer ();
	}
	else
	{
		clear_Buzzer ();
		clear_all_LED ();
		if (caution == 1)
		{
			set_LED ('R');
			if (cautionCounter >= 2)
				set_Buzzer ();
		}
	}
}

void process_server_user_write_request (sl_bt_msg_t *evt)
{
	sl_status_t sc;
	uint32_t connection =
			evt->data.evt_gatt_server_user_write_request.connection;
	uint32_t characteristic =
			evt->data.evt_gatt_server_user_write_request.characteristic;
	sc = sl_bt_gatt_server_send_user_write_response (connection, characteristic,
													 0);

	if (characteristic == gattdb_device_ch)
	{

		uint8_t header = evt->data.evt_gatt_server_attribute_value.value.data[0];
		uint8_t header1 =
				evt->data.evt_gatt_server_attribute_value.value.data[1];
		uint8_t header2 =
				evt->data.evt_gatt_server_attribute_value.value.data[2];
		uint8_t len = evt->data.evt_gatt_server_attribute_value.value.len;

		app_log("header: %d %d %d - len: %d \n", header, header1, header2, len);

		if (header == 1 && len == 7)
		{ //set date time
			datetest.time_zone = 0;
			datetest.year =
					evt->data.evt_gatt_server_attribute_value.value.data[3];
			datetest.month =
					evt->data.evt_gatt_server_attribute_value.value.data[2];
			datetest.month_day =
					evt->data.evt_gatt_server_attribute_value.value.data[1];
			datetest.hour =
					evt->data.evt_gatt_server_attribute_value.value.data[4];
			datetest.min =
					evt->data.evt_gatt_server_attribute_value.value.data[5];
			datetest.sec =
					evt->data.evt_gatt_server_attribute_value.value.data[6];
			sl_sleeptimer_set_datetime (&datetest);
			sl_app_log(" Connection open time: %d %d %d \n ", datetest.hour, datetest.min, datetest.sec);
			if (newUnreadDataHeader == 1)
			{
				MSC_CheckPage (&unReadCounter, &dataCounter);
				memory_data_header = dataCounter;
				uint8_t numOfUnReadData = dataCounter - unReadCounter;
				uint8_t readAll[7 * numOfUnReadData];
				uint8_t read[9];
				uint8_t read_temp[9];
				uint8_t i;
				uint16_t temp_mem = 0;
				uint16_t spo2_mem = 0;
				uint16_t bpm_mem = 0;
				uint8_t count_time = 0;
				uint8_t count_data = 0;
				MSC_read (read_temp, unReadCounter);
				for (i = unReadCounter; i < dataCounter; i++)
				{
					sl_app_log("%d \n", i);
					MSC_read (read, i);
					if (read[2] == read_temp[2])
					{
						if (read[1] == read_temp[1])
						{
							if (read[4] == read_temp[4])
							{
								temp_mem += read[8];
								spo2_mem += read[7];
								bpm_mem += read[6];
								count_time++;
							}
							else
							{
								temp_mem = temp_mem / count_time;
								spo2_mem = spo2_mem / count_time;
								bpm_mem = bpm_mem / count_time;
								readAll[count_data * 7] = read_temp[1];
								readAll[count_data * 7 + 1] = read_temp[2];
								readAll[count_data * 7 + 2] = read_temp[3];
								readAll[count_data * 7 + 3] = read_temp[4];
								readAll[count_data * 7 + 4] = temp_mem;
								readAll[count_data * 7 + 5] = spo2_mem;
								readAll[count_data * 7 + 6] = bpm_mem;
								temp_mem = 0;
								spo2_mem = 0;
								bpm_mem = 0;
								count_time = 0;
								uint8_t k;
								for (k = 0; k < 9; k++)
								{
									read_temp[k] = read[k];
								}
								temp_mem += read[8];
								spo2_mem += read[7];
								bpm_mem += read[6];
								count_time++;
								count_data++;
							}
						}
						else
						{
							temp_mem = temp_mem / count_time;
							spo2_mem = spo2_mem / count_time;
							bpm_mem = bpm_mem / count_time;
							readAll[count_data * 7] = read_temp[1];
							readAll[count_data * 7 + 1] = read_temp[2];
							readAll[count_data * 7 + 2] = read_temp[3];
							readAll[count_data * 7 + 3] = read_temp[4];
							readAll[count_data * 7 + 4] = temp_mem;
							readAll[count_data * 7 + 5] = spo2_mem;
							readAll[count_data * 7 + 6] = bpm_mem;
							temp_mem = 0;
							spo2_mem = 0;
							bpm_mem = 0;
							count_time = 0;
							uint8_t k;
							for (k = 0; k < 9; k++)
							{
								read_temp[k] = read[k];
							}
							temp_mem += read[8];
							spo2_mem += read[7];
							bpm_mem += read[6];
							count_time++;
							count_data++;
						}
					}
					else
					{
						temp_mem = temp_mem / count_time;
						spo2_mem = spo2_mem / count_time;
						bpm_mem = bpm_mem / count_time;
						readAll[count_data * 7] = read_temp[1];
						readAll[count_data * 7 + 1] = read_temp[2];
						readAll[count_data * 7 + 2] = read_temp[3];
						readAll[count_data * 7 + 3] = read_temp[4];
						readAll[count_data * 7 + 4] = temp_mem;
						readAll[count_data * 7 + 5] = spo2_mem;
						readAll[count_data * 7 + 6] = bpm_mem;
						temp_mem = 0;
						spo2_mem = 0;
						bpm_mem = 0;
						count_time = 0;
						uint8_t k;
						for (k = 0; k < 9; k++)
						{
							read_temp[k] = read[k];
						}
						temp_mem += read[8];
						spo2_mem += read[7];
						bpm_mem += read[6];
						count_time++;
						count_data++;
					}
					if (i == dataCounter - 1)
					{
						temp_mem = temp_mem / count_time;
						spo2_mem = spo2_mem / count_time;
						bpm_mem = bpm_mem / count_time;
						readAll[count_data * 7] = read[1];
						readAll[count_data * 7 + 1] = read[2];
						readAll[count_data * 7 + 2] = read[3];
						readAll[count_data * 7 + 3] = read[4];
						readAll[count_data * 7 + 4] = temp_mem;
						readAll[count_data * 7 + 5] = spo2_mem;
						readAll[count_data * 7 + 6] = bpm_mem;
						count_data++;
					}
				}
				uint8_t final_arr[count_data * 7];
				uint16_t len_check = sizeof(final_arr) / sizeof(uint8_t);
				uint8_t z1;
				for (z1 = 0; z1 < len_check; z1++)
				{
					final_arr[z1] = readAll[z1];
				}
				uint8_t y;
				for (y = 0; y < len_check; y++)
				{
					printf ("%d ", final_arr[y]);
				}
				send_all_old_data (&notifyEnabled, &app_connection, final_arr,
								   &count_data);
				newUnreadDataHeader = 0;
			}

		}
		else if (header == 5 && len == 1)
		{
			sl_app_log(" Gui data manual - nut nhan app \n");
			set_LED ('W');
			float T;
			uint8_t i;
			for (i = 0; i < 3; i++)
			{
				T = LM75_ReadTemperature ();
				uint8_t res = BPM_SpO2_Update (&bpm_spo2_value, i);
				sl_app_log(" Nhiet do: %d \n", (uint32_t ) (1000 * T));
				sl_app_log(" SpO2: %d \n", bpm_spo2_value.SpO2);
				sl_app_log(" BPM: %d \n", bpm_spo2_value.BPM);

				float spo2 = (float) (bpm_spo2_value.SpO2);
				float bpm = (float) (bpm_spo2_value.BPM);
				send_all_data_count (&notifyEnabled, &app_connection, &T, &spo2,
									 &bpm, i);

				if (T > 38 || res == 1)
					caution = 1;
				else
					caution = 0;
			}
			clear_all_LED ();
		}

	}
}
/**************************************************************************//**
 * Bluetooth stack event handler.
 * This overrides the dummy weak implementation.
 *
 * @param[in] evt Event coming from the Bluetooth stack.
 *****************************************************************************/
void sl_bt_on_event (sl_bt_msg_t *evt)
{
	sl_status_t sc;
	bd_addr address;
	uint8_t address_type;
	uint8_t system_id[8];

	switch (SL_BT_MSG_ID(evt->header))
	{
		// -------------------------------
		// This event indicates the device has started and the radio is ready.
		// Do not call any stack command before receiving this boot event!
		case sl_bt_evt_system_boot_id:

			// Extract unique ID from BT Address.
			sc = sl_bt_system_get_identity_address (&address, &address_type);
			app_assert_status(sc);

			// Pad and reverse unique ID to get System ID.
			system_id[0] = address.addr[5];
			system_id[1] = address.addr[4];
			system_id[2] = address.addr[3];
			system_id[3] = 0xFF;
			system_id[4] = 0xFE;
			system_id[5] = address.addr[2];
			system_id[6] = address.addr[1];
			system_id[7] = address.addr[0];

			sc = sl_bt_gatt_server_write_attribute_value (gattdb_system_id, 0,
														  sizeof(system_id),
														  system_id);
			app_assert_status(sc);

			// Create an advertising set.
			sc = sl_bt_advertiser_create_set (&advertising_set_handle);
			app_assert_status(sc);

			// Set advertising interval to 100ms.
			sc = sl_bt_advertiser_set_timing (advertising_set_handle, 160, // min. adv. interval (milliseconds * 1.6)
											  160, // max. adv. interval (milliseconds * 1.6)
											  0,   // adv. duration
											  0);  // max. num. adv. events
			app_assert_status(sc);
			// Start general advertising and enable connections.
			sc = sl_bt_advertiser_start (
					advertising_set_handle,
					sl_bt_advertiser_general_discoverable,
					sl_bt_advertiser_connectable_scannable);
			app_assert_status(sc);
			break;

			// -------------------------------
			// This event indicates that a new connection was opened.
		case sl_bt_evt_connection_opened_id:
			app_connection = evt->data.evt_connection_opened.connection;
			sl_app_log("connection opened \n");
			uint32_t diff_t = diff_time (&date_disconnect);
			break;

			// -------------------------------
			// This event indicates that a connection was closed.
		case sl_bt_evt_connection_closed_id:
			// Restart advertising after client has disconnected.
			sc = sl_bt_advertiser_start (
					advertising_set_handle,
					sl_bt_advertiser_general_discoverable,
					sl_bt_advertiser_connectable_scannable);
			sl_app_log("connection closed \n");
			sl_sleeptimer_get_datetime (&date_disconnect);
			app_assert_status(sc);
			app_connection = 0;
			break;

			///////////////////////////////////////////////////////////////////////////
			// Add additional event handlers here as your application requires!      //
			///////////////////////////////////////////////////////////////////////////

		case sl_bt_evt_gatt_server_characteristic_status_id:
			if (gatt_server_client_config
					== (gatt_server_characteristic_status_flag_t) evt->data.evt_gatt_server_characteristic_status.status_flags)
			{
				if (evt->data.evt_gatt_server_characteristic_status.client_config_flags
						== 1)
				{
					notifyEnabled = true;
					app_log("Enable temperature characteristic\n");
				}
				else
				{
					notifyEnabled = false;
					app_log("Disable temperature characteristic\n");
				}
			}
			break;
		case sl_bt_evt_gatt_server_user_write_request_id:
			process_server_user_write_request (evt);

			break;
		case sl_bt_evt_system_soft_timer_id:
			if (evt->data.evt_system_soft_timer.handle == SEC)
			{
				if (check_fall == 0)
				{
					set_LED ('R');
				}
				// get data, check connect_condition => push memory
			}
			if (evt->data.evt_system_soft_timer.handle == MIIN)
			{
//				sl_sleeptimer_date_t datetest;
				sl_sleeptimer_get_datetime (&datetest);
				sl_app_log(" G: %d %d %d \n ", datetest.hour, datetest.min, datetest.sec);
				if (GPIO_PinInGet (button_port, button_pin))
				{

					sl_app_log(" Gui data dinh ky \n");
					set_LED ('W');
					float T = LM75_ReadTemperature ();
					uint8_t res = BPM_SpO2_Update (&bpm_spo2_value, 0);
					sl_app_log(" Nhiet do: %d \n", (uint32_t ) (1000 * T));
					sl_app_log(" SpO2: %d \n", bpm_spo2_value.SpO2);
					sl_app_log(" BPM: %d \n", bpm_spo2_value.BPM);

					if (app_connection == 0)
					{
						sl_sleeptimer_date_t date_current;
						sl_sleeptimer_get_datetime (&date_current);
						if (date_current.year != 70)
						{
							// ghi v�o memory
							uint8_t res = MSC_CheckPage (&unReadCounter,
														 &dataCounter);
							if (res == 1)
								memory_data_header = unReadCounter;
							sl_app_log(" unread: %d; counter: %d \n",
									   unReadCounter, dataCounter);
							dataPointer = dataCounter;
							uint8_t data[9];
							uint8_t t = LM75_FloatToOneByte (T);

							// l?y ngay, thang, nam, gio, phut
							sl_sleeptimer_get_datetime (&datetest);

							// gan vao mang
							data[0] = memory_data_header;
							data[1] = datetest.month_day; 			// ng�y
							data[2] = datetest.month + 1;			// th�ng
							data[3] = datetest.year;				// nam;
							data[4] = datetest.hour;				// gi?;
							data[5] = datetest.min;					// ph�t;
							data[6] = bpm_spo2_value.BPM;		// nhip tim
							data[7] = bpm_spo2_value.SpO2;			// spo2
							data[8] = t;						// nhiet do

							// viet vao bo nho
							MSC_write (data, dataPointer);

							// Doc lai de kiem tra
							uint8_t read[9];
							MSC_read (read, dataPointer);
							newUnreadDataHeader = 1;
						}
					}
					else
					{
						float spo2 = (float) (bpm_spo2_value.SpO2);
						float bpm = (float) (bpm_spo2_value.BPM);
						send_all_data (&notifyEnabled, &app_connection, &T,
									   &spo2, &bpm);
					}

					if (T > 38 || res == 1)
					{
						if (caution == 1)
						{
							cautionCounter += 1;
						}
						caution = 1;
						sl_bt_system_set_soft_timer (TIMER_S(20), MIIN, 0);

					}
					else
					{
						caution = 0;
						sl_bt_system_set_soft_timer (TIMER_S(60* 2), MIIN, 0);
						cautionCounter = 0;
					}

					clear_all_LED ();
				}
			}
			if (evt->data.evt_system_soft_timer.handle == BUTTON_PRESS_TIMER)
			{
				button_press_timerCounter += 1;
				sl_app_log(" button_press_timerCounter: %d \n",
						   button_press_timerCounter);
				if (!GPIO_PinInGet (button_port, button_pin))
				{
					if (button_press_timerCounter >= LONG_PRESS_TIMEOUT)
					{
						sl_bt_system_set_soft_timer (TIMER_MS(0),
						BUTTON_PRESS_TIMER,
													 1);

						if (help == 0)
						{
							help = 1;
						}
						else if (help == 1)
						{
							help = 0;
						}
						button_press_counter = 0;
						button_press_timerCounter = 0;
					}
				}
				else
				{
					sl_bt_system_set_soft_timer (TIMER_MS(0),
					BUTTON_PRESS_TIMER,
												 1);
					if (button_press_timerCounter > DEBOUND_TIMEOUT
							&& button_press_timerCounter < SINGLE_PRESS_TIMEOUT)
					{
						sl_app_log(" Gui data manual - nut nhan mach \n");
						set_LED ('W');
						float T;
						uint8_t i;
						for (i = 0; i < 3; i++)
						{
							T = LM75_ReadTemperature ();
							uint8_t res = BPM_SpO2_Update (&bpm_spo2_value, i);
							sl_app_log(" Nhiet do: %d \n",
									   (uint32_t ) (1000 * T));
							sl_app_log(" SpO2: %d \n", bpm_spo2_value.SpO2);
							sl_app_log(" BPM: %d \n", bpm_spo2_value.BPM);

							float spo2 = (float) (bpm_spo2_value.SpO2);
							float bpm = (float) (bpm_spo2_value.BPM);
							send_all_data_count (&notifyEnabled,
												 &app_connection, &T, &spo2,
												 &bpm, i);

							if (T > 38 || res == 1)
								caution = 1;
							else
								caution = 0;
						}
						clear_all_LED ();
					}
					button_press_timerCounter = 0;
					button_press_counter = 0;
				}
			}
			break;
		case sl_bt_evt_system_external_signal_id:
			if (evt->data.evt_system_external_signal.extsignals == button_pin)
			{
				if (button_press_counter == 0)
				{
					sl_bt_system_set_soft_timer (TIMER_MS(100),
					BUTTON_PRESS_TIMER,
												 0);
					button_press_counter += 1;
				}
			}
			if (evt->data.evt_system_external_signal.extsignals == mpu_intr_pin)
			{
				dmpDataReady ();
			}
			break;
			// -------------------------------
			// Default event handler.
		default:
			break;
	}
}
