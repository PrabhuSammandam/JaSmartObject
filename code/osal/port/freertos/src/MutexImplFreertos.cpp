/*
 * MutexImplFreertos.cpp
 *
 *  Created on: Sep 8, 2017
 *      Author: psammand
 */

#ifdef _OS_FREERTOS_

#include "port/freertos/inc/MutexImplFreertos.h"

#ifdef _DEBUG_
#define dbg( format, ... ) printf( format "\n", ## __VA_ARGS__ )
#else
#define dbg( format, ... )
#endif

namespace ja_iot {
namespace osal {
MutexImplFreertos::MutexImplFreertos ()
{
}

MutexImplFreertos::~MutexImplFreertos ()
{
}

OsalError MutexImplFreertos::Init()
{
  dbg( "%s=>Initing mutex\n", __FUNCTION__ );
  mutex_impl_ = xSemaphoreCreateRecursiveMutex();

  if( mutex_impl_ == nullptr )
  {
    dbg( "%s=>Initing mutex failed\n", __FUNCTION__ );
    return ( OsalError::ERR );
  }

  return ( OsalError::OK );
}

OsalError MutexImplFreertos::Uninit()
{
  vSemaphoreDelete( mutex_impl_ );

  mutex_impl_ = nullptr;

  return ( OsalError::OK );
}

OsalError MutexImplFreertos::Lock()
{
  if( mutex_impl_ != nullptr )
  {
    dbg( "%s=>Locking mutex\n", __FUNCTION__ );

    if( xSemaphoreTakeRecursive( mutex_impl_, portMAX_DELAY ) == pdFALSE )
    {
      dbg( "%s=>Locking mutex failed\n", __FUNCTION__ );
      return ( OsalError::ERR );
    }
  }

  return ( OsalError::OK );
}

OsalError MutexImplFreertos::Unlock()
{
  dbg( "%s=>UnLocking mutex\n", __FUNCTION__ );

  if( mutex_impl_ != nullptr )
  {
    xSemaphoreGiveRecursive( mutex_impl_ );
  }

  return ( OsalError::OK );
}
}  // namespace osal
}  // namespace ja_iot
#endif /* _OS_FREERTOS_ */