#include "driver/ja_gpio_led.h"

void ja_gpio_led_init(uint8 gpio_no)
{
	ja_gpio_set_pin_output(gpio_no);
}

void ja_gpio_led_on(uint8 gpio_no)
{
	ja_gpio_set_pin_value(gpio_no, 1);
}

void ja_gpio_led_off(uint8 gpio_no)
{
	ja_gpio_set_pin_value(gpio_no, 0);
}

void ja_gpio_set_led(uint8 gpio_no, uint8 on_off)
{
	ja_gpio_set_pin_value(gpio_no, on_off);
}
