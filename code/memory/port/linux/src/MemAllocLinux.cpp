/*
 * MemAllocLinux.cpp
 *
 *  Created on: 03-Sep-2017
 *      Author: prabhu
 */

#include "port/linux/inc/MemAllocLinux.h"

namespace ja_iot {
namespace memory {

MemAllocLinux::MemAllocLinux() {
}

MemAllocLinux::~MemAllocLinux() {
}

void* MemAllocLinux::alloc(size_t mem_size) {
	return (::operator new(mem_size));
}

void MemAllocLinux::free(void* p_memory) {
	::operator delete(p_memory);
}

}  // namespace memory
}  // namespace ja_iot

