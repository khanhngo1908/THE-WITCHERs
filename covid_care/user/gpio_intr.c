/*
 * gpio_intr.c
 *
 *  Created on: 18 thg 3, 2022
 *      Author: Ngo Minh Khanh
 */

#include "gpio_intr.h"
#include "stdint.h"
#include "max30102.h"
#include "gpiointerrupt.h"

void gpio_INTR_init (void)
{
	CMU_ClockEnable (cmuClock_GPIO, true);
	GPIOINT_Init ();
	// Configure Button PB1 as input and enable interrupt
	GPIO_PinModeSet (mpu_intr_port, mpu_intr_pin, gpioModeInput, 1);
	GPIO_ExtIntConfig (mpu_intr_port,
					   mpu_intr_pin,
					   mpu_intr_pin,
						true,
						false,
						true);

	GPIO_ExtIntConfig (button_port,
		button_pin,
							button_pin,
							false,
							true,
							true);
//  GPIO_ExtIntConfig(intr_port,intr_pin, AD5940_INT_DATA_FIFO_FLAG, 0, 1, true);
	GPIOINT_CallbackRegister (mpu_intr_pin, (void*) ad5940_gpio_ext_handler);
	GPIOINT_CallbackRegister (button_pin, (void*) ad5940_gpio_ext_handler);

//  // Enable EVEN interrupt to catch button press that changes slew rate
//  NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
//  NVIC_EnableIRQ(GPIO_EVEN_IRQn);
//
//  // Enable ODD interrupt to catch button press that changes slew rate
//  NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
//  NVIC_EnableIRQ(GPIO_ODD_IRQn);

}

//void IRQ_Handler (void)
//{
//	// Get and clear all pending GPIO interrupts
//	uint32_t interruptMask = GPIO_IntGet ();
//	GPIO_IntClear (interruptMask);
//
//	// Check if button 0 was pressed
//	if (interruptMask & (1 << mpu_intr_pin))
//	{
////          MAX30102_ReadFIFO();
//		MAX30102_ClearIntr ();
//		GPIO_PinOutToggle (LED_on_board_port, LED_on_board_pin);
////          MAX30102_CheckReg();
//	}
//}

// /**************************************************************************//**
//  * @brief GPIO Interrupt handler for even pins.
//  *****************************************************************************/
//void GPIO_EVEN_IRQHandler(void)
//{
//    IRQ_Handler();
//}
//
// /**************************************************************************//**
//  * @brief GPIO Interrupt handler for even pins.
//  *****************************************************************************/
// void GPIO_ODD_IRQHandler(void)
// {
//    IRQ_Handler();
// }

void ad5940_gpio_ext_handler (uint32_t int_num)
{
	if (int_num == mpu_intr_pin)
	{
		sl_bt_external_signal (mpu_intr_pin);
	}
	if (int_num == button_pin)
	{
		sl_bt_external_signal (button_pin);
	}
}
