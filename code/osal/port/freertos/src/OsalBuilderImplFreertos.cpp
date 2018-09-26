/*
 * OsalBuilderImplFreertos.cpp
 *
 *  Created on: Sep 8, 2017
 *      Author: psammand
 */
#ifdef _OS_FREERTOS_

#include <port/freertos/inc/TaskImplFreertos.h>
#include <port/freertos/inc/MutexImplFreertos.h>
#include <port/freertos/inc/OsalBuilderImplFreertos.h>
#include <port/freertos/inc/SemaphoreImplFreertos.h>
#include <Task.h>
#include <OsalError.h>
#include <ScopedMutex.h>
#include <SimpleList.h>
#include <cstdint>
#include <forward_list>
#include <algorithm>

namespace ja_iot {
namespace osal {
constexpr uint16_t MAX_NO_OF_MUTEX      = 10;
constexpr uint16_t MAX_NO_OF_CONDITION  = 10;
constexpr uint16_t MAX_NO_OF_TASKS      = 10;
constexpr uint16_t MAX_NO_OF_SEMAPHORES = 10;

static OsalBuilderImplFreertos gs_osal_builder_impl_freertos{};

//static ja_iot::base::SimpleList<MutexImplFreertos, MAX_NO_OF_MUTEX> gs_mutex_list{};
//static ja_iot::base::SimpleList<SemaphoreImplFreertos, MAX_NO_OF_SEMAPHORES> gs_semaphore_list{};
//static ja_iot::base::SimpleList<TaskImplFreertos, MAX_NO_OF_TASKS> gs_task_list{};

OsalBuilder* OSAL_GetBuilder()
{
  return ( &gs_osal_builder_impl_freertos );
}

OsalBuilderImplFreertos::OsalBuilderImplFreertos ()
{
}

OsalBuilderImplFreertos::~OsalBuilderImplFreertos ()
{
}

void OsalBuilderImplFreertos::Init()
{
//    access_mutex.Init();
}

Mutex * OsalBuilderImplFreertos::AllocateMutex()
{
//  ScopedMutex scoped_mutex( &access_mutex );
//  auto        mutex = gs_mutex_list.Alloc();

  auto        mutex = new MutexImplFreertos{};
  mutex->Init();

  return ( mutex );
}

void OsalBuilderImplFreertos::FreeMutex( Mutex *mutex )
{
  if( mutex != nullptr )
  {
    mutex->Uninit();
    delete mutex;
//    ScopedMutex scoped_mutex( &access_mutex );
//    gs_mutex_list.Free( (MutexImplFreertos *) mutex );
  }
}

Condition * OsalBuilderImplFreertos::CreateCondition()
{
  return ( nullptr );
}

void OsalBuilderImplFreertos::FreeCondition( Condition *condition )
{
}

Task * OsalBuilderImplFreertos::AllocateTask()
{
//  ScopedMutex scoped_mutex( &access_mutex );
//  auto        task = gs_task_list.Alloc();
//  return ( task );
	return new TaskImplFreertos{};
}


void OsalBuilderImplFreertos::FreeTask( Task *task )
{
  if( task != nullptr )
  {
//    ScopedMutex scoped_mutex( &this->access_mutex );
//    gs_task_list.Free( (TaskImplFreertos *) task );
	  delete task;
  }
}

Semaphore * OsalBuilderImplFreertos::alloc_semaphore()
{
//  ScopedMutex scoped_mutex( &access_mutex );
//  auto        new_semaphore = gs_semaphore_list.Alloc();
//  return ( new_semaphore );
	return new SemaphoreImplFreertos{};
}

void OsalBuilderImplFreertos::free_semaphore( Semaphore *semaphore )
{
  if( semaphore )
  {
//    ScopedMutex scoped_mutex( &access_mutex );
//    gs_semaphore_list.Free( (SemaphoreImplFreertos*) semaphore );
	  delete semaphore;
  }
}
}  // namespace osal
}  // namespace ja_iot

#endif /* _OS_FREERTOS_ */
