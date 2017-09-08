
/*
 * SemaphoreImplFreertos.cpp
 *
 *  Created on: Sep 8, 2017
 *      Author: psammand
 */
#ifdef _OS_FREERTOS_

#include <port/freertos/inc/SemaphoreImplFreertos.h>

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
  semaphore_handle_ = xSemaphoreCreateCounting( access_count, initial_count );

  if( semaphore_handle_ == nullptr )
  {
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
  if( xSemaphoreTake( semaphore_handle_, portMAX_DELAY ) == pdFALSE )
  {
    return ( OsalError::ERR );
  }

  return ( OsalError::OK );
}

OsalError SemaphoreImplFreertos::Post()
{
  if(xSemaphoreGive( semaphore_handle_ ) == pdFALSE)
  {
	  return ( OsalError::ERR );
  }

  return ( OsalError::OK );
}
}  // namespace osal
}  // namespace ja_iot

#endif /* _OS_FREERTOS_ */
