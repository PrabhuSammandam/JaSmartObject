/*
 * MemAllocWindows.h
 *
 *  Created on: Jul 25, 2017
 *      Author: psammand
 */

#ifndef MEMALLOCWINDOWS_H_
#define MEMALLOCWINDOWS_H_

#ifdef _OS_WINDOWS_

#include <IMemAllocator.h>

namespace ja_iot {
namespace memory {
class MemAllocWindows : public IMemAllocator
{
  public:

    MemAllocWindows ();

    ~MemAllocWindows ();

  public:
    void* alloc( size_t mem_size ) override;
    void  free( void *p_memory )   override;
};
}
}

#endif /* _OS_WINDOWS_ */
#endif /* MEMALLOCWINDOWS_H_ */
