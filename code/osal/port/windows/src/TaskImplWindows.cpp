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

TaskImplWindowsSem::TaskImplWindowsSem ()
{
}

TaskImplWindowsSem::~TaskImplWindowsSem ()
{
}

OsalError TaskImplWindowsSem::PortCreateTask()
{
  _taskHandle = CreateThread( NULL, 0, (PTHREAD_START_ROUTINE) task_base_function, this, 0, NULL );

  if( _taskHandle == NULL )
  {
    return ( OsalError::ERR );
  }

  return ( OsalError::OK );
}

OsalError TaskImplWindowsSem::PortDeleteTask()
{
  if( _taskHandle != 0 )
  {
    CloseHandle( _taskHandle );
    _taskHandle = 0;
  }

  return ( OsalError::OK );
}

OsalError TaskImplWindowsSem::Wait()
{
  OsalError res         = OsalError::OK;
  DWORD     wait_result = WaitForSingleObject( _taskHandle, INFINITE );

  if( WAIT_OBJECT_0 != wait_result )
  {
    res = OsalError::ERR;
  }

  return ( res );
}

static void task_base_function( void *arg )
{
  TaskBase *task_base = (TaskBase *) arg;

  if( task_base != nullptr )
  {
    task_base->Run();
  }
}
}
}

#endif /* _OS_WINDOWS_ */