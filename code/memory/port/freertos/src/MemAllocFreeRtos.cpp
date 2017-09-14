/*
 * MemAllocFreeRtos.cpp
 *
 *  Created on: Sep 13, 2017
 *      Author: psammand
 */

#ifdef _OS_FREERTOS_
#include <port/freertos/inc/MemAllocFreeRtos.h>

namespace ja_iot {
namespace memory {
MemAllocFreeRtos::MemAllocFreeRtos ()
{
}

MemAllocFreeRtos::~MemAllocFreeRtos ()
{
}

void * MemAllocFreeRtos::alloc( size_t mem_size )
{
  return ( ::operator new ( mem_size ) );
}

void         MemAllocFreeRtos::free( void *p_memory ) {
  ::operator delete ( p_memory );
}
}
}

#endif /* _OS_FREERTOS_ */