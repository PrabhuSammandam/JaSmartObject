#include "esp_common.h"
#include "esp_timer.h"
#include "esp8266/ets_sys.h"

#include "driver/ja_key_mgr.h"
#include "driver/ja_gpio.h"

//#define _DEBUG

#ifdef _DEBUG
#define DBG(format,...) printf(format"\n", ##__VA_ARGS__)
#else
#define DBG(format,...)
#endif /*_DEBUG*/

static void key_intr_handler(void* userParam);

static keys_container_t _keys_container = {};

single_key_param_t * ICACHE_FLASH_ATTR ja_key_mgr_create_key(uint8 gpio_num, key_callback_func long_press_cb_func,
                                                             key_callback_func short_press_cb_func)
{
	single_key_param_t* p_key_info = (single_key_param_t*) os_zalloc(sizeof(single_key_param_t));

	p_key_info->gpio_no = gpio_num;
	p_key_info->long_key_press_cb_func = long_press_cb_func;
	p_key_info->short_key_press_cb_func = short_press_cb_func;

	DBG("Created the single key, GPIO no %d\n", gpio_num);

	return p_key_info;
}

#define ETS_GPIO_INTR_ENABLE() _xt_isr_unmask ((1 << ETS_GPIO_INUM))
#define ETS_GPIO_INTR_DISABLE() _xt_isr_mask ((1 << ETS_GPIO_INUM))

void ICACHE_FLASH_ATTR ja_key_mgr_init(keys_container_t* p_key_list)
{
	uint8 i = 0;

	if (p_key_list == NULL || (p_key_list->no_of_keys > 0 && p_key_list->pp_key_list == NULL))
	{
		return;
	}

	DBG("ja_key_mgr_init=> ENTER\n");

	_keys_container.no_of_keys = p_key_list->no_of_keys;
	_keys_container.pp_key_list = p_key_list->pp_key_list;

	_xt_isr_attach(ETS_GPIO_INUM, key_intr_handler, &_keys_container);
	DBG("ja_key_mgr_init=> Attached GPIO interrupt handler, user_param %u\n", &_keys_container);

	ETS_GPIO_INTR_DISABLE();
	DBG("ja_key_mgr_init=> Disabled GPIO interrupt\n");

	for (i = 0; i < _keys_container.no_of_keys; i++)
	{
		single_key_param_t* p_single_key_param = _keys_container.pp_key_list[i];

		if (p_single_key_param == NULL)
		{
			continue;
		}

		uint8 curr_gpio_no = p_single_key_param->gpio_no;
		p_single_key_param->key_level = 1;

		DBG("ja_key_mgr_init=> Applying settings to GPIO no%d\n", curr_gpio_no);

		ja_gpio_set_pin_input(curr_gpio_no);
		DBG("ja_key_mgr_init=> Set to input, GPIO no%d\n", curr_gpio_no);
		ja_gpio_pin_set_driver_type(curr_gpio_no, GPIO_DRIVER_TYPE_NORMAL);
		DBG("ja_key_mgr_init=> Set driver type, GPIO no%d\n", curr_gpio_no);
		ja_gpio_pin_set_source_type(curr_gpio_no, GPIO_SOURCE_TYPE_GPIO_DATA);
		DBG("ja_key_mgr_init=> Set source type, GPIO no%d\n", curr_gpio_no);
		ja_gpio_clear_interrupt(curr_gpio_no);
		DBG("ja_key_mgr_init=> Cleared interrupt, GPIO no%d\n", curr_gpio_no);
		ja_gpio_pin_set_interrupt_type(curr_gpio_no, GPIO_INTERRUPT_TYPE_NEGATIVE_EDGE);
		DBG("ja_key_mgr_init=> Set interrupt type, GPIO no%d\n", curr_gpio_no);
	}

	ETS_GPIO_INTR_ENABLE();
	DBG("ja_key_mgr_init=> Enabled GPIO interrupt\n");

	DBG("ja_key_mgr_init=> EXIT\n");
}

