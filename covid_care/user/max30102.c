#include <i2c_lib.h>
#include <max30102.h>
#include "sl_app_log.h"

//// pulseWidth
//const uint8_t  pw69  = 0x00;    // 69us pulse
//const uint8_t  pw118 = 0x01;    // 118us pulse
//const uint8_t  pw215 = 0x10;    // 215us pulse
//const uint8_t  pw411 = 0x11;  // 411us pulse
//
//// 0x00~0xFF = 0mA~51mA
//const uint8_t MAX_CURRENT = 0xFF;
//const uint8_t FIX_IR_CURRENT = 0x50;
//
//// Sample Rate
//const uint16_t  sr50   = 0x000;  // 50 samples per second
//const uint16_t  sr100  = 0x001;  // 100 samples per second
//const uint16_t  sr167  = 0x010;  // 167 samples per second
//const uint16_t  sr200  = 0x011;  // 200 samples per second
//const uint16_t  sr400  = 0x100;  // 400 samples per second
//const uint16_t  sr600  = 0x101;  // 600 samples per second
//const uint16_t  sr800  = 0x110;  // 800 samples per second
//const uint16_t  sr1000 = 0x111;  // 1000 samples per second
//
//// Mode Configtion
//const uint8_t  MAX30102_RESET          = 0x04;
//const uint8_t  MAX30102_MODE_HR_ONLY   = 0x02;
//const uint8_t  MAX30102_MODE_SPO2_HR   = 0x03;
//const uint8_t  MAX30102_MODE_MULTI_LED = 0x07;

uint8_t data = 0;

void MAX30102_init()
{
     i2c_write(MAX30102_ADDRESS,REG_MODE_CONFIG, MAX30102_RESET);
//     i2c_read(MAX30102_ADDRESS,REG_MODE_CONFIG, &data, 1);
//
     i2c_write(MAX30102_ADDRESS,REG_MODE_CONFIG, MAX30102_MODE_SPO2_HR);

     i2c_write(MAX30102_ADDRESS,REG_LED1_PA, FIX_CURRENT);
//     i2c_read(MAX30102_ADDRESS,REG_LED1_PA, &data, 1);
     i2c_write(MAX30102_ADDRESS,REG_LED2_PA, FIX_CURRENT);

     i2c_write(MAX30102_ADDRESS,REG_PILOT_PA, PA_CURRENT);

     i2c_write(MAX30102_ADDRESS,REG_SPO2_CONFIG,(sr100<<2) | pw411);
     /*I2C_Write(I2C1,MAX30102_ADDRESS,REG_INT_EN_1,0xC0);
         Delay(5000);
         I2C_Write(I2C1,MAX30102_ADDRESS,REG_INT_EN_2,0x00);
         Delay(5000);*/
     i2c_write(MAX30102_ADDRESS,REG_FIFO_WR_PTR,0x00);
     i2c_write(MAX30102_ADDRESS,REG_OVF_COUNTER,0x00);
     i2c_write(MAX30102_ADDRESS,REG_FIFO_RD_PTR,0x00);
}

void MAX30102_ReadFIFO(max30102_t *obj)
{
    uint8_t wr_ptr = 0, rd_ptr = 0;
    i2c_read(MAX30102_ADDRESS,REG_FIFO_WR_PTR, &wr_ptr, 1);
    i2c_read(MAX30102_ADDRESS,REG_FIFO_RD_PTR, &rd_ptr, 1);

    int8_t num_samples;
    num_samples = (int8_t)wr_ptr - (int8_t)rd_ptr;
    if (num_samples < 1)
    {
         num_samples += 32;
    }

    for (int8_t i = 0; i < num_samples; i++)
    {
        uint8_t sample[6];
        i2c_read(MAX30102_ADDRESS, REG_FIFO_DATA, sample, 6);
        uint32_t ir_sample = ((uint32_t)(sample[0] << 16) | (uint32_t)(sample[1] << 8) | (uint32_t)(sample[2])) & 0x3ffff;
        uint32_t red_sample = ((uint32_t)(sample[3] << 16) | (uint32_t)(sample[4] << 8) | (uint32_t)(sample[5])) & 0x3ffff;
        obj->_ir_samples[i] = ir_sample;
        obj->_red_samples[i] = red_sample;
        sl_app_log(" NUM: %d \t IR: %d \t RED: %d \n", num_samples, ir_sample, red_sample);
    }
}

void MAX30102_Shutdown(bool mode)
{
   uint8_t config;
   i2c_read(MAX30102_ADDRESS, REG_MODE_CONFIG, &config, 1);
   if(mode)
   {
       config = config | 0x80;
   }
   else {
     config = config | 0x7F;
   }
   i2c_write(MAX30102_ADDRESS,REG_MODE_CONFIG, config);
}
