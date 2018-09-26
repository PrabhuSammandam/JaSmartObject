/*
 * SemaphoreImplLinux.cpp
 *
 *  Created on: 05-Sep-2017
 *      Author: prabhu
 */

#ifdef _OS_LINUX_

#include <time.h>
#include "port/linux/inc/SemaphoreImplLinux.h"

namespace ja_iot {
namespace osal {
SemaphoreImplLinux::SemaphoreImplLinux ()
{
  semaphore_handle_ = new sem_t{};
}
SemaphoreImplLinux::~SemaphoreImplLinux ()
{
}

OsalError SemaphoreImplLinux::Init( uint32_t initial_count, uint32_t access_count )
{
  int ret_status = sem_init( semaphore_handle_, 0, initial_count );

  if( ret_status != 0 )
  {
    return ( OsalError::ERR );
  }

  return ( OsalError::OK );
}

OsalError SemaphoreImplLinux::Uninit()
{
  int ret_status = sem_destroy( semaphore_handle_ );

  if( ret_status != 0 )
  {
    return ( OsalError::ERR );
  }

  return ( OsalError::OK );
}

OsalError SemaphoreImplLinux::Wait()
{
  int ret_status = sem_wait( semaphore_handle_ );

  if( ret_status != 0 )
  {
    return ( OsalError::ERR );
  }

  return ( OsalError::OK );
}

OsalError SemaphoreImplLinux::Wait( uint32_t timeout_ms )
{
  struct timespec tm {};

  clock_gettime( CLOCK_REALTIME, &tm );

  tm.tv_sec  += ( timeout_ms / 1000 );
  tm.tv_nsec += ( timeout_ms % 1000 ) * 1000000;

  int ret_status = sem_timedwait( semaphore_handle_, &tm );

  if( ret_status != 0 )
  {
    return ( OsalError::ERR );
  }

  return ( OsalError::OK );
}

OsalError SemaphoreImplLinux::Post()
{
  int ret_status = sem_post( semaphore_handle_ );

  if( ret_status != 0 )
  {
    return ( OsalError::ERR );
  }

  return ( OsalError::OK );
}
}  // namespace osal
}  // namespace ja_iot

#endif /* _OS_LINUX_ */