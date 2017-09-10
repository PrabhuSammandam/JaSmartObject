
/*
 * SemaphoreImplFreertos.cpp
 *
 *  Created on: Sep 8, 2017
 *      Author: psammand
 */
#ifdef _OS_FREERTOS_

#include <port/freertos/inc/SemaphoreImplFreertos.h>


#ifdef _DEBUG_
#define dbg( format, ... ) printf( format "\n", ## __VA_ARGS__ )
#else
#define dbg( format, ... )
#endif

namespace ja_iot {
namespace osal {
SemaphoreImplFreertos::SemaphoreImplFreertos ()
{
}

SemaphoreImplFreertos::~SemaphoreImplFreertos ()
{
}

OsalError SemaphoreImplFreertos::Init( uint32_t initial_count, uint32_t access_count )
{
  dbg( "%s=>Init semaphore\n", __FUNCTION__ );
  semaphore_handle_ = xSemaphoreCreateCounting( access_count, initial_count );

  if( semaphore_handle_ == nullptr )
  {
    dbg( "%s=>Init semaphore failed\n", __FUNCTION__ );
    return ( OsalError::ERR );
  }

  return ( OsalError::OK );
}

OsalError SemaphoreImplFreertos::Uninit()
{
  vSemaphoreDelete( semaphore_handle_ );
  semaphore_handle_ = nullptr;

  return ( OsalError::OK );
}

OsalError SemaphoreImplFreertos::Wait()
{
  dbg( "%s=>Wait semaphore\n", __FUNCTION__ );

  if( xSemaphoreTake( semaphore_handle_, portMAX_DELAY ) == pdFALSE )
  {
    dbg( "%s=>Wait semaphore failed\n", __FUNCTION__ );
    return ( OsalError::ERR );
  }

  return ( OsalError::OK );
}

OsalError SemaphoreImplFreertos::Post()
{
  dbg( "%s=>Post semaphore\n", __FUNCTION__ );

  if( xSemaphoreGive( semaphore_handle_ ) == pdFALSE )
  {
    dbg( "%s=>Post semaphore failed\n", __FUNCTION__ );
    return ( OsalError::ERR );
  }

  return ( OsalError::OK );
}
}  // namespace osal
}  // namespace ja_iot

#endif /* _OS_FREERTOS_ */