#include <i2c_lib.h>
#include <max30102.h>
#include "sl_app_log.h"

uint16_t data = 0;

int32_t IRcw = 0;
int32_t REDcw = 0;

uint32_t raw_IR = 0;
uint32_t raw_RED = 0;

int32_t msum = 0;
int32_t mvalues[MEAN_FILTER_SIZE];
int32_t mindex = 0;
int32_t mcount = 0;

Filter_Data irf = {0};
Filter_Data redf = {0};

uint8_t currentPulseDetectorState = PULSE_IDLE;
uint32_t lastBeatThreshold = 0;
uint32_t currentBPM;
uint32_t valuesBPM[PULSE_BPM_SAMPLE_SIZE] = {0};
uint32_t valuesBPMSum = 0;
uint8_t valuesBPMCount = 0;
uint8_t bpmIndex = 0;

uint32_t micros = 0;

void MAX30102_init()
{
    i2c_write(MAX30102_ADDRESS, REG_MODE_CONFIG, MAX30102_RESET);

    i2c_write(MAX30102_ADDRESS, REG_MODE_CONFIG, MAX30102_MODE_SPO2_HR);

    i2c_write(MAX30102_ADDRESS, REG_FIFO_CONFIG, (smp4<<5) );

    i2c_write(MAX30102_ADDRESS, REG_LED1_PA, FIX_CURRENT);
    i2c_write(MAX30102_ADDRESS, REG_LED2_PA, FIX_CURRENT);

    i2c_write(MAX30102_ADDRESS, REG_SPO2_CONFIG, (adc16384<<5) | (sr1000<<2) | pw411);

    i2c_write(MAX30102_ADDRESS, REG_FIFO_WR_PTR, 0x00);
    i2c_write(MAX30102_ADDRESS, REG_OVF_COUNTER, 0x00);
    i2c_write(MAX30102_ADDRESS, REG_FIFO_RD_PTR, 0x00);
}

