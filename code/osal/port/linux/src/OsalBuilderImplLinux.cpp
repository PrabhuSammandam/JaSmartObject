/*
 * OsalBuilderImplLinux.cpp
 *
 *  Created on: 05-Sep-2017
 *      Author: prabhu
 */

#ifdef _OS_LINUX_

#include <OsalError.h>
#include <port/linux/inc/TaskImplLinux.h>
#include <port/linux/inc/MutexImplLinux.h>
#include <port/linux/inc/SemaphoreImplLinux.h>
#include <port/linux/inc/OsalBuilderImplLinux.h>
#include <ScopedMutex.h>
#include <SimpleList.h>
#include <cstdint>

namespace ja_iot {
namespace osal {
constexpr uint16_t MAX_NO_OF_MUTEX      = 10;
constexpr uint16_t MAX_NO_OF_CONDITION  = 10;
constexpr uint16_t MAX_NO_OF_TASKS      = 10;
constexpr uint16_t MAX_NO_OF_SEMAPHORES = 10;

static OsalBuilderImplLinux gs_osal_builder_impl_linux{};

OsalBuilder* OSAL_GetBuilder()
{
  return ( &gs_osal_builder_impl_linux );
}

OsalBuilderImplLinux::OsalBuilderImplLinux ()
{
}

OsalBuilderImplLinux::~OsalBuilderImplLinux ()
{
}

void OsalBuilderImplLinux::Init()
{
}

Mutex * OsalBuilderImplLinux::AllocateMutex()
{
  auto        mutex = new MutexImplLinux{};
  mutex->Init();

  return ( mutex );
}

void OsalBuilderImplLinux::FreeMutex( Mutex *mutex )
{
  if( mutex != nullptr )
  {
    mutex->Uninit();
    delete mutex;
  }
}

Condition * OsalBuilderImplLinux::CreateCondition()
{
  return ( nullptr );
}

void OsalBuilderImplLinux::FreeCondition( Condition *condition )
{
}

Task * OsalBuilderImplLinux::AllocateTask()
{
  return ( new TaskImplLinux{} );
}


void OsalBuilderImplLinux::FreeTask( Task *task )
{
  if( task != nullptr )
  {
	  delete task;
  }
}

Semaphore * OsalBuilderImplLinux::alloc_semaphore()
{
  return ( new SemaphoreImplLinux{} );
}

void OsalBuilderImplLinux::free_semaphore( Semaphore *semaphore )
{
  if( semaphore )
  {
	  delete semaphore;
  }
}
}  // namespace osal
}  // namespace ja_iot

#endif /* _OS_LINUX_ */
