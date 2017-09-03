/*
 * TaskImplWindows.cpp
 *
 *  Created on: Jun 27, 2017
 *      Author: psammand
 */

#ifdef _OS_WINDOWS_

#include "windows/inc/TaskImplWindows.h"
#include "OsalMgr.h"
#include "ScopedMutex.h"

namespace ja_iot {
namespace osal {
static void TaskBaseFunction( void *arg );

TaskImplWindows::TaskImplWindows ()
{
}

TaskImplWindows::~TaskImplWindows ()
{
}

OsalError TaskImplWindows::Start()
{
  if( _isMsgQTask )
  {
    if( _isToStop == false )
    {
      /* task already running*/
      return ( OsalError::OK );
    }

    {
      ScopedMutex scoped_mutex( _mutex );
      _isToStop = false;
    }

    _taskHandle = CreateThread( NULL, 0, (PTHREAD_START_ROUTINE) TaskBaseFunction, this, 0, NULL );

    if( _taskHandle == NULL )
    {
      ScopedMutex scoped_mutex( _mutex );
      _isToStop = true;

      return ( OsalError::ERR );
    }
  }
  else
  {
    _taskHandle = CreateThread( NULL, 0, (PTHREAD_START_ROUTINE) TaskBaseFunction, this, 0, NULL );
  }

  return ( OsalError::OK );
}

OsalError TaskImplWindows::Stop()
{
  if( !_isToStop )
  {
    ScopedMutex scoped_mutex( _mutex );
    _isToStop = true;
    _condition->Signal();             // notify the thread
    _condition->Wait( _mutex, 0 );
  }

  return ( OsalError::OK );
}

OsalError TaskImplWindows::Destroy()
{
  if( _msgQ != nullptr )
  {
    if( _mutex != nullptr )
    {
      _mutex->Lock();
    }

    while( _msgQ->IsEmpty() == false )
    {
      auto msg = _msgQ->Dequeue();

      if( msg != nullptr )
      {
        _taskMsgHandler->DeleteMsg( msg );
      }
    }

    _msgQ = nullptr;

    if( _mutex != nullptr )
    {
      _mutex->Unlock();
    }
  }

  if( _mutex != nullptr )
  {
    OsalMgr::Inst()->FreeMutex( _mutex );
  }

  _mutex = nullptr;

  if( _condition != nullptr )
  {
    OsalMgr::Inst()->FreeCondition( _condition );
  }

  _condition = nullptr;

  if( _taskHandle != 0 )
  {
    CloseHandle( _taskHandle );
  }

  return ( OsalError::OK );
}

OsalError TaskImplWindows::Wait()
{
  OsalError res     = OsalError::OK;
  DWORD     joinres = WaitForSingleObject( _taskHandle, INFINITE );

  if( WAIT_OBJECT_0 != joinres )
  {
    res = OsalError::ERR;
  }

  return ( res );
}

OsalError TaskImplWindows::Init( pu8 taskName, u32 taskPriority, u32 stackSize, ITaskRoutine *taskRoutine, pvoid taskArg )
{
  _isMsgQTask   = false;
  _taskPriority = taskPriority;
  _stackSize    = stackSize;
  _taskArg      = taskArg;
  _taskRoutine  = taskRoutine;

  return ( OsalError::OK );
}

OsalError TaskImplWindows::InitWithMsgQ( pu8 pu8_task_name, u32 u32_task_priority, u32 u32_stack_size, TaskMsgQParam *taskMsgQParam, pvoid taskArg )
{
  _mutex        = OsalMgr::Inst()->AllocMutex();
  _condition    = OsalMgr::Inst()->AllocCondition();
  _isToStop     = true;
  _taskPriority = u32_task_priority;
  _stackSize    = u32_stack_size;
  _taskArg      = taskArg;

  if( taskMsgQParam != nullptr )
  {
    _msgQ           = taskMsgQParam->msgQ;
    _taskMsgHandler = taskMsgQParam->taskMsgHandler;
  }

  _isMsgQTask = true;

  return ( OsalError::OK );
}

OsalError TaskImplWindows::SendMsg( pvoid msgMem )
{
  if( ( msgMem == nullptr ) || ( _msgQ == nullptr ) )
  {
    return ( OsalError::INVALID_ARGS );
  }

  ScopedMutex scoped_mutex( _mutex );

  if( _msgQ->Enqueue( msgMem ) == true )
  {
    _condition->Signal();
  }

  return ( OsalError::OK );
}

void TaskImplWindows::Run()
{
  if( _isMsgQTask )
  {
    while( _isToStop == false )
    {
      _mutex->Lock();

      /* if it is not stopped and the message queue is empty then wait for message*/
      if( ( _isToStop == false ) && _msgQ->IsEmpty() )
      {
        _condition->Wait( _mutex, 0 );
      }

      /* if it is exited from wait condition first check whether stop signal is
       * send by anyone */
      if( _isToStop == true )
      {
        _mutex->Unlock();
        continue;
      }

      auto new_msg = _msgQ->Dequeue();

      _mutex->Unlock();

      if( new_msg != nullptr )
      {
        _taskMsgHandler->HandleMsg( new_msg );
        _taskMsgHandler->DeleteMsg( new_msg );
      }
    }

    _mutex->Lock();
    _condition->Signal();
    _mutex->Unlock();
  }
  else
  {
    _taskRoutine->Run( _taskArg );
  }
}

static void TaskBaseFunction( void *arg )
{
  TaskImplWindows *task = (TaskImplWindows *) arg;

  if( task != nullptr )
  {
    task->Run();
  }
}
}
}

#endif