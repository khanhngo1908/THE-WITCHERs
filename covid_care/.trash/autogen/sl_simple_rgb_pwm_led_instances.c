///***************************************************************************//**
// * @file
// * @brief Simple RGB PWM LED Driver Instances
// *******************************************************************************
// * # License
// * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
// *******************************************************************************
// *
// * The licensor of this software is Silicon Laboratories Inc.  Your use of this
// * software is governed by the terms of Silicon Labs Master Software License
// * Agreement (MSLA) available at
// * www.silabs.com/about-us/legal/master-software-license-agreement.  This
// * software is distributed to you in Source Code format and is governed by the
// * sections of the MSLA applicable to Source Code.
// *
// ******************************************************************************/
//
//#include "em_gpio.h"
//#include "sl_simple_rgb_pwm_led.h"
//
//#include "sl_simple_rgb_pwm_led_led_config.h"
//
//
//
//sl_led_pwm_t red_led = {
//  .port = SL_SIMPLE_RGB_PWM_LED_LED_RED_PORT,
//  .pin = SL_SIMPLE_RGB_PWM_LED_LED_RED_PIN,
//  .level = SL_SIMPLE_RGB_PWM_LED_LED_RESOLUTION - 1,
//  .polarity = SL_SIMPLE_RGB_PWM_LED_LED_RED_POLARITY,
//  .channel = SL_SIMPLE_RGB_PWM_LED_LED_RED_CHANNEL,
//#if defined(SL_SIMPLE_RGB_PWM_LED_LED_RED_LOC)
//  .location = SL_SIMPLE_RGB_PWM_LED_LED_RED_LOC,
//#endif
//  .timer = SL_SIMPLE_RGB_PWM_LED_LED_PERIPHERAL,
//  .frequency = SL_SIMPLE_RGB_PWM_LED_LED_FREQUENCY,
//  .resolution = SL_SIMPLE_RGB_PWM_LED_LED_RESOLUTION,
//};
//
//sl_led_pwm_t green_led = {
//  .port = SL_SIMPLE_RGB_PWM_LED_LED_GREEN_PORT,
//  .pin = SL_SIMPLE_RGB_PWM_LED_LED_GREEN_PIN,
//  .level = SL_SIMPLE_RGB_PWM_LED_LED_RESOLUTION - 1,
//  .polarity = SL_SIMPLE_RGB_PWM_LED_LED_GREEN_POLARITY,
//  .channel = SL_SIMPLE_RGB_PWM_LED_LED_GREEN_CHANNEL,
//#if defined(SL_SIMPLE_RGB_PWM_LED_LED_GREEN_LOC)
//  .location = SL_SIMPLE_RGB_PWM_LED_LED_GREEN_LOC,
//#endif
//  .timer = SL_SIMPLE_RGB_PWM_LED_LED_PERIPHERAL,
//  .frequency = SL_SIMPLE_RGB_PWM_LED_LED_FREQUENCY,
//  .resolution = SL_SIMPLE_RGB_PWM_LED_LED_RESOLUTION,
//};
//
//sl_led_pwm_t blue_led = {
//  .port = SL_SIMPLE_RGB_PWM_LED_LED_BLUE_PORT,
//  .pin = SL_SIMPLE_RGB_PWM_LED_LED_BLUE_PIN,
//  .level = SL_SIMPLE_RGB_PWM_LED_LED_RESOLUTION - 1,
//  .polarity = SL_SIMPLE_RGB_PWM_LED_LED_BLUE_POLARITY,
//  .channel = SL_SIMPLE_RGB_PWM_LED_LED_BLUE_CHANNEL,
//#if defined(SL_SIMPLE_RGB_PWM_LED_LED_BLUE_LOC)
//  .location = SL_SIMPLE_RGB_PWM_LED_LED_BLUE_LOC,
//#endif
//  .timer = SL_SIMPLE_RGB_PWM_LED_LED_PERIPHERAL,
//  .frequency = SL_SIMPLE_RGB_PWM_LED_LED_FREQUENCY,
//  .resolution = SL_SIMPLE_RGB_PWM_LED_LED_RESOLUTION,
//};
//
//sl_simple_rgb_pwm_led_context_t simple_rgb_pwm_led_context = {
//  .red = &red_led,
//  .green = &green_led,
//  .blue = &blue_led,
//
//  .timer = SL_SIMPLE_RGB_PWM_LED_LED_PERIPHERAL,
//  .frequency = SL_SIMPLE_RGB_PWM_LED_LED_FREQUENCY,
//  .resolution = SL_SIMPLE_RGB_PWM_LED_LED_RESOLUTION,
//};
//
//const sl_led_rgb_pwm_t sl_led = {
//  .led_common.context = &simple_rgb_pwm_led_context,
//  .led_common.init = sl_simple_rgb_pwm_led_init,
//  .led_common.turn_on = sl_simple_rgb_pwm_led_turn_on,
//  .led_common.turn_off = sl_simple_rgb_pwm_led_turn_off,
//  .led_common.toggle = sl_simple_rgb_pwm_led_toggle,
//  .led_common.get_state = sl_simple_rgb_pwm_led_get_state,
//  .set_rgb_color = sl_simple_rgb_pwm_led_set_color,
//  .get_rgb_color = sl_simple_rgb_pwm_led_get_color,
//};
//
//
//
//void sl_simple_rgb_pwm_led_init_instances(void)
//{
//
//  sl_led_init((sl_led_t *)&sl_led);
//
//}
