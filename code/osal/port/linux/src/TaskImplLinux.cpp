
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

OsalError TaskImplLinux::PortCreateTask()
{
  int ret_status = pthread_create( &this->task_handle_, nullptr, task_base_function, task_argument_ );

  if( ret_status != 0 )
  {
    return ( OsalError::ERR );
  }

  return ( OsalError::OK );
}

OsalError TaskImplLinux::PortDeleteTask()
{
  this->task_handle_ = 0;

  return ( OsalError::OK );
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