#ifndef __JA_KEY_MGR_H__
#define __JA_KEY_MGR_H__

#include "driver/ja_gpio.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef void (*key_callback_func)(void);

typedef struct single_key_param_t_
{
	uint8 key_level;
	uint8 gpio_no;
	os_timer_t key_5s;
	os_timer_t key_50ms;
	key_callback_func short_key_press_cb_func;
	key_callback_func long_key_press_cb_func;
} single_key_param_t;

typedef struct keys_container_t_
{
	uint8 no_of_keys;
	single_key_param_t ** pp_key_list;
} keys_container_t;

single_key_param_t* ja_key_mgr_create_key(uint8 gpio_no, key_callback_func long_press, key_callback_func short_press);
void ja_key_mgr_init(keys_container_t* keys_container);

#ifdef __cplusplus
}
#endif

#endif /*__JA_KEY_MGR_H__*/
