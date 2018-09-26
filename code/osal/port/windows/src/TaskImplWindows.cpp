/*
 * TaskImplWindowsSem.cpp
 *
 *  Created on: Sep 7, 2017
 *      Author: psammand
 */

#ifdef _OS_WINDOWS_

#include <port/windows/inc/TaskImplWindows.h>

namespace ja_iot {
namespace osal {
static void task_base_function( void *arg );
TaskImplWindows::TaskImplWindows ()
{
}
TaskImplWindows::~TaskImplWindows ()
{
}

OsalError TaskImplWindows::port_create_task()
{
  _taskHandle = CreateThread( nullptr, 0, PTHREAD_START_ROUTINE( task_base_function ), this, 0, nullptr );

  if( _taskHandle == nullptr )
  {
    return ( OsalError::ERR );
  }

  return ( OsalError::OK );
}

OsalError TaskImplWindows::port_delete_task()
{
  if( _taskHandle != nullptr )
  {
    CloseHandle( _taskHandle );
    _taskHandle = nullptr;
  }

  return ( OsalError::OK );
}

OsalError TaskImplWindows::Wait()
{
  auto       res         = OsalError::OK;
  const auto wait_result = WaitForSingleObject( _taskHandle, INFINITE );

  if( WAIT_OBJECT_0 != wait_result )
  {
    res = OsalError::ERR;
  }

  return ( res );
}

static void task_base_function( void *arg )
{
  auto task_base = static_cast<TaskBase *>( arg );

  if( task_base != nullptr )
  {
    task_base->Run();
  }
}
}
}

#endif /* _OS_WINDOWS_ */