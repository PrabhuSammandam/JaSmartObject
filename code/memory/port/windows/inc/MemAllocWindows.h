/*
 * MemAllocWindows.h
 *
 *  Created on: Jul 25, 2017
 *      Author: psammand
 */

#pragma once

#ifdef _OS_WINDOWS_

#include <IMemAllocator.h>

namespace ja_iot
{
  namespace memory
  {
    class MemAllocWindows : public IMemAllocator
    {
    public:

      MemAllocWindows();

      ~MemAllocWindows();

      void* alloc(size_t mem_size) override;
      void free(void* p_memory) override;
    };
  }
}

#endif /* _OS_WINDOWS_ */
