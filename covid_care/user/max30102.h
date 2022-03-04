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
#define REG_MULTI_LED_CTRL1 0x11
#define REG_MULTI_LED_CTRL2 0x12
#define REG_TEMP_INTR 0x1F
#define REG_TEMP_FRAC 0x20
#define REG_TEMP_CONFIG 0x21
#define REG_PROX_INT_THRESH 0x30
#define REG_REV_ID 0xFE
#define REG_PART_ID 0xFF

// 0x00~0xFF = 0mA~51mA
#define MAX_CURRENT  0xFF
#define FIX_CURRENT  0x64  // ~8mA for LED1 & LED2

#define  MAX30102_RESET           0x04
#define  MAX30102_MODE_HR_ONLY    0b02
#define  MAX30102_MODE_SPO2_HR    0x03
#define  MAX30102_MODE_MULTI_LED  0x07

// Sample Averaging
#define smp1   0b000
#define smp2   0b001
#define smp4   0b010
#define smp8   0b011
#define smp16  0b100
#define smp32  0b101

// ADC Range Control
#define  adc2048   0b00
#define  adc4096   0b01
#define  adc8192   0b10
#define  adc16384  0b11

// Sample Rate
#define   sr50    0b000  // 50 samples per second
#define   sr100   0b001  // 100 samples per second
#define   sr200   0b010  // 200 samples per second
#define   sr400   0b011  // 400 samples per second
#define   sr800   0b100  // 800 samples per second
#define   sr1000  0b101  // 1000 samples per second
#define   sr1600  0b110  // 1600 samples per second
#define   sr3200  0b111  // 3200 samples per second

// pulseWidth
// This is the same for both LEDs
#define  pw69   0x00    // 69us pulse
#define  pw118  0b01    // 118us pulse
#define  pw215  0b10    // 215us pulse
#define  pw411  0b11  // 411us pulse

#define I2C_BUFFER_LENGTH 32


typedef struct max30102_t
{
    uint8_t BPM;
    uint8_t SpO2;
} max30102_t;

#define MEAN_FILTER_SIZE        15
typedef struct{
   int32_t value[2];
}Filter_Data;

#define MAGIC_ACCEPTABLE_INTENSITY_DIFF         65000
#define RED_LED_CURRENT_ADJUSTMENT_NS           1000000

/* Pulse detection parameters */
#define PULSE_MIN_THRESHOLD         100 //300 is good for finger, but for wrist you need like 20, and there is shitloads of noise
#define PULSE_MAX_THRESHOLD         2000
#define PULSE_GO_DOWN_THRESHOLD     1
#define PULSE_BPM_SAMPLE_SIZE       10 //Moving average size
/* SaO2 parameters */
#define RESET_SPO2_EVERY_N_PULSES     100
typedef enum PulseStateMachine{
   PULSE_IDLE,
   PULSE_TRACE_UP,
   PULSE_TRACE_DOWN
}PulseStateMachine;

extern int32_t IRcw;
extern int32_t REDcw;
extern uint32_t raw_IR;
extern uint32_t raw_RED;

extern uint8_t redLEDCurrent;

void MAX30102_init();
void MAX30102_ReadFIFO();
void MAX30102_Read();
void MAX30102_Shutdown(bool mode);
int32_t dcRemove(int32_t value,int32_t *cw);
int32_t MeanDiff(int32_t M);
int32_t LowPassButterworthFilter(int32_t value,Filter_Data *filter_data);
bool detectPulse(uint32_t sensor_value);
