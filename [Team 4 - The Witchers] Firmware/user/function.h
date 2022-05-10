/*
 * function.h
 *
 *  Created on: Apr 8, 2022
 *      Author: ADMIN
 */

#ifndef USER_FUNCTION_H_
#define USER_FUNCTION_H_
void
send_data(uint8_t *notifyEnabled,uint8_t *app_connection,float *data,uint8_t type);
void clear_data (void);
void convert_data(uint8_t arr[],float *data);
void send_all_data(uint8_t *notifyEnabled,uint8_t *app_connection,float *temperature, float *spo2, float *bmp);
void send_check(uint8_t *notifyEnabled,uint8_t *app_connection);
uint32_t diff_time(sl_sleeptimer_date_t *date_disconnect);
void send_all_old_data(uint8_t *notifyEnabled,uint8_t *app_connection,uint8_t arr[], uint8_t *len);
#endif /* USER_FUNCTION_H_ */

