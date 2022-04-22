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
#include "lm75.h"
void convert_data(uint8_t arr[],float *data)
{
  arr[0] = (uint8_t) (*data);
  arr[1] = ((*data)-arr[0])/0.125;

}

void
send_data(uint8_t *notifyEnabled,uint8_t *app_connection,float *data,uint8_t type)
{
  sl_status_t sc;
  uint8_t buffer[8];
  uint8_t arr[1];
  uint16_t len = 9;
  sl_sleeptimer_date_t date_time;
  sl_sleeptimer_get_datetime(&date_time);
  switch(type)
  {
    case 1:

      convert_data(arr, data);
      buffer[2] = arr[1];
      buffer[1] = arr[0];
      buffer[0] = 1;
      break;
    case 2:
      buffer[2] = 0;
      buffer[1] = (uint8_t) (*data);
      buffer[0] = 2;
      break;
    case 3:
      buffer[2] = 0;
      buffer[1] = (uint8_t) (*data);
      buffer[0] = 3;
      break;
  }

  buffer[3] =date_time.month_day;
  buffer[4] =date_time.month;
  buffer[5] =date_time.year;
  buffer[6] =date_time.hour;
  buffer[7] =date_time.min;
  buffer[8] =date_time.sec;
  if (*notifyEnabled)
    {
      sc = sl_bt_gatt_server_send_notification (*app_connection, gattdb_data_ch,
                                                len, buffer);
    }
  if (sc == SL_STATUS_OK)
    {
      app_log("send ok\n");
    }
  else
    app_log("send erorr\n");

}
void send_check(uint8_t *notifyEnabled,uint8_t *app_connection)
{
 sl_status_t sc;
 app_log("check %d\n",*notifyEnabled);
 uint16_t len =8;
 uint8_t buffer[7];
 buffer[0]= 9;
 buffer[1]= 23;
 buffer[2]= 45;
 buffer[3]= 19;
 buffer[4]= 122;
 buffer[5]= 97;
 buffer[6]= 12;
 buffer[7]= 239;
 if( *notifyEnabled == 1)
   {
     sc = sl_bt_gatt_server_send_notification(*app_connection, gattdb_data_ch, len, buffer);
   }
 if (sc == SL_STATUS_OK)
   {
     app_log("send ok ok ok ok\n");
   }
 else
   {
   app_log("send erorr\n");
   }
}
void send_all_data(uint8_t *notifyEnabled,uint8_t *app_connection,float *temperature, float *spo2, float *bmp)
{
   sl_status_t sc;
   uint8_t buffer[10];
   uint8_t arr[1];
   uint16_t len = 11;
   sl_sleeptimer_date_t date_time;
   sl_sleeptimer_get_datetime(&date_time);
   convert_data(arr, temperature);
   buffer[0] = 4;

   buffer[1] = arr[0];
   buffer[2] = arr[1];
   buffer[3] = (uint8_t) (*spo2);
   buffer[4] = (uint8_t) (*bmp);
   buffer[5] =date_time.month_day;
   buffer[5] =date_time.month;
   buffer[7] =date_time.year;
   buffer[8] =date_time.hour;
   buffer[9] =date_time.min;
   buffer[10] =date_time.sec;
   if (*notifyEnabled)
     {
       sc = sl_bt_gatt_server_send_notification (*app_connection, gattdb_data_ch,
                                                 len, buffer);
     }
   if (sc == SL_STATUS_OK)
     {
       app_log("send ok\n");
     }
   else
     {
     app_log("send erorr\n");
     }
}
void send_all_old_data(uint8_t *notifyEnabled,uint8_t *app_connection,uint8_t arr[][8], uint8_t len)

{
     sl_status_t sc;
     uint8_t buffer[1+(len)*10];
     uint8_t temp[1];
     buffer[0] = 6;
     buffer[1] = (len) ;
     uint16_t length = 2+(len)*10;
     uint8_t count = 0;
     for(int i=0;i<(len);i++)
       {
//	     float T = LM75_OneByteToFloat (arr[i][8]);
//	     convert_data(temp, &T);
	     buffer[count+2]=36;
	     buffer[count+3]=1;
	     buffer[count+4]=arr[i][7];
	     buffer[count+5]=arr[i][6];
	     buffer[count+6]=arr[i][1];
	     buffer[count+7]=arr[i][2];
	     buffer[count+8]=arr[i][3];
	     buffer[count+9]=arr[i][4];
	     buffer[count+10]=arr[i][5];
	     buffer[count+11]=0;
	     count+=10;
       }
     if (*notifyEnabled)
       {
         sc = sl_bt_gatt_server_send_notification (*app_connection, gattdb_data_ch,
                                                   length, buffer);
       }
     if (sc == SL_STATUS_OK)
       {
         app_log("send ok\n");
       }
     else
       {
       app_log("send erorr\n");
       }

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
  sc = sl_bt_gatt_server_write_attribute_value(gattdb_device_ch, 0, sizeof(buffer),
                                              buffer);
  if (sc == SL_STATUS_OK)
    {
      app_log("clear OK\n");
    }

}
uint32_t diff_time(sl_sleeptimer_date_t *date_disconnect)
{
	  sl_sleeptimer_date_t date_current;
	  sl_sleeptimer_timestamp_t timestamp;
	  sl_sleeptimer_timestamp_t timestamp1;
	  sl_sleeptimer_get_datetime(&date_current);
	  sl_sleeptimer_convert_date_to_time(&date_current, &timestamp);
	  sl_sleeptimer_convert_date_to_time(date_disconnect, &timestamp1);
	  return timestamp - timestamp1;
}
