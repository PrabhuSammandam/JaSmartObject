/*
 * cplusplus.cpp
 *
 *  Created on: May 24, 2017
 *      Author: psammand
 */

#include <cstddef>
#include <c_types.h>

#include "esp_libc.h"

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

extern "C" int atexit(void (*function)(void))
{
	return 0;
}

