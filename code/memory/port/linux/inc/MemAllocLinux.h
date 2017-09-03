/*
 * MemAllocLinux.h
 *
 *  Created on: 03-Sep-2017
 *      Author: prabhu
 */

#ifndef MEMORY_PORT_LINUX_INC_MEMALLOCLINUX_H_
#define MEMORY_PORT_LINUX_INC_MEMALLOCLINUX_H_

#include <IMemAllocator.h>

namespace ja_iot {
namespace memory {

class MemAllocLinux : public IMemAllocator
{
  public:

	MemAllocLinux ();

    ~MemAllocLinux ();

  public:
    void* alloc( size_t mem_size ) override;
    void  free( void *p_memory )   override;
};

}  // namespace memory
}  // namespace ja_iote

#endif /* MEMORY_PORT_LINUX_INC_MEMALLOCLINUX_H_ */