/******************************************************************************
 * FunctionName : key_5s_cb
 * Description  : long press 5s timer callback
 * Parameters   : single_key_param_t *single_key - single key parameter
 * Returns      : none
 *******************************************************************************/
void ja_key_mgr_intl_5s_timer_cb(void* arg)
{
	single_key_param_t* key_data = arg;
	os_timer_disarm(&key_data->key_5s);

	// low, then restart
	if (ja_gpio_is_pin_low(key_data->gpio_no))
	{
		DBG("ja_key_mgr_intl_5s_timer_cb=> 5s timer handler called\n");
		if (key_data->long_key_press_cb_func)
		{
			key_data->long_key_press_cb_func();
		}
	}
}

/******************************************************************************
 * FunctionName : key_50ms_cb
 * Description  : 50ms timer callback to check it's a real key push
 * Parameters   : single_key_param_t *single_key - single key parameter
 * Returns      : none
 *******************************************************************************/
void ja_key_mgr_intl_50ms_timer_cb(void* arg)
{
	single_key_param_t* key_data = arg;
	os_timer_disarm(&key_data->key_50ms);

	// high, then key is up
	if (ja_gpio_is_pin_high(key_data->gpio_no))
	{
		DBG("ja_key_mgr_intl_50ms_timer_cb=> 50ms timer handler called\n");
		os_timer_disarm(&key_data->key_5s);
		key_data->key_level = 1;
		ja_gpio_pin_set_interrupt_type(key_data->gpio_no, GPIO_INTERRUPT_TYPE_NEGATIVE_EDGE);

		if (key_data->short_key_press_cb_func)
		{
			key_data->short_key_press_cb_func();
		}
	}
	else
	{
		ja_gpio_pin_set_interrupt_type(key_data->gpio_no, GPIO_INTERRUPT_TYPE_POSITIVE_EDGE);
	}
}

static void key_intr_handler(void* userParam)
{
	keys_container_t* keys_container = userParam;
	uint8 i = 0;

	if (keys_container == NULL)
	{
		DBG("key_intr_handler-> user_param is null\n");
	}

	DBG("key_intr_handler=> ENTER user_param %u no of keys[%d]\n", keys_container, keys_container->no_of_keys);

	for (i = 0; i < keys_container->no_of_keys; i++)
	{
		single_key_param_t* p_current_key = keys_container->pp_key_list[i];

		if (p_current_key == NULL)
		{
			continue;
		}

		if (ja_gpio_is_interrupted(p_current_key->gpio_no))
		{
			DBG("key_intr_handler=> Interrupt on GPIO %d\n", p_current_key->gpio_no);
			//disable interrupt
			ja_gpio_pin_set_interrupt_type(p_current_key->gpio_no, GPIO_INTERRUPT_TYPE_DISABLE);

			//clear interrupt status
			ja_gpio_clear_interrupt(p_current_key->gpio_no);
			if (p_current_key->key_level == 1)
			{
				// 5s, restart
				os_timer_disarm(&p_current_key->key_5s);
				os_timer_setfn(&p_current_key->key_5s, ja_key_mgr_intl_5s_timer_cb, p_current_key);
				os_timer_arm(&p_current_key->key_5s, 5000, 0);
				p_current_key->key_level = 0;
				ja_gpio_pin_set_interrupt_type(p_current_key->gpio_no, GPIO_INTERRUPT_TYPE_POSITIVE_EDGE);
				DBG("key_intr_handler=> Set 5s timer\n");
			}
			else
			{
				// 50ms, check if this is a real key up
				os_timer_disarm(&p_current_key->key_50ms);
				os_timer_setfn(&p_current_key->key_50ms, ja_key_mgr_intl_50ms_timer_cb, p_current_key);
				os_timer_arm(&p_current_key->key_50ms, 50, 0);
				DBG("key_intr_handler=> Set 50ms timer\n");
			}
		}
	}

	DBG("key_intr_handler=> EXIT\n");
}
