/*
 * IMemAllocator.cpp
 *
 *  Created on: Jul 25, 2017
 *      Author: psammand
 */

#include "IMemAllocator.h"
#include <stdio.h>

#ifdef _OS_LINUX_
#include "port/linux/inc/MemAllocLinux.h"
#endif

namespace ja_iot {
namespace memory {

#ifdef _OS_WINDOWS_
static MemAllocWindows gs_mem_allocator_windows{};
#endif

#ifdef _OS_LINUX_
static MemAllocLinux gs_mem_allocator_linux{};
#endif

IMemAllocator *MemAllocatorFactory::cur_mem_allocator_factory_ = nullptr;

void *IMemAllocator::alloc(const char *file_name, const uint32_t line_no,
                           size_t mem_size) {
  auto ptr = MemAllocatorFactory::get().alloc(mem_size);

  printf("ALLOC F[%s], L[%u], S[%u], M[0x%p]\n", file_name, line_no,
         (uint32_t)mem_size, (void *)ptr);
  return (ptr);
}

void IMemAllocator::free(const char *file_name, const uint32_t line_no,
                         void *p_memory) {
  printf("FREE F[%s], L[%u], M[0x%p]\n", file_name, line_no, p_memory);
  MemAllocatorFactory::get().free(p_memory);
}

MemAllocatorFactory::MemAllocatorFactory() {}

MemAllocatorFactory::~MemAllocatorFactory() {}

IMemAllocator *MemAllocatorFactory::get(MemAlloctorType mem_allocator_type) {
#ifdef _OS_WINDOWS_

  if (mem_allocator_type == MemAlloctorType::kWindows) {
    return (&gs_mem_allocator_windows);
  }

#endif

#ifdef _OS_LINUX_

  if (mem_allocator_type == MemAlloctorType::kLinux) {
    return (&gs_mem_allocator_linux);
  }

#endif

  return (nullptr);
}
}
}
