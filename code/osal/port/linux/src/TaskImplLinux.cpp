
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

OsalError TaskImplLinux::InitWithMsgQ( uint8_t *taskName, uint32_t u32_task_priority, uint32_t u32_stack_size, TaskMsgQParam *taskMsgQParam, void *taskArg )
{
  msg_q_mutex_ = OsalMgr::Inst()->AllocMutex();
  // _condition    = OsalMgr::Inst()->AllocCondition();
  this->is_to_stop_     = true;
  this->task_priority_  = u32_task_priority;
  this->stack_size_     = u32_stack_size;
  this->task_paramater_ = taskArg;

  if( taskMsgQParam != nullptr )
  {
    msg_q_            = taskMsgQParam->msgQ;
    task_msg_handler_ = taskMsgQParam->taskMsgHandler;
  }

  is_msg_q_enabled_ = true;

  return ( OsalError::OK );
}

OsalError TaskImplLinux::Init( uint8_t *taskName, uint32_t taskPriority, uint32_t stackSize, ITaskRoutine *taskRoutine, void *taskArg )
{
  this->is_msg_q_enabled_ = false;
  this->task_priority_    = taskPriority;
  this->stack_size_       = stackSize;
  this->task_paramater_   = taskArg;
  this->task_routine_     = taskRoutine;

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


    ret_status = pthread_create( &this->task_handle_, nullptr, task_base_function, task_paramater_ );

    if( ret_status != 0 )
    {
      ScopedMutex scoped_mutex( this->msg_q_mutex_ );
      this->is_to_stop_ = true;

      return ( OsalError::ERR );
    }
  }
  else
  {
    ret_status = pthread_create( &this->task_handle_, nullptr, task_base_function, task_paramater_ );

    if( ret_status != 0 )
    {
      return ( OsalError::ERR );
    }
  }

  return ( OsalError::OK );
}

OsalError TaskImplLinux::Stop()
{
	return ( OsalError::OK );
}

OsalError TaskImplLinux::Destroy()
{
	return ( OsalError::OK );
}

OsalError TaskImplLinux::Wait()
{
	return ( OsalError::OK );
}

OsalError TaskImplLinux::SendMsg( void *msgMem )
{
	return ( OsalError::OK );
}

void TaskImplLinux::Run()
{
}
}
}

#endif /* _OS_LINUX_ */
