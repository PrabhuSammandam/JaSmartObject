#include "driver/ja_gpio.h"
#include "esp_common.h"

static uint32   ja_gpio_get_mux_reg_intl(uint8 gpio_no);

void ja_gpio_set_pin_output(uint8   gpio_no)
{
    /* set the direction of the gpio pin to output*/
    GPIO_REG_WRITE(GPIO_ENABLE_W1TS_ADDRESS, BIT(gpio_no));

    uint32  gpio_mux_reg = ja_gpio_get_mux_reg_intl(gpio_no);

    if((gpio_no == 0) || (gpio_no == 2) || (gpio_no == 4) || (gpio_no == 5)) {
        PIN_FUNC_SELECT(gpio_mux_reg, 0);
    }
    else {
        PIN_FUNC_SELECT(gpio_mux_reg, 3);
    }
}

void ja_gpio_set_pin_input(uint8   gpio_no)
{
    GPIO_REG_WRITE(GPIO_ENABLE_W1TC_ADDRESS, BIT(gpio_no));
}

void ja_gpio_set_pin_value(uint8   gpio_no,
                           uint8   value)
{
    // writing 1 in GPIO_OUT_W1TS_ADDRESS sets the output high
    // writing 1 in GPIO_OUT_W1TC_ADDRESS sets the output low
    uint32  to_write_reg = (value > 0) ? GPIO_OUT_W1TS_ADDRESS : GPIO_OUT_W1TC_ADDRESS;
    GPIO_REG_WRITE(to_write_reg, BIT(gpio_no));
}

uint8 ja_gpio_is_pin_high(uint8   gpio_no)
{
    uint32  input_reg = GPIO_REG_READ(GPIO_IN_ADDRESS);

    return ((input_reg & BIT(gpio_no)) != 0);
}

uint8 ja_gpio_is_pin_low(uint8   gpio_no)
{
    uint32  input_reg = GPIO_REG_READ(GPIO_IN_ADDRESS);

    return ((input_reg & BIT(gpio_no)) == 0);
}

void ja_gpio_clear_interrupt(uint8   gpio_no)
{
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(gpio_no));
}

uint8 ja_gpio_is_interrupted(uint8   gpio_no)
{
    return ((GPIO_REG_READ(GPIO_STATUS_ADDRESS) & BIT(gpio_no)) != 0);
}

#define GPIO_PIN0_REG_ADDR              0x28                // GPIO pin 0 reg address offset
#define GPIO_PIN_REG_ADDR(_x_)          (GPIO_PIN0_REG_ADDR + _x_ * 4)

#define GPIO_WAKEUP_ENABLE_BITS_MASK    0x00000400          /*WAKEUP is single bit  value and it is 10th bit of gpio register*/
#define GPIO_INT_TYPE_BITS_MASK         0x00000380          /*INTERRUPT type is defined in 7,8,9 bit of gpio register*/
#define GPIO_PAD_DRIVER_MASK            0x00000004          /*driver type is defined in the 2 bit of gpio register*/
#define GPIO_SOURCE_MASK                0x00000001          /*driver type is defined in the 0 bit of gpio register*/

void ja_gpio_pin_enable_wakeup(uint8   gpio_no,
                               uint8   is_enable)
{
    uint32  gpio_reg_addr = GPIO_PIN_REG_ADDR(gpio_no);     /*convinient to get the gpio register*/
    uint32  gpio_reg_value = GPIO_REG_READ(gpio_reg_addr);  /*read the current value of the gpio register*/

    if(is_enable) {
        GPIO_REG_WRITE(gpio_reg_addr, (gpio_reg_value | GPIO_WAKEUP_ENABLE_BITS_MASK));
    }
    else {
        GPIO_REG_WRITE(gpio_reg_addr, (gpio_reg_value & (~GPIO_WAKEUP_ENABLE_BITS_MASK)));
    }
}

uint8 ja_gpio_pin_is_wakeup_enabled(uint8   gpio_no)
{
    uint32  gpio_reg_value = GPIO_REG_READ(GPIO_PIN_REG_ADDR(gpio_no));

    return ((gpio_reg_value & GPIO_WAKEUP_ENABLE_BITS_MASK) != 0);
}

void ja_gpio_pin_set_interrupt_type(uint8                       gpio_no,
                                    gpio_pin_interrupt_type_e   interrupt_type)
{
    uint32  gpio_reg_addr = GPIO_PIN_REG_ADDR(gpio_no);     /*Convenient to get the gpio register*/
    uint32  gpio_reg_value = GPIO_REG_READ(gpio_reg_addr);  /*read the current value of the gpio register*/

    gpio_reg_value &= (~GPIO_INT_TYPE_BITS_MASK);           // first clear the interrupt type bits
    gpio_reg_value |= (interrupt_type << 7);                // since it start from 7th bit need to shift the passed type
    GPIO_REG_WRITE(gpio_reg_addr, gpio_reg_value);
}

gpio_pin_interrupt_type_e ja_gpio_pin_get_interrupt_type(uint8   gpio_no)
{
    uint32  gpio_reg_value = GPIO_REG_READ(GPIO_PIN_REG_ADDR(gpio_no)) & GPIO_INT_TYPE_BITS_MASK;

    return (gpio_pin_interrupt_type_e) (gpio_reg_value >> 7);
}

