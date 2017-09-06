/*
 * SemaphoreImplWindows.cpp
 *
 *  Created on: Sep 5, 2017
 *      Author: psammand
 */

#ifdef _OS_WINDOWS_

#include "port/windows/inc/SemaphoreImplWindows.h"

namespace ja_iot {
namespace osal {
SemaphoreImplWindows::SemaphoreImplWindows ()
{
}

SemaphoreImplWindows::~SemaphoreImplWindows ()
{
}

OsalError SemaphoreImplWindows::Init( uint32_t initial_count, uint32_t access_count )
{
  semaphore_hndl_ = CreateSemaphore(
    NULL,               // default security attributes
    initial_count,                      // initial count
    access_count,                      // maximum count
    NULL );                             // unnamed semaphore

  if( semaphore_hndl_ == NULL )
  {
    return ( OsalError::ERR );
  }

  return ( OsalError::OK );
}

OsalError SemaphoreImplWindows::Uninit()
{
  if( semaphore_hndl_ != NULL )
  {
    CloseHandle( semaphore_hndl_ );
  }

  semaphore_hndl_ = INVALID_HANDLE_VALUE;

  return ( OsalError::OK );
}

OsalError SemaphoreImplWindows::Wait()
{
  DWORD dwWaitResult;

  dwWaitResult = WaitForSingleObject( semaphore_hndl_, INFINITE );

  if( dwWaitResult == WAIT_OBJECT_0 )
  {
    return ( OsalError::OK );
  }

  return ( OsalError::ERR );
}

OsalError SemaphoreImplWindows::Post()
{
  if( semaphore_hndl_ != NULL )
  {
    if( !ReleaseSemaphore( semaphore_hndl_, 1, NULL ) )
    {
      return ( OsalError::ERR );
    }

    return ( OsalError::OK );
  }

  return ( OsalError::ERR );
}
}  // namespace osal
}  // namespace ja_iot

#endif /* _OS_WINDOWS_ */
