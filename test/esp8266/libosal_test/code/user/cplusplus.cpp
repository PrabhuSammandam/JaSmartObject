/*
 * cplusplus.cpp
 *
 *  Created on: May 24, 2017
 *      Author: psammand
 */

#include <cstddef>
#include <c_types.h>

#include "esp_libc.h"

namespace std
{

void __throw_length_error(const char*)
{
	while(1);
}

void __throw_length_error(char* const str)
{
	while(1);
}

void __throw_bad_function_call()
{
	while(1);
}

void __throw_logic_error(const char*)
{
	while(1);
}

void __throw_bad_alloc()
{
	while(1);
}
void
  __throw_out_of_range_fmt(const char*, ...)
{
	while(1);
}
}


void* ICACHE_FLASH_ATTR operator new(size_t size)
{
	return os_malloc(size);
}

void* ICACHE_FLASH_ATTR operator new[](size_t size)
{
	return os_malloc(size);
}

void ICACHE_FLASH_ATTR operator delete(void* ptr)
{
	os_free(ptr);
}

void ICACHE_FLASH_ATTR operator delete(void* ptr, unsigned int size)
{
	os_free(ptr);
}

void ICACHE_FLASH_ATTR operator delete[](void* ptr)
{
	os_free(ptr);
}

extern "C" void ICACHE_FLASH_ATTR __cxa_pure_virtual(void)
{
}

