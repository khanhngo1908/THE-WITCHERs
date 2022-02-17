#include <blynk.h>
#include "em_gpio.h"
#include "em_cmu.h"

void gpio_init(void)
{
    CMU_ClockEnable(cmuClock_GPIO, true);
    GPIO_PinModeSet(gpioPortA, 4, gpioModePushPull, 1);
    GPIO_PinModeSet(gpioPortC, 7, gpioModeInput, 0);
}
void blynk(void)
{
  if (!GPIO_PinInGet(gpioPortC, 7))
  {
      GPIO_PinOutSet(gpioPortA, 4);
  }
  else
  {
      GPIO_PinOutClear(gpioPortA, 4);
  }
}
