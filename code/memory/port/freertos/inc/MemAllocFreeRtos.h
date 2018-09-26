/*
 * MemAllocFreeRtos.h
 *
 *  Created on: Sep 13, 2017
 *      Author: psammand
 */

#pragma once

#ifdef _OS_FREERTOS_
#include <IMemAllocator.h>

namespace ja_iot {
  namespace memory {
    class MemAllocFreeRtos : public IMemAllocator
    {
    public:

      MemAllocFreeRtos();

      virtual ~MemAllocFreeRtos();

      void* alloc(size_t mem_size) override;
      void  free(void *p_memory)   override;
    };
  }
}
#endif