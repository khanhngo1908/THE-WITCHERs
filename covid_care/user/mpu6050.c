/*
 * mpu6050.c
 *
 *  Created on: 11 thg 3, 2022
 *      Author: Admin
 */

#include "MPU6050.h"
#include "i2c_lib.h"
#include "sl_app_log.h"
void MPU6050_Init(void)
{
  i2c_write(MPU6050_ADD, SMPLRT_DIV, 0x07);//set sample rate to 8000/(1+7) = 1000Hz
  i2c_write(MPU6050_ADD, PWR_MGMT_1, 0x00);// wake up MPU6050
  //i2c_write(MPU6050_ADD,CONFIG,0x00);//disable DLPF
  i2c_write(MPU6050_ADD,ACCEL_CONFIG,0x00);//full scale range mode 0 +- 2g
  i2c_write(MPU6050_ADD,GYRO_CONFIG,0x00);//full scale range mode 0 +- 250do/s
  //i2c_write(MPU6050_ADD,0x74,0x06);//disable sensor output to FIFO buffer
}
void MPU6050_Read(void)
{
  int16_t Ax,Ay,Az;
  int16_t Gx,Gy,Gz;
  // Prepare For Reading, Starting From ACCEL_XOUT_H
  uint8_t data_ax_h,data_ax_l,data_ay_h,data_ay_l,data_az_h,data_az_l;
  uint8_t data_gx_h,data_gx_l,data_gy_h,data_gy_l,data_gz_h,data_gz_l;
  //read data accelerometer
  i2c_read(MPU6050_ADD, ACCEL_XOUT_H, &data_ax_h, 1);
  i2c_read(MPU6050_ADD, ACCEL_XOUT_L, &data_ax_l, 1);
  Ax = (int16_t)(data_ax_h << 8 | data_ax_l);
  i2c_read(MPU6050_ADD, ACCEL_YOUT_H, &data_ay_h, 1);
  i2c_read(MPU6050_ADD, ACCEL_YOUT_L, &data_ay_l, 1);
  Ay = (int16_t)(data_ay_h << 8 | data_ay_l);
  i2c_read(MPU6050_ADD, ACCEL_ZOUT_H, &data_az_h, 1);
  i2c_read(MPU6050_ADD, ACCEL_ZOUT_L, &data_az_l, 1);
  Az = (int16_t)(data_az_h << 8 | data_az_l);
  // read data gyroscope
  i2c_read(MPU6050_ADD, GYRO_XOUT_H, &data_gx_h, 1);
  i2c_read(MPU6050_ADD, GYRO_XOUT_L, &data_gx_l, 1);
  Gx = (int16_t)(data_gx_h << 8 | data_gx_l);
  i2c_read(MPU6050_ADD, GYRO_YOUT_H, &data_gy_h, 1);
  i2c_read(MPU6050_ADD, GYRO_YOUT_L, &data_gy_l, 1);
  Gy = (int16_t)(data_gy_h << 8 | data_gy_l);
  i2c_read(MPU6050_ADD, GYRO_ZOUT_H, &data_gz_h, 1);
  i2c_read(MPU6050_ADD, GYRO_ZOUT_L, &data_gz_l, 1);
  Gz = (int16_t)(data_gz_h << 8 | data_gz_l);
  //log data accelerometer
  app_log("Ax :%d", Ax);
  app_log(" | Ay :%d", Ay);
  app_log(" | Az :%d", Az);
  //log data accelerometer
  app_log(" | Gx :%d", Gx);
  app_log(" | Gy :%d", Gy);
  app_log(" | Gz :%d", Gz);
  app_log("\n");
}
void CheckID(void)
{
    uint8_t data;
    i2c_read(0x68, 0x75, &data, 1);
    app_log("check : %d\n",data);
  }

