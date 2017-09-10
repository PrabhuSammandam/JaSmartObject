/*
 * TaskBase.cpp
 *
 *  Created on: Sep 7, 2017
 *      Author: psammand
 */

#include <stdio.h>
#include <common/inc/TaskBase.h>
#include <MsgQ.h>
#include <Mutex.h>
#include <OsalError.h>
#include <OsalMgr.h>
#include <ScopedMutex.h>
#include <Sem.h>

#ifdef _DEBUG_
#define dbg( format, ... ) printf( format "\n", ## __VA_ARGS__ )
#else
#define dbg( format, ... )
#endif

namespace ja_iot {
namespace osal {
OsalError TaskBase::InitWithMsgQ( uint8_t *taskName, uint32_t taskPriority, uint32_t stackSize, TaskMsgQParam *task_msg_q_parameter, void *taskArg )
{
  bool is_msg_q_sem_inited = false;

  msg_q_mutex_ = OsalMgr::Inst()->AllocMutex();

  if( msg_q_mutex_ == nullptr )
  {
    goto failed;
  }

  if( ( task_msg_q_parameter == nullptr ) || ( task_msg_q_parameter->msgQ == nullptr ) || ( task_msg_q_parameter->taskMsgHandler == nullptr ) )
  {
    goto failed;
  }

  if( task_msg_q_parameter->msgQ->GetCapacity() <= 0 )
  {
    goto failed;
  }

  msg_q_semaphore_ = OsalMgr::Inst()->alloc_semaphore();

  if( msg_q_semaphore_ == nullptr )
  {
    goto failed;
  }

  msg_q_            = task_msg_q_parameter->msgQ;
  task_msg_handler_ = task_msg_q_parameter->taskMsgHandler;

  msg_q_semaphore_->Init( 0, task_msg_q_parameter->msgQ->GetCapacity() );
  is_msg_q_sem_inited = true;

  this->is_to_stop_    = true;
  this->task_priority_ = taskPriority;
  this->stack_size_    = stackSize;
  this->task_argument_ = taskArg;

  is_msg_q_enabled_ = true;

  return ( OsalError::OK );

failed:

  dbg( "%s=> Failed to init task\n", __FUNCTION__ );

  if( msg_q_semaphore_ != nullptr )
  {
    if( is_msg_q_sem_inited == true )
    {
      msg_q_semaphore_->Uninit();
    }

    OsalMgr::Inst()->free_semaphore( msg_q_semaphore_ );
    msg_q_semaphore_ = nullptr;
  }

  if( msg_q_mutex_ != nullptr )
  {
    OsalMgr::Inst()->FreeMutex( msg_q_mutex_ );
    msg_q_mutex_ = nullptr;
  }

  return ( OsalError::ERR );
}

OsalError TaskBase::Init( uint8_t *taskName, uint32_t taskPriority, uint32_t stackSize, ITaskRoutine *taskRoutine, void *taskArg )
{
  is_msg_q_enabled_ = false;
  task_priority_    = taskPriority;
  stack_size_       = stackSize;
  task_argument_    = taskArg;
  task_routine_     = taskRoutine;

  return ( OsalError::OK );
}

OsalError TaskBase::Start()
{
  OsalError status = OsalError::OK;

  if( ( is_msg_q_enabled_ == true ) && ( is_to_stop_ == true ) )
  {
    dbg( "%s=>Creating msg q task\n", __FUNCTION__ );
    ScopedMutex scoped_mutex( msg_q_mutex_ );
    is_to_stop_ = false;
    status      = PortCreateTask();

    if( status != OsalError::OK )
    {
      is_to_stop_ = true;
    }

    dbg( "%s=>Created task succesfully\n", __FUNCTION__ );
  }
  else
  {
    status = PortCreateTask();
  }

  return ( status );
}

OsalError TaskBase::Stop()
{
  /* get the lock to modify the shared variable */
  ScopedMutex scoped_mutex( msg_q_mutex_ );

  if( !is_to_stop_ )
  {
    is_to_stop_ = true;

    /* inform the thread that there are some signals sent */
    msg_q_semaphore_->Post();
  }

  return ( OsalError::OK );
}

OsalError TaskBase::Destroy()
{
  if( !is_to_stop_ )
  {
    return ( OsalError::ERR );
  }

  if( msg_q_ != nullptr )
  {
    if( msg_q_mutex_ != nullptr )
    {
      msg_q_mutex_->Lock();
    }

    while( msg_q_->IsEmpty() == false )
    {
      auto msg = msg_q_->Dequeue();

      if( msg != nullptr )
      {
        task_msg_handler_->DeleteMsg( msg );
      }
    }

    msg_q_ = nullptr;

    if( msg_q_mutex_ != nullptr )
    {
      msg_q_mutex_->Unlock();
    }
  }

  if( msg_q_mutex_ != nullptr )
  {
    OsalMgr::Inst()->FreeMutex( msg_q_mutex_ );
  }

  msg_q_mutex_ = nullptr;

  if( msg_q_semaphore_ != nullptr )
  {
    msg_q_semaphore_->Uninit();
    OsalMgr::Inst()->free_semaphore( msg_q_semaphore_ );
  }

  msg_q_semaphore_ = nullptr;

  PortDeleteTask();

  return ( OsalError::OK );
}

OsalError TaskBase::SendMsg( void *msgMem )
{
  if( ( msgMem == nullptr ) || ( msg_q_ == nullptr ) )
  {
    return ( OsalError::INVALID_ARGS );
  }

  dbg( "%s=>ENTER\n", __FUNCTION__ );

  ScopedMutex scoped_mutex( msg_q_mutex_ );

  if( msg_q_->Enqueue( msgMem ) == true )
  {
    msg_q_semaphore_->Post();

    dbg( "%s=>Msg sent successfully, msg_q_sem %p\n", __FUNCTION__, msg_q_semaphore_ );
    return ( OsalError::OK );
  }

  return ( OsalError::ERR );
}


void TaskBase::Run()
{
  if( is_msg_q_enabled_ )
  {
    while( is_to_stop_ == false )
    {
      dbg( "%s=>**********************************going to wait******************\n", __FUNCTION__ );
      msg_q_semaphore_->Wait();

      msg_q_mutex_->Lock();

      /* if it is exited from wait condition, first check whether stop signal is
       * send by anyone */
      if( is_to_stop_ == true )
      {
        msg_q_mutex_->Unlock();
        continue;
      }

      auto new_msg = msg_q_->Dequeue();

      msg_q_mutex_->Unlock();

      if( new_msg != nullptr )
      {
        dbg( "%s=>Calling task handler\n", __FUNCTION__ );
        task_msg_handler_->HandleMsg( new_msg );
        task_msg_handler_->DeleteMsg( new_msg );
      }
    }
  }
  else
  {
    task_routine_->Run( task_argument_ );
  }
}
}
}