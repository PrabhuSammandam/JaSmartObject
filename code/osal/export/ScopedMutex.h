/*
 * ScopedMutex.h
 *
 *  Created on: Jul 1, 2017
 *      Author: psammand
 */

#ifndef OSAL_EXPORT_SCOPEDMUTEX_H_
#define OSAL_EXPORT_SCOPEDMUTEX_H_

#include "Mutex.h"

namespace ja_iot {
namespace osal {
class ScopedMutex
{
  ScopedMutex( const ScopedMutex & )               = delete;
  ScopedMutex & operator = ( const ScopedMutex & ) = delete;
  Mutex * _mutex;

  public:

    ScopedMutex( Mutex const *mutex ) : _mutex{ (Mutex *) mutex }
    {
      if( _mutex != nullptr )
      {
        _mutex->Lock();
      }
    }

    ~ScopedMutex ()
    {
      if( _mutex != nullptr )
      {
        _mutex->Unlock();
      }
    }
};
}
}


#endif /* OSAL_EXPORT_SCOPEDMUTEX_H_ */