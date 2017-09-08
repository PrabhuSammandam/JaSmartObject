/*
 * MutexImplFreertos.cpp
 *
 *  Created on: Sep 8, 2017
 *      Author: psammand
 */

#ifdef _OS_FREERTOS_

#include "port/freertos/inc/MutexImplFreertos.h"

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
  mutex_impl_ = xSemaphoreCreateRecursiveMutex();

  if( mutex_impl_ == nullptr )
  {
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
    if( xSemaphoreTakeRecursive( mutex_impl_, portMAX_DELAY ) == pdFALSE )
    {
      return ( OsalError::ERR );
    }
  }

  return ( OsalError::OK );
}

OsalError MutexImplFreertos::Unlock()
{
  if( mutex_impl_ != nullptr )
  {
    xSemaphoreGiveRecursive( mutex_impl_ );
  }

  return ( OsalError::OK );
}
}  // namespace osal
}  // namespace ja_iot
#endif /* _OS_FREERTOS_ */