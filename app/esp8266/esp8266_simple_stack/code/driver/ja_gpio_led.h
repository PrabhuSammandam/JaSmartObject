#ifndef __JA_GPIO_LED_H__
#define __JA_GPIO_LED_H__

#include "driver/ja_gpio.h"

#ifdef __cplusplus
extern "C"
{
#endif

void ja_gpio_led_init(uint8 gpio_no);
void ja_gpio_led_on(uint8 gpio_no);
void ja_gpio_led_off(uint8 gpio_no);
void ja_gpio_set_led(uint8 gpio_no, uint8 on_off);

#ifdef __cplusplus
}
#endif
#endif /*__JA_GPIO_LED_H__*/
