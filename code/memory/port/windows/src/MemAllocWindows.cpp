/*
 * MemAllocWindows.cpp
 *
 *  Created on: Jul 25, 2017
 *      Author: psammand
 */

#ifdef _OS_WINDOWS_

#include "port/windows/inc/MemAllocWindows.h"

namespace ja_iot {
namespace memory {
MemAllocWindows::MemAllocWindows() {
}

MemAllocWindows::~MemAllocWindows() {
}

void * MemAllocWindows::alloc(size_t mem_size) {
	return (::operator new(mem_size));
}

void MemAllocWindows::free(void *p_memory) {
	::operator delete(p_memory);
}
}
}

#endif /* _OS_WINDOWS_ */
