#ifndef __JA_GPIO_H__
#define __JA_GPIO_H__

#include <c_types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*Following defines are convenience for the named gpio nos instead of the constant number.
 This the GPIO no and it not the pin nos as used in the arduino style. In arduino the pins are
 referenced by digital and analog pins. But in esp8266 the GPIO nos are already defined. */
#define GPIO_NO_0   0
#define GPIO_NO_1   1
#define GPIO_NO_2   2
#define GPIO_NO_3   3
#define GPIO_NO_4   4
#define GPIO_NO_5   5
#define GPIO_NO_6   6
#define GPIO_NO_7   7
#define GPIO_NO_8   8
#define GPIO_NO_9   9
#define GPIO_NO_10  10
#define GPIO_NO_11  11
#define GPIO_NO_12  12
#define GPIO_NO_13  13
#define GPIO_NO_14  14
#define GPIO_NO_15  15

typedef enum gpio_pin_interrupt_type_
{
	GPIO_INTERRUPT_TYPE_DISABLE = 0,
	GPIO_INTERRUPT_TYPE_POSITIVE_EDGE = 1,
	GPIO_INTERRUPT_TYPE_NEGATIVE_EDGE = 2,
	GPIO_INTERRUPT_TYPE_BOTH_EDGES = 3,
	GPIO_INTERRUPT_TYPE_LOW_LEVEL = 4,
	GPIO_INTERRUPT_TYPE_HIGH_LEVEL = 5
} gpio_pin_interrupt_type_e;

typedef enum gpio_pin_driver_type_
{
	GPIO_DRIVER_TYPE_NORMAL = 0, GPIO_DRIVER_TYPE_OPEN_DRAIN = 1,
} gpio_pin_driver_type_e;

typedef enum gpio_pin_source_type_
{
	GPIO_SOURCE_TYPE_GPIO_DATA = 0, GPIO_SOURCE_TYPE_SIGMA_DELTA = 1,
} gpio_pin_source_type_e;

void ja_gpio_set_pin_output(uint8 gpio_no);
void ja_gpio_set_pin_input(uint8 gpio_no);

void ja_gpio_pin_enable_wakeup(uint8 gpio_no, uint8 is_enable);
uint8 ja_gpio_pin_is_wakeup_enabled(uint8 gpio_no);

void ja_gpio_pin_set_interrupt_type(uint8 gpio_no, gpio_pin_interrupt_type_e interrupt_type);
gpio_pin_interrupt_type_e ja_gpio_pin_get_interrupt_type(uint8 gpio_no);

void ja_gpio_pin_set_driver_type(uint8 gpio_no, gpio_pin_driver_type_e driver_type);
gpio_pin_driver_type_e ja_gpio_pin_get_driver_type(uint8 gpio_no);

void ja_gpio_pin_set_source_type(uint8 gpio_no, gpio_pin_source_type_e source_type);
gpio_pin_source_type_e ja_gpio_pin_get_source_type(uint8 gpio_no);

void ja_gpio_enable_pull_up(uint8 gpio_no);
void ja_gpio_disable_pull_up(uint8 gpio_no);

void ja_gpio_set_pin_value(uint8 gpio_no, uint8 value);

uint8 ja_gpio_is_pin_high(uint8 gpio_no);
uint8 ja_gpio_is_pin_low(uint8 gpio_no);

void ja_gpio_clear_interrupt(uint8 gpio_no);
uint8 ja_gpio_is_interrupted(uint8 gpio_no);

void gpio16_output_set(uint8 value);
void gpio16_output_conf(void);


#ifdef __cplusplus
}
#endif

#endif /*__JA_GPIO_H__*/
