/*
 * i2c_lib.h
 *
 *  Created on: 18 thg 2, 2022
 *      Author: Ngo Minh Khanh
 */

#include "em_i2c.h"
#include "em_cmu.h"

#define I2C_PERIPHERAL              I2C0
#define I2C_PERIPHERAL_NO           0

// I2C0 SCL on PD02
#define I2C_SCL_PORT                gpioPortD
#define I2C_SCL_PIN                 2

// I2C0 SDA on PD03
#define I2C_SDA_PORT               gpioPortD
#define I2C_SDA_PIN                3

//#define I2C_TXBUFFER_SIZE                 10
//#define I2C_RXBUFFER_SIZE                 10

#define I2C_INIT_FAST                                                 \
{                                                                        \
    true,                  /* Enable when initialization done. */          \
    true,                  /* Set to Controller mode. */                   \
    0,                     /* Use currently configured reference clock. */ \
    I2C_FREQ_FAST_MAX,     /* Set to fast max assuring being */       \
    /*                        within I2C specification. */                 \
    i2cClockHLRAsymetric    /* Set to use 6:3 low/high duty cycle. */       \
}


void i2c_init(void);
void i2c_read(uint16_t followerAddress, uint8_t targetAddress, uint8_t *rxBuff, uint8_t numBytes);
void i2c_write(uint16_t followerAddress, uint8_t targetAddress, uint8_t txBuff);

