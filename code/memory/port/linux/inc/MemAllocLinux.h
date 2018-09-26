/*
 * MemAllocLinux.h
 *
 *  Created on: 03-Sep-2017
 *      Author: prabhu
 */

#pragma once

#ifdef _OS_LINUX_

#include <IMemAllocator.h>

namespace ja_iot {
  namespace memory {

    class MemAllocLinux : public IMemAllocator
    {
    public:

      MemAllocLinux();

      ~MemAllocLinux();

    public:
      void* alloc(size_t mem_size) override;
      void  free(void *p_memory)   override;
    };

  }  // namespace memory
}  // namespace ja_iote

#endif /* _OS_LINUX_ */
