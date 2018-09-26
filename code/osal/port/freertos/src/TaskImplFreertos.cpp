/*
 * TaskImplFreertos.cpp
 *
 *  Created on: Sep 8, 2017
 *      Author: psammand
 */
#ifdef _OS_FREERTOS_

#include <stdio.h>
#include "port/freertos/inc/TaskImplFreertos.h"
#include "OsalError.h"
#include "OsalMgr.h"
#include "ScopedMutex.h"

//#define _DEBUG_

#ifdef _DEBUG_
#define dbg( format, ... ) printf( format "\n", ## __VA_ARGS__ )
#else
#define dbg( format, ... )
#endif

namespace ja_iot {
namespace osal {
static void task_base_function( void *arg );
TaskImplFreertos::TaskImplFreertos ()
{
}
TaskImplFreertos::~TaskImplFreertos ()
{
}

OsalError TaskImplFreertos::port_create_task()
{
  if( xTaskCreate( task_base_function,
    (const signed char *) _st_task_params.cz_name.c_str(),
    _st_task_params.u16_stack_size,
    this
                 , _st_task_params.u32_priority
                 , &task_handle_ ) == pdFALSE )
  {
    dbg( "%s=> Failed to created task\n", __FUNCTION__ );
    return ( OsalError::ERR );
  }

  dbg( "%s=> Created task successfully\n", __FUNCTION__ );

  return ( OsalError::OK );
}

OsalError TaskImplFreertos::port_delete_task()
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
