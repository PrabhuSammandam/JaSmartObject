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

static ja_iot::base::SimpleList<MutexImplLinux, MAX_NO_OF_MUTEX> gs_mutex_list{};
static ja_iot::base::SimpleList<SemaphoreImplLinux, MAX_NO_OF_SEMAPHORES> gs_semaphore_list{};
// static ja_iot::base::SimpleList<ConditionImplWindows, MAX_NO_OF_CONDITION> _gsConditionList{};
static ja_iot::base::SimpleList<TaskImplLinux, MAX_NO_OF_TASKS> gs_task_list{};

OsalBuilder* OSAL_GetBuilder()
{
  return ( &gs_osal_builder_impl_linux );
}

OsalBuilderImplLinux::OsalBuilderImplLinux ()
{
}

OsalBuilderImplLinux::~OsalBuilderImplLinux ()
{
  gs_mutex_list.Free( (MutexImplLinux *) access_mutex );
  access_mutex = nullptr;
}

void OsalBuilderImplLinux::Init()
{
  access_mutex = gs_mutex_list.Alloc();

  if( access_mutex != nullptr )
  {
    access_mutex->Init();
  }
}

Mutex * OsalBuilderImplLinux::AllocateMutex()
{
  ScopedMutex scoped_mutex( access_mutex );
  auto        mutex = gs_mutex_list.Alloc();

  mutex->Init();

  return ( mutex );
}

void OsalBuilderImplLinux::FreeMutex( Mutex *mutex )
{
  if( mutex != nullptr )
  {
    mutex->Uninit();
    ScopedMutex scoped_mutex( access_mutex );
    gs_mutex_list.Free( (MutexImplLinux *) mutex );
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
  ScopedMutex scoped_mutex( access_mutex );

  auto        task = gs_task_list.Alloc();

  return ( task );
}


void OsalBuilderImplLinux::FreeTask( Task *task )
{
  if( task != nullptr )
  {
    ScopedMutex scoped_mutex( this->access_mutex );

    gs_task_list.Free( (TaskImplLinux *) task );
  }
}

Semaphore * OsalBuilderImplLinux::alloc_semaphore()
{
  ScopedMutex scoped_mutex( access_mutex );
  auto        new_semaphore = gs_semaphore_list.Alloc();

  return ( new_semaphore );
}

void OsalBuilderImplLinux::free_semaphore( Semaphore *semaphore )
{
  if( semaphore )
  {
    ScopedMutex scoped_mutex( access_mutex );
    gs_semaphore_list.Free( (SemaphoreImplLinux *) semaphore );
  }
}
}  // namespace osal
}  // namespace ja_iot

#endif /* _OS_LINUX_ */