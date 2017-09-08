/*
 * TaskImplFreertos.cpp
 *
 *  Created on: Sep 8, 2017
 *      Author: psammand
 */
#ifdef _OS_FREERTOS_

#include <port/freertos/inc/TaskImplFreertos.h>
#include <OsalError.h>
#include "OsalMgr.h"
#include "ScopedMutex.h"

namespace ja_iot {
namespace osal {
static void task_base_function( void *arg );

TaskImplFreertos::TaskImplFreertos ()
{
}

TaskImplFreertos::~TaskImplFreertos ()
{
}

OsalError TaskImplFreertos::PortCreateTask()
{
  if( xTaskCreate( task_base_function, (const signed char *) &task_name_[0], stack_size_, this, task_priority_, &task_handle_ ) == pdPASS )
  {
    return ( OsalError::ERR );
  }

  return ( OsalError::OK );
}

OsalError TaskImplFreertos::PortDeleteTask()
{
  vTaskDelete( this->task_handle_ );
  this->task_handle_ = 0;

  return ( OsalError::OK );
}

OsalError TaskImplFreertos::Wait()
{
  return ( OsalError::OK );
}

void task_base_function( void *arg )
{
  TaskImplFreertos *task_impl_linux = (TaskImplFreertos *) arg;

  if( task_impl_linux != nullptr )
  {
    task_impl_linux->Run();
  }
}
}
}

#endif /* _OS_FREERTOS_ */