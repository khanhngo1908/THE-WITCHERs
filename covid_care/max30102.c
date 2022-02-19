#include <max30102.h>
#include "i2c_lib.h"

uint8_t data;

void MAX30102_init()
{
     data = MAX30102_MODE_SPO2_HR;
     i2c_write(MAX30102_ADDRESS,REG_MODE_CONFIG, &data, 1);
     i2c_read(MAX30102_ADDRESS,REG_MODE_CONFIG, &data, 1);
//     i2c_write(MAX30102_ADDRESS,REG_LED1_PA, 0x7, 1);
//     i2c_write(MAX30102_ADDRESS,REG_LED2_PA, FIX_IR_CURRENT, 1);
//     i2c_write(MAX30102_ADDRESS,REG_PILOT_PA, 0x7f, 1);
//     i2c_write(MAX30102_ADDRESS,REG_SPO2_CONFIG,(sr100<<2) | pw411, 1);
//     /*  I2C_Write(I2C1,MAX30102_ADDRESS,REG_INT_EN_1,0xC0);
//         Delay(5000);
//         I2C_Write(I2C1,MAX30102_ADDRESS,REG_INT_EN_2,0x00);
//         Delay(5000);*/
//     i2c_write(I2C1,MAX30102_ADDRESS,REG_FIFO_WR_PTR,0x00);
//     i2c_write(I2C1,MAX30102_ADDRESS,REG_OVF_COUNTER,0x00);
//     i2c_write(I2C1,MAX30102_ADDRESS,REG_FIFO_RD_PTR,0x00);
}