void MAX30102_ReadFIFO()
{
    uint8_t writePointer = 0, readPointer = 0;
    i2c_read(MAX30102_ADDRESS,REG_FIFO_WR_PTR, &writePointer, 1);
    i2c_read(MAX30102_ADDRESS,REG_FIFO_RD_PTR, &readPointer, 1);

//    if(writePointer != readPointer)
//    {
//        int numberOfSamples = writePointer - readPointer;
//        if (numberOfSamples < 0) numberOfSamples += 32;
//        int bytesLeftToRead = numberOfSamples * 2 * 3;
//        sl_app_log("%d\n", bytesLeftToRead);
//
//        while (bytesLeftToRead > 0)
//        {
//            int toGet = bytesLeftToRead;
//            if (toGet > I2C_BUFFER_LENGTH)
//                toGet = I2C_BUFFER_LENGTH - (I2C_BUFFER_LENGTH % (2 * 3));
//
//            bytesLeftToRead -= toGet;
//
//            while(toGet > 0)
//            {
//                uint8_t sample[6];
//                i2c_read(MAX30102_ADDRESS, REG_FIFO_DATA, sample, 6);
//                raw_RED = ((uint32_t)(sample[0] << 16) | (uint32_t)(sample[1] << 8) | (uint32_t)(sample[2])) & 0x3ffff;
//                raw_IR = ((uint32_t)(sample[3] << 16) | (uint32_t)(sample[4] << 8) | (uint32_t)(sample[5])) & 0x3ffff;
//                sl_app_log("%d\n", raw_IR);
//
//                toGet -= 2 * 3;
//            }
//        }

    int16_t num_samples;
    num_samples = (int8_t)writePointer - (int8_t)readPointer;
    if (num_samples < 1)
    {
        num_samples += 32;
    }
    for (int8_t i = 0; i < num_samples; i++)
    {
        uint8_t sample[6];
        i2c_read(MAX30102_ADDRESS, REG_FIFO_DATA, sample, 6);
        raw_RED = ((uint32_t)(sample[0] << 16) | (uint32_t)(sample[1] << 8) | (uint32_t)(sample[2])) & 0x3ffff;
        raw_IR = ((uint32_t)(sample[3] << 16) | (uint32_t)(sample[4] << 8) | (uint32_t)(sample[5])) & 0x3ffff;
        sl_app_log("%d\n", raw_IR);
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

void MAX30102_Read()
{
   MAX30102_ReadFIFO();
   int32_t IR_ac = dcRemove(raw_IR,&IRcw);
   IR_ac = MeanDiff(IR_ac);
   IR_ac = LowPassButterworthFilter(IR_ac,&irf);
   sl_app_log("%u\n", IR_ac);
//   detectPulse(IR_ac);
}

int32_t dcRemove(int32_t value,int32_t *cw)
{
   int32_t oldcw = *cw;
   *cw = value + 0.94 * *cw;
   return *cw - oldcw;
}

int32_t MeanDiff(int32_t M)
{
   int32_t avg = 0;

   msum -= mvalues[mindex];
   mvalues[mindex] = M;
   msum += mvalues[mindex];

   mindex++;
   mindex = mindex % MEAN_FILTER_SIZE;

   if(mcount < MEAN_FILTER_SIZE){
      mcount++;
   }

   avg = msum / mcount;
   return avg - M;
}

int32_t LowPassButterworthFilter(int32_t value,Filter_Data *filter_data)
{
   filter_data->value[0] = filter_data->value[1];
   //Fs = 100Hz and Fc = 10Hz
   filter_data->value[1] = (2.452372752527856026e-1 * value) + (0.50952544949442879485 * filter_data->value[0]);

   //Fs = 100Hz and Fc = 4Hz
   //filter_data->value[1] = (1.367287359973195227e-1 * value) + (0.72654252800536101020 * filter_data->value[0]); //Very precise butterworth filter

   return filter_data->value[0] + filter_data->value[1];
}

bool detectPulse(uint32_t sensor_value)
{
   static uint32_t prev_sensor_value = 0;
   static uint8_t values_went_down = 0;
   static uint32_t currentBeat = 0;
   static uint32_t lastBeat = 0;

   if(sensor_value > PULSE_MAX_THRESHOLD)
   {
      currentPulseDetectorState = PULSE_IDLE;
      prev_sensor_value = 0;
      lastBeat = 0;
      currentBeat = 0;
      values_went_down = 0;
      lastBeatThreshold = 0;
      return false;
   }

   switch(currentPulseDetectorState)
   {
      case PULSE_IDLE:
   if(sensor_value >= PULSE_MIN_THRESHOLD) {
      currentPulseDetectorState = PULSE_TRACE_UP;
      values_went_down = 0;
   }
   break;

      case PULSE_TRACE_UP:
   if(sensor_value > prev_sensor_value)
   {
      currentBeat = micros;
      lastBeatThreshold = sensor_value;
   }
   else
   {
      uint32_t beatDuration = currentBeat - lastBeat;
      lastBeat = currentBeat;

      uint32_t rawBPM = 0;
      if(beatDuration > 0)
         rawBPM = 60000000 / beatDuration;
      valuesBPM[bpmIndex] = rawBPM;
      valuesBPMSum = 0;
      for(int i=0; i<PULSE_BPM_SAMPLE_SIZE; i++)
      {
         valuesBPMSum += valuesBPM[i];
      }
      bpmIndex++;
      bpmIndex = bpmIndex % PULSE_BPM_SAMPLE_SIZE;

      if(valuesBPMCount < PULSE_BPM_SAMPLE_SIZE)
         valuesBPMCount++;

      currentBPM = valuesBPMSum / valuesBPMCount;
      sl_app_log(" BPM: %d \n", currentBPM);

      currentPulseDetectorState = PULSE_TRACE_DOWN;

      return true;
   }
   break;

      case PULSE_TRACE_DOWN:
   if(sensor_value < prev_sensor_value)
   {
      values_went_down++;
   }


   if(sensor_value < PULSE_MIN_THRESHOLD)
   {
      currentPulseDetectorState = PULSE_IDLE;
   }
   break;
   }

   prev_sensor_value = sensor_value;
   return false;
}