void ja_gpio_pin_set_driver_type(uint8                   gpio_no,
                                 gpio_pin_driver_type_e  driver_type)
{
    uint32  gpio_reg_addr = GPIO_PIN_REG_ADDR(gpio_no);     /*Convenient to get the gpio register*/
    uint32  gpio_reg_value = GPIO_REG_READ(gpio_reg_addr);  /*read the current value of the gpio register*/

    gpio_reg_value &= (~GPIO_PAD_DRIVER_MASK);              // first clear the driver type bits
    gpio_reg_value |= (driver_type << 2);                   // since it start from 3th bit need to shift the passed type
    GPIO_REG_WRITE(gpio_reg_addr, gpio_reg_value);
}

gpio_pin_driver_type_e ja_gpio_pin_get_driver_type(uint8   gpio_no)
{
    uint32  gpio_reg_value = GPIO_REG_READ(GPIO_PIN_REG_ADDR(gpio_no)) & GPIO_PAD_DRIVER_MASK;

    return (gpio_pin_driver_type_e) (gpio_reg_value >> 2);
}

void ja_gpio_pin_set_source_type(uint8                   gpio_no,
                                 gpio_pin_source_type_e  source_type)
{
    uint32  gpio_reg_addr = GPIO_PIN_REG_ADDR(gpio_no);     /*convinient to get the gpio register*/
    uint32  gpio_reg_value = GPIO_REG_READ(gpio_reg_addr);  /*read the current value of the gpio register*/

    gpio_reg_value &= (~GPIO_SOURCE_MASK);                  // first clear the driver type bits
    gpio_reg_value |= (source_type);
    GPIO_REG_WRITE(gpio_reg_addr, gpio_reg_value);
}

gpio_pin_source_type_e ja_gpio_pin_get_source_type(uint8   gpio_no)
{
    uint32  gpio_reg_value = GPIO_REG_READ(GPIO_PIN_REG_ADDR(gpio_no)) & GPIO_SOURCE_MASK;

    return (gpio_pin_driver_type_e) (gpio_reg_value);
}

void ja_gpio_enable_pull_up(uint8   gpio_no)
{
    uint32  gpio_mux_reg = ja_gpio_get_mux_reg_intl(gpio_no);
    uint32  gpio_mux_reg_value = GPIO_REG_READ(gpio_mux_reg);

    GPIO_REG_WRITE(gpio_mux_reg, (gpio_mux_reg_value | PERIPHS_IO_MUX_PULLUP));
}

void ja_gpio_disable_pull_up(uint8   gpio_no)
{
    uint32  gpio_mux_reg = ja_gpio_get_mux_reg_intl(gpio_no);
    uint32  gpio_mux_reg_value = GPIO_REG_READ(gpio_mux_reg);

    GPIO_REG_WRITE(gpio_mux_reg, (gpio_mux_reg_value & (~PERIPHS_IO_MUX_PULLUP)));
}

static uint32 ja_gpio_get_mux_reg_intl(uint8   gpio_no)
{
    switch(gpio_no)
    {
        case 0:
            return PERIPHS_IO_MUX_GPIO0_U;

        case 1:
            return PERIPHS_IO_MUX_U0TXD_U;

        case 2:
            return PERIPHS_IO_MUX_GPIO2_U;

        case 3:
            return PERIPHS_IO_MUX_U0RXD_U;

        case 4:
            return PERIPHS_IO_MUX_GPIO4_U;

        case 5:
            return PERIPHS_IO_MUX_GPIO5_U;

        case 6:
            return PERIPHS_IO_MUX_SD_CLK_U;

        case 7:
            return PERIPHS_IO_MUX_SD_DATA0_U;

        case 8:
            return PERIPHS_IO_MUX_SD_DATA1_U;

        case 9:
            return PERIPHS_IO_MUX_SD_DATA2_U;

        case 10:
            return PERIPHS_IO_MUX_SD_DATA3_U;

        case 11:
            return PERIPHS_IO_MUX_SD_CMD_U;

        case 12:
            return PERIPHS_IO_MUX_MTDI_U;

        case 13:
            return PERIPHS_IO_MUX_MTCK_U;

        case 14:
            return PERIPHS_IO_MUX_MTMS_U;

        case 15:
            return PERIPHS_IO_MUX_MTDO_U;
    }

    return 0;

}

void gpio16_output_conf(void)
{
    WRITE_PERI_REG(PAD_XPD_DCDC_CONF,
                   (READ_PERI_REG(PAD_XPD_DCDC_CONF) & 0xffffffbc) | (uint32)0x1); 	// mux configuration for XPD_DCDC to output rtc_gpio0

    WRITE_PERI_REG(RTC_GPIO_CONF,
                   (READ_PERI_REG(RTC_GPIO_CONF) & (uint32)0xfffffffe) | (uint32)0x0);	//mux configuration for out enable

    WRITE_PERI_REG(RTC_GPIO_ENABLE,
                   (READ_PERI_REG(RTC_GPIO_ENABLE) & (uint32)0xfffffffe) | (uint32)0x1);	//out enable
}

void gpio16_output_set(uint8 value)
{
    WRITE_PERI_REG(RTC_GPIO_OUT,
                   (READ_PERI_REG(RTC_GPIO_OUT) & (uint32)0xfffffffe) | (uint32)(value & 1));
}

