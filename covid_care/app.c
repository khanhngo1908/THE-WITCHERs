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
#define TIMER_MS(ms) ((32768 * ms) / 1000)
#define TEMPERATURE 0
#define SEC 1
#define MIIN 2
#define BUTTON_PRESS_TIMER 3
/*MPU6050 */
volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
void dmpDataReady() {
  mpuInterrupt = true;
}
struct MPU6050_Base mpu;
uint16_t packetSize;
uint8_t devStatus;
uint8_t mpuIntStatus;
bool dmpReady = false;
struct tm time_date;
float te = 36.125;
float spo2 = 96.0;
float bmp = 80.0;
uint32_t diff;
uint8_t check_count;
uint8_t check_fall = 0;
sl_sleeptimer_date_t date_disconnect;
sl_sleeptimer_date_t datetest;

uint8_t button_press_timerCounter = 0;
uint8_t button_press_counter = 0;
uint8_t help = 0;

uint8_t unReadCheck;
uint8_t dataCounter;
uint8_t dataPointer = 0;
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
  sl_app_log("Initiation.... \n");

  // LED & Buzzer init
    led_buzzer_init ();
    sl_app_log(" LED & buzzer init Ok \n");

  set_LED('w');

  // Chip init
  CHIP_Init ();
  sl_app_log(" Chip init Ok \n");

  // I2C init
  i2c_init ();
  sl_app_log(" I2C init Ok \n");

  // Max30102 init
  MAX30102_init ();
  sl_app_log(" MAX30102 init Ok \n");

  // MSC init
  //msc_init ();
  sl_app_log(" MSC init Ok \n");

  // GPIO Interrupt init
  gpio_INTR_init();
  sl_app_log(" GPIO Intr init Ok \n");

  // MPU6050init
  MPU6050_ConfigDMP(&mpu, &devStatus, &dmpReady, &mpuIntStatus, &packetSize);
//  sl_bt_system_set_soft_timer(TIMER_MS(300*1000), MIIN, 0);

  // MSC init
  MSC_Init();
  MSC_CheckUnRead(&dataCounter, dataCounter);

   set_Buzzer();
   sl_sleeptimer_delay_millisecond(600);
   clear_Buzzer();
   clear_all_LED();
}

/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
SL_WEAK void app_process_action (void)
{

  /*********************** Duong's Process **********************************/
//  MPU6050_GetData(&mpu, &dmpReady, &mpuInterrupt, &packetSize, &mpuIntStatus,&check_fall);
//  if(check_fall == 1)
//    {
//      sl_bt_system_set_soft_timer(TIMER_MS(5000), SEC, 1);
//    }

}


