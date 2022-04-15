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
#include "float.h"

uint8_t notifyEnabled = false;
uint8_t indicateEnabled = false;
uint8_t app_connection;

// The advertising set handle allocated from Bluetooth stack.
static uint8_t advertising_set_handle = 0xff;

/*----------------- Define Area -----------------*/
#define TIMER_MS(ms) ((32768 * ms) / 1000)
#define TEMPERATURE 0
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

BPM_SpO2_value_t BPM_SpO2_value;
uint8_t msc_dataCount = 0;

uint8_t unReadCounter = 0;
uint8_t dataCounter = 0;

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

	// Chip init
	CHIP_Init ();
	sl_app_log(" Chip init Ok \n");

	// I2C init
	i2c_init ();
	sl_app_log(" I2C init Ok \n");

	// Max30102 init
	MAX30102_init ();
	sl_app_log(" MAX30102 init Ok \n");

	// LED & Buzzer init
	led_buzzer_init ();
	sl_app_log(" LED & buzzer init Ok \n");

	// MSC init
	MSC_init ();
//	MSC_ErasePage(USERDATA);
	MSC_CheckUnRead(&unReadCounter, &dataCounter);
	sl_app_log(" %d %d \n", unReadCounter, dataCounter);
	sl_app_log(" MSC init Ok \n");

	// GPIO Interrupt init
	//    gpio_INTR_init();
	sl_app_log(" GPIO Intr init Ok \n");

	// MPU6050init
//	MPU6050_ConfigDMP(&mpu, &devStatus, &dmpReady, &mpuIntStatus, &packetSize);

	//RTCC init

	sl_app_log("Ok....... \n");

//	sl_bt_system_set_soft_timer (TIMER_MS(12000), 0, 0);
}

/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
SL_WEAK void app_process_action (void)
{
	/////////////////////////////////////////////////////////////////////////////
	// Put your additional application code here!                              //
	// This is called infinitely.                                              //
	// Do not call blocking functions from here!                               //
	/////////////////////////////////////////////////////////////////////////////
	/*********************** Khanh's Process **********************************/
	BPM_SpO2_Update(&BPM_SpO2_value, 1);


	if (!GPIO_PinInGet (button_port, button_pin))
	{

		/************************** LM75 test *****************************/
//		sl_app_log("---------------- \n");
//		float T = LM75_ReadTemperature ();
//		sl_app_log(" %d \n", (uint32_t) (1000*T) );
//		uint8_t t_d = LM75_FloatToOneByte (T);
//		sl_app_log(" %d %d \n", (t_d >> 3)+20, (t_d & 0x07));
//		float t_f = LM75_OneByteToFloat(t_d);
//		sl_app_log(" %d \n", (uint32_t) (1000*t_f) );

		/************************ MAX30102 test **************************/
//		BPM_SpO2_Update(&BPM_SpO2_value, 3);

		/************************** MSC test *****************************/

		/** Ghi vào flash vào vị trí tiếp theo */
		float T = 29.125;
		uint8_t t_d = LM75_FloatToOneByte (T);  // 73

		//	vị trí word mà từ đó các dữ liệu chưa dc đọc, ngày, tháng, năm, giờ, phút, BPM, SpO2, nhiet do
		uint8_t data[9] = {28, 31, 12, 22, 23, 59, 88, 98, t_d};
		uint8_t read[9];
		MSC_write(data, &dataCounter);
		sl_app_log(" Write OK \n");
		MSC_read(read, dataCounter-1);
//		MSC_CheckPage();

		/** Doc cac cac data trong flash mà chưa được gui len app */
//		MSC_CheckPage();
//		sl_app_log("\n------------------- \n");
//		uint8_t read[9];
//		uint8_t i;
//		for(i = unReadCounter; i < dataCounter; i++)
//		{
//			sl_app_log("%d \n", i);
//			MSC_read(read, i);
//		}
	}

	/*********************** Duong's Process **********************************/
//  MPU6050_GetData(&mpu, &dmpReady, &mpuInterrupt, &packetSize, &mpuIntStatus);
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
      break;

    // -------------------------------
    // This event indicates that a connection was closed.
    case sl_bt_evt_connection_closed_id:
      // Restart advertising after client has disconnected.
      sc = sl_bt_advertiser_start(
        advertising_set_handle,
        sl_bt_advertiser_general_discoverable,
        sl_bt_advertiser_connectable_scannable);
      app_assert_status(sc);
      break;

    ///////////////////////////////////////////////////////////////////////////
    // Add additional event handlers here as your application requires!      //
    ///////////////////////////////////////////////////////////////////////////

    case sl_bt_evt_gatt_server_characteristic_status_id :
      if(evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_temp_ch)
        {
          if(evt->data.evt_gatt_server_characteristic_status.status_flags == 0x01)
            {
              notifyEnabled = true;
              app_log("enable notifyEnabled \n");
            }
          else if(evt->data.evt_gatt_server_characteristic_status.status_flags == 0x02)
            {
              indicateEnabled = true;
              app_log("enable indicateEnabled \n");
            }
          else if(evt->data.evt_gatt_server_characteristic_status.status_flags == 0x00)
            {
              notifyEnabled = false;
              indicateEnabled = false;
              app_log("disable \n");
            }
        }

      break;
    case sl_bt_evt_gatt_server_attribute_value_id:
      if (evt->data.evt_gatt_server_characteristic_status.characteristic
          == gattdb_temp_ch)
        {
          uint8_t data = evt->data.evt_gatt_server_attribute_value.value.data[0];
          app_log("data :%d\n", data);
          if (data == 1)
            {
              clear_data ();
            }
        }
      break;
//    case sl_bt_evt_system_soft_timer_id:
//      if (evt->data.evt_system_soft_timer.handle == TEMPERATURE)
//        {
//          send_notify ();
//        }
//      break;
    case sl_bt_evt_system_external_signal_id:
      dmpDataReady();
      break;
    // -------------------------------
    // Default event handler.
    default:
      break;
  }
}
