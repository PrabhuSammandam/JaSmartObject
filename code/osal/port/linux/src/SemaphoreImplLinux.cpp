/*
 * SemaphoreImplLinux.cpp
 *
 *  Created on: 05-Sep-2017
 *      Author: prabhu
 */

#ifdef _OS_LINUX_

#include "port/linux/inc/SemaphoreImplLinux.h"

namespace ja_iot {
namespace osal {
SemaphoreImplLinux::SemaphoreImplLinux ()
{
}

SemaphoreImplLinux::~SemaphoreImplLinux ()
{
}

OsalError SemaphoreImplLinux::Init(uint32_t initial_count, uint32_t access_count )
{
  int ret_status;

  ret_status = sem_init( &semaphore_handle_, initial_count, access_count );

  if( ret_status != 0 )
  {
    return ( OsalError::ERR );
  }

  return ( OsalError::OK );
}

OsalError SemaphoreImplLinux::Uninit()
{
  int ret_status;

  ret_status = sem_destroy( &semaphore_handle_ );

  if( ret_status != 0 )
  {
    return ( OsalError::ERR );
  }

  return ( OsalError::OK );
}

OsalError SemaphoreImplLinux::Wait()
{
  int ret_status;

  ret_status = sem_wait( &semaphore_handle_ );

  if( ret_status != 0 )
  {
    return ( OsalError::ERR );
  }

  return ( OsalError::OK );
}

OsalError SemaphoreImplLinux::Post()
{
  int ret_status;

  ret_status = sem_post( &semaphore_handle_ );

  if( ret_status != 0 )
  {
    return ( OsalError::ERR );
  }

  return ( OsalError::OK );
}
}  // namespace osal
}  // namespace ja_iot

#endif /* _OS_LINUX_ */