void process_server_user_write_request(sl_bt_msg_t *evt)
{
  sl_status_t sc;
  uint32_t connection =
      evt->data.evt_gatt_server_user_write_request.connection;
  uint32_t characteristic =
      evt->data.evt_gatt_server_user_write_request.characteristic;
  sc = sl_bt_gatt_server_send_user_write_response(connection,characteristic,0);

  if (characteristic == gattdb_device_ch)
    {

      uint8_t header = evt->data.evt_gatt_server_attribute_value.value.data[0];
      uint8_t header1 = evt->data.evt_gatt_server_attribute_value.value.data[1];
      uint8_t header2 = evt->data.evt_gatt_server_attribute_value.value.data[2];
      uint8_t len = evt->data.evt_gatt_server_attribute_value.value.len;

      app_log("header: %d %d %d - len: %d \n",header,header1,header2, len);

      if (header == 1 && len ==  7 ){ //set date time
	  datetest.time_zone = 0;
	  datetest.year = evt->data.evt_gatt_server_attribute_value.value.data[3];
	  datetest.month = evt->data.evt_gatt_server_attribute_value.value.data[2];
	  datetest.month_day = evt->data.evt_gatt_server_attribute_value.value.data[1];
	  datetest.hour = evt->data.evt_gatt_server_attribute_value.value.data[4];
	  datetest.min = evt->data.evt_gatt_server_attribute_value.value.data[5];
	  datetest.sec = evt->data.evt_gatt_server_attribute_value.value.data[6];
	  sl_sleeptimer_set_datetime(&datetest);
  	        uint8_t read[27] ={0,22,4,122,21,44,87,98,26,0,22,4,122,21,30,80,96,22,0,22,4,122,21,20,81,98,22};
  	        uint8_t len1 = sizeof(read)/sizeof(uint8_t);
//  	        for(i = 4; i < 12; i++)
//  	        {
//  	            MSC_read(&read[i][0], i);
//  	        }
  	      send_all_old_data(&notifyEnabled, &app_connection, read, &len1);
  	      app_log("%d\n",len1);
  	        app_log("send success\n");
      }
      	  else if(header == 2 && len == 1 )
      	    {
      	      send_data(&notifyEnabled, &app_connection, &te, 1);
      	    }
      	  else if(header == 3 && len ==1 )
      	    {
      	      send_data(&notifyEnabled, &app_connection, &spo2, 2);
      	    }
      	  else if(header == 4 && len ==1 )
      	    {
      	      send_data(&notifyEnabled, &app_connection, &bmp, 3);
      	    }
      	  else if(header == 5 && len ==1)
      	    {
      		   sl_app_log(" Gui data manual - nut nhan app \n");
      		   set_LED('w');
      		   float T;
      		   uint8_t i;
      		   for(i = 1; i < 4; i++)
      		   {
      			   T = LM75_ReadTemperature();
      			   BPM_SpO2_Update(&bpm_spo2_value, i);
					float a1 = (float) (bpm_spo2_value.BPM);
					float a2 = (float) (bpm_spo2_value.SpO2);
					sl_app_log(" Nhiet do: %d \n", (uint32_t ) (1000 * T));
					sl_app_log(" BPM: %d \n", bpm_spo2_value.BPM);
					sl_app_log(" Spo2: %d \n", bpm_spo2_value.SpO2);
					send_all_data (&notifyEnabled, &app_connection, &T, &a2, &a1);
      		   }
      		   clear_all_LED();
      	    }
      	  else if(header == 6 && len == 1)
      	    {
	        uint8_t read[2][9] ={{8,2,34,6,3,5,87,14,26},{9,21,124,65,32,115,27,124,216}};
//  	        for(i = 4; i < 12; i++)
//  	        {
//  	            MSC_read(&read[i][0], i);
//  	        }
	        uint8_t len =2;
	        send_all_old_data(&notifyEnabled, &app_connection, read, len);
	        app_log("send success\n");
      	    }
      	  else if(header == 7 && len ==1)
      	    {
      	uint32_t diff = diff_time(&date_disconnect);
      	app_log("time unix : %d\n",diff);
      	    }
    }
}
  /**************************************************************************//**
   * Bluetooth stack event handler.
   * This overrides the dummy weak implementation.
   *
   * @param[in] evt Event coming from the Bluetooth stack.
   *****************************************************************************/
  void sl_bt_on_event(sl_bt_msg_t *evt)
  {
    sl_status_t sc;
    bd_addr address;
    uint8_t address_type;
    uint8_t system_id[8];

    switch (SL_BT_MSG_ID(evt->header)) {
      // -------------------------------
      // This event indicates the device has started and the radio is ready.
      // Do not call any stack command before receiving this boot event!
      case sl_bt_evt_system_boot_id:

	// Extract unique ID from BT Address.
	sc = sl_bt_system_get_identity_address(&address, &address_type);
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

	sc = sl_bt_gatt_server_write_attribute_value(gattdb_system_id,
						     0,
						     sizeof(system_id),
						     system_id);
	app_assert_status(sc);

	// Create an advertising set.
	sc = sl_bt_advertiser_create_set(&advertising_set_handle);
	app_assert_status(sc);

	// Set advertising interval to 100ms.
	sc = sl_bt_advertiser_set_timing(
	    advertising_set_handle,
	    160, // min. adv. interval (milliseconds * 1.6)
	    160, // max. adv. interval (milliseconds * 1.6)
	    0,   // adv. duration
	    0);  // max. num. adv. events
	app_assert_status(sc);
	// Start general advertising and enable connections.
	sc = sl_bt_advertiser_start(
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
//	uint32_t diff_t = diff_time(&date_disconnect);
//	if(date_disconnect.year != 1900 && diff_t > 600)
//	  {
//	    // todo
//	  }
	break;

	// -------------------------------
	// This event indicates that a connection was closed.
      case sl_bt_evt_connection_closed_id:
	// Restart advertising after client has disconnected.
	sc = sl_bt_advertiser_start(
	    advertising_set_handle,
	    sl_bt_advertiser_general_discoverable,
	    sl_bt_advertiser_connectable_scannable);
	sl_app_log("connection closed \n");
	sl_sleeptimer_get_datetime(&date_disconnect);
	app_assert_status(sc);
	app_connection = 0;
	break;

	///////////////////////////////////////////////////////////////////////////
	// Add additional event handlers here as your application requires!      //
	///////////////////////////////////////////////////////////////////////////

      case sl_bt_evt_gatt_server_characteristic_status_id :
	if(evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_data_ch)
	  {
	    if(evt->data.evt_gatt_server_characteristic_status.status_flags == 0x01)
	      {
		notifyEnabled = true;
		app_log("enable notifyEnabled \n");
	      }

	    else if(evt->data.evt_gatt_server_characteristic_status.status_flags == 0x00)
	      {
		notifyEnabled = false;
		app_log("disable \n");
	      }
	  }

	break;
	//    case sl_bt_evt_gatt_server_attribute_value_id:
	//      if (evt->data.evt_gatt_server_characteristic_status.characteristic
	//          == gattdb_device_ch)
	//        {
	//          uint8_t header = evt->data.evt_gatt_server_attribute_value.value.data[0];
	//          if (header == 1){ //set date time
	//                datetest.time_zone = 7;
	//                datetest.year = evt->data.evt_gatt_server_attribute_value.value.data[3];
	//                datetest.month = evt->data.evt_gatt_server_attribute_value.value.data[2];
	//                datetest.month_day = evt->data.evt_gatt_server_attribute_value.value.data[1];
	//                datetest.hour = evt->data.evt_gatt_server_attribute_value.value.data[4];
	//                datetest.min = evt->data.evt_gatt_server_attribute_value.value.data[5];
	//                datetest.sec = evt->data.evt_gatt_server_attribute_value.value.data[6];
	//                sl_sleeptimer_set_datetime(&datetest);
	//          }
	//
	//        }
	//      break;
      case sl_bt_evt_gatt_server_user_write_request_id:
	process_server_user_write_request(evt);


	break;
		case sl_bt_evt_system_soft_timer_id:
			if (evt->data.evt_system_soft_timer.handle == SEC)
			{
				if (check_fall == 0)
				{
					set_LED ('r');
				}
				// get data, check connect_condition => push memory
			}
			if (evt->data.evt_system_soft_timer.handle == MIIN)
			{
				sl_app_log(" Gui data dinh ky \n");
				set_LED('w');
				float T = LM75_ReadTemperature ();
				BPM_SpO2_Update (&bpm_spo2_value, 1);
				float a1 = (float) (bpm_spo2_value.BPM);
				float a2 = (float) (bpm_spo2_value.SpO2);
				sl_app_log(" Nhiet do: %d \n", (uint32_t) (1000*T));
				sl_app_log(" BPM: %d \n", bpm_spo2_value.BPM);
				sl_app_log(" Spo2: %d \n", bpm_spo2_value.SpO2);
				send_all_data (&notifyEnabled, &app_connection, &T, &a2, &a1);
				if(app_connection == 0)
				{
					// ghi vào memory
				}
				clear_all_LED();
			}
			if(evt->data.evt_system_soft_timer.handle == BUTTON_PRESS_TIMER)
			{
				button_press_timerCounter += 1;
				sl_app_log(" button_press_timerCounter: %d \n", button_press_timerCounter);
				if(!GPIO_PinInGet (button_port, button_pin))
				{
					if(button_press_timerCounter >= LONG_PRESS_TIMEOUT)
					{
						sl_bt_system_set_soft_timer (TIMER_MS(0), BUTTON_PRESS_TIMER, 1);
						button_press_timerCounter = 0;
						if(help == 0)
						{
							help = 1;
							set_LED ('r');
//							set_Buzzer ();
						}
						else if(help == 1)
						{
							help = 0;
							clear_all_LED();
							clear_Buzzer();
						}
						button_press_counter = 0;
					}
				}
				else
				{
					sl_bt_system_set_soft_timer (TIMER_MS(0), BUTTON_PRESS_TIMER, 1);
					if(button_press_timerCounter > DEBOUND_TIMEOUT && button_press_timerCounter < SINGLE_PRESS_TIMEOUT)
					{
						sl_app_log(" Gui data manual - nut nhan mach \n");
						set_LED('w');
						float T;
						uint8_t i;
						for (i = 1; i < 4; i++)
						{
							T = LM75_ReadTemperature ();
							BPM_SpO2_Update (&bpm_spo2_value, i);
							float a1 = (float) (bpm_spo2_value.BPM);
							float a2 = (float) (bpm_spo2_value.SpO2);
							sl_app_log(" Nhiet do: %d \n",
									   (uint32_t ) (1000 * T));
							sl_app_log(" BPM: %d \n", bpm_spo2_value.BPM);
							sl_app_log(" Spo2: %d \n", bpm_spo2_value.SpO2);
							send_all_data (&notifyEnabled, &app_connection, &T,
										   &a2, &a1);
						}
						clear_all_LED();
					}
					button_press_timerCounter = 0;
					button_press_counter = 0;
				}
			}
			break;
		case sl_bt_evt_system_external_signal_id:
			if(evt->data.evt_system_external_signal.extsignals == button_pin)
			{
				if(button_press_counter == 0)
				{
					sl_bt_system_set_soft_timer (TIMER_MS(100), BUTTON_PRESS_TIMER, 0);
					button_press_counter += 1;
				}
			}
			if(evt->data.evt_system_external_signal.extsignals == mpu_intr_pin)
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
