
#ifdef _OS_LINUX_

#include "port/linux/inc/TaskImplLinux.h"
#include "OsalMgr.h"
#include "ScopedMutex.h"

namespace ja_iot {
namespace osal {
static void* task_base_function( void *arg );

TaskImplLinux::TaskImplLinux ()
{
}

TaskImplLinux::~TaskImplLinux ()
{
}

OsalError TaskImplLinux::InitWithMsgQ( uint8_t *task_name, uint32_t u32_task_priority, uint32_t u32_stack_size, TaskMsgQParam *task_msg_q_parameter, void *task_argument )
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
  this->task_priority_ = u32_task_priority;
  this->stack_size_    = u32_stack_size;
  this->task_argument_ = task_argument;

  is_msg_q_enabled_ = true;

  return ( OsalError::OK );

failed:

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

OsalError TaskImplLinux::Init( uint8_t *task_name, uint32_t task_priority, uint32_t stack_size, ITaskRoutine *task_routine, void *task_argument )
{
  this->is_msg_q_enabled_ = false;
  this->task_priority_    = task_priority;
  this->stack_size_       = stack_size;
  this->task_argument_    = task_argument;
  this->task_routine_     = task_routine;

  return ( OsalError::OK );
}

OsalError TaskImplLinux::Start()
{
  int ret_status;

  if( this->is_msg_q_enabled_ )
  {
    if( this->is_to_stop_ == false )
    {
      /* task already running*/
      return ( OsalError::OK );
    }

    {
      ScopedMutex scoped_mutex( this->msg_q_mutex_ );
      this->is_to_stop_ = false;
    }


    ret_status = pthread_create( &this->task_handle_, nullptr, task_base_function, task_argument_ );

    if( ret_status != 0 )
    {
      ScopedMutex scoped_mutex( this->msg_q_mutex_ );
      this->is_to_stop_ = true;

      return ( OsalError::ERR );
    }
  }
  else
  {
    ret_status = pthread_create( &this->task_handle_, nullptr, task_base_function, task_argument_ );

    if( ret_status != 0 )
    {
      return ( OsalError::ERR );
    }
  }

  return ( OsalError::OK );
}

OsalError TaskImplLinux::Stop()
{
  if( !is_to_stop_ )
  {
    ScopedMutex scoped_mutex( msg_q_mutex_ );
    is_to_stop_ = true;
  }

  return ( OsalError::OK );
}

OsalError TaskImplLinux::Destroy()
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

  task_handle_ = 0;

  return ( OsalError::OK );
}

OsalError TaskImplLinux::Wait()
{
  int ret_status;

  ret_status = pthread_join( task_handle_, NULL );

  if( ret_status != 0 )
  {
    return ( OsalError::ERR );
  }

  return ( OsalError::OK );
}

OsalError TaskImplLinux::SendMsg( void *msgMem )
{
  return ( OsalError::OK );
}

void TaskImplLinux::Run()
{
  if( is_msg_q_enabled_ )
  {
  }
  else
  {
    task_routine_->Run( task_argument_ );
  }
}

void* task_base_function( void *arg )
{
  TaskImplLinux *task_impl_linux = (TaskImplLinux *) arg;

  if( task_impl_linux != nullptr )
  {
    task_impl_linux->Run();
  }

  return ( nullptr );
}
}
}

#endif /* _OS_LINUX_ */
