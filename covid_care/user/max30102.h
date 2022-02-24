
#include "stdint.h"

#define MAX30102_ADDRESS 0xAE

#define REG_INTR_STATUS_1 0x00
#define REG_INTR_STATUS_2 0x01
#define REG_INTR_ENABLE_1 0x02
#define REG_INTR_ENABLE_2 0x03
#define REG_FIFO_WR_PTR 0x04
#define REG_OVF_COUNTER 0x05
#define REG_FIFO_RD_PTR 0x06
#define REG_FIFO_DATA 0x07
#define REG_FIFO_CONFIG 0x08
#define REG_MODE_CONFIG 0x09
#define REG_SPO2_CONFIG 0x0A
#define REG_LED1_PA 0x0C
#define REG_LED2_PA 0x0D
#define REG_PILOT_PA 0x10
#define REG_MULTI_LED_CTRL1 0x11
#define REG_MULTI_LED_CTRL2 0x12
#define REG_TEMP_INTR 0x1F
#define REG_TEMP_FRAC 0x20
#define REG_TEMP_CONFIG 0x21
#define REG_PROX_INT_THRESH 0x30
#define REG_REV_ID 0xFE
#define REG_PART_ID 0xFF

// pulseWidth
// This is the same for both LEDs
#define  pw69   0x00    // 69us pulse
#define  pw118  0x01    // 118us pulse
#define  pw215  0x10    // 215us pulse
#define  pw411  0x11  // 411us pulse

// 0x00~0xFF = 0mA~51mA
#define MAX_CURRENT  0xFF
#define FIX_CURRENT  0x64  // ~20mA for LED1 & LED2
#define PA_CURRENT   0x7F  // ~25mA for Pilot LED

#define  MAX30102_RESET           0x04
#define  MAX30102_MODE_HR_ONLY    0x02
#define  MAX30102_MODE_SPO2_HR    0x03
#define  MAX30102_MODE_MULTI_LED  0x07

// Sample Rate
#define   sr50    0x000  // 50 samples per second
#define   sr100   0x001  // 100 samples per second
#define   sr167   0x010  // 167 samples per second
#define   sr200   0x011  // 200 samples per second
#define   sr400   0x100  // 400 samples per second
#define   sr600   0x101  // 600 samples per second
#define   sr800   0x110  // 800 samples per second
#define   sr1000  0x111  // 1000 samples per second

typedef struct max30102_t
{
    uint32_t _ir_samples[32];
    uint32_t _red_samples[32];
} max30102_t;

void MAX30102_init();
void MAX30102_ReadFIFO(max30102_t *obj);
void MAX30102_Shutdown(bool mode);
void dcRemoval();
