/*
 * funtion.c
 *
 *  Created on: Apr 8, 2022
 *      Author: ADMIN
 */
#include "em_common.h"
#include "app_assert.h"
#include "sl_bluetooth.h"
#include "gatt_db.h"
#include "app.h"
#include "sl_app_log.h"
#include "led_buzzer.h"
#include "em_chip.h"
#include "i2c_lib.h"
#include "lm75.h"
#include "max30102.h"
#include "mpu6050.h"
#include "em_rtcc.h"
#include "gpio_intr.h"
#include "time.h"
#include "test_variable.h"
#include "math.h"
void convert_data(uint8_t arr[],float *data)
{
  int16_t n;
  n=(*data)*1000;
  arr[0] = n/1000;
  arr[1] = (n-arr[0]*1000)/125;

}
void
send_notify (uint8_t *notifyEnabled,uint8_t *app_connection)
{
  sl_status_t sc;
  uint8_t buffer[2];
  buffer[1] = 10;
  buffer[0] = 5;
  uint16_t len = 2;
  if (*notifyEnabled)
    {
      sc = sl_bt_gatt_server_send_notification (*app_connection, gattdb_temp_ch,
                                                len, buffer);
    }
  if (sc == SL_STATUS_OK)
    {
      app_log("send ok\n");
    }
  else
    app_log("send erorr\n");

}
void
clear_data (void)
{
  sl_status_t sc;
  uint8_t buffer[2];
  for(int i=0;i<2;i++)
    {
      buffer[i] = 0;
    }
  sc = sl_bt_gatt_server_write_attribute_value(gattdb_temp_ch, 0, sizeof(buffer),
                                              buffer);
  if (sc == SL_STATUS_OK)
    {
      app_log("clear OK\n");
    }

}
