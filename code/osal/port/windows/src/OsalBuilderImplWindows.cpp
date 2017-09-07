/*
 * OsalBuilderImplWindows.cpp
 *
 *  Created on: Jun 27, 2017
 *      Author: psammand
 */

#ifdef _OS_WINDOWS_

#include "port/windows/inc/OsalBuilderImplWindows.h"
#include "port/windows/inc/MutexImplWindows.h"
#include "port/windows/inc/ConditionImplWindows.h"
#include <port/windows/inc/TaskImplWindows.h>
#include "port/windows/inc/SemaphoreImplWindows.h"
#include "SimpleList.h"
#include "ScopedMutex.h"

namespace ja_iot {
namespace osal {
constexpr u16 MAX_NO_OF_MUTEX      = 10;
constexpr u16 MAX_NO_OF_CONDITION  = 10;
constexpr u16 MAX_NO_OF_TASKS      = 10;
constexpr u16 MAX_NO_OF_SEMAPHORES = 10;

static OsalBuilderImplWindows _gsOsalBuilderImplWindows{};

static ja_iot::base::SimpleList<MutexImplWindows, MAX_NO_OF_MUTEX> _gsMutexList{};
static ja_iot::base::SimpleList<SemaphoreImplWindows, MAX_NO_OF_SEMAPHORES> _gs_semaphore_list{};
static ja_iot::base::SimpleList<ConditionImplWindows, MAX_NO_OF_CONDITION> _gsConditionList{};
static ja_iot::base::SimpleList<TaskImplWindowsSem, MAX_NO_OF_TASKS> _gsTaskList{};

OsalBuilderImplWindows::OsalBuilderImplWindows ()
{
}

OsalBuilderImplWindows::~OsalBuilderImplWindows ()
{
  _gsMutexList.Free( (MutexImplWindows *) _accessMutex );
  _accessMutex = nullptr;
}

void OsalBuilderImplWindows::Init()
{
  _accessMutex = _gsMutexList.Alloc();

  if( _accessMutex != nullptr )
  {
    _accessMutex->Init();
  }
}

Mutex * OsalBuilderImplWindows::AllocateMutex()
{
  ScopedMutex scoped_mutex( _accessMutex );
  auto        mutex = _gsMutexList.Alloc();

  mutex->Init();

  return ( mutex );
}

void OsalBuilderImplWindows::FreeMutex( Mutex *mutex )
{
  mutex->Uninit();

  if( mutex != nullptr )
  {
    ScopedMutex scoped_mutex( _accessMutex );
    _gsMutexList.Free( (MutexImplWindows *) mutex );
  }
}

Condition * OsalBuilderImplWindows::CreateCondition()
{
  ScopedMutex scoped_mutex( _accessMutex );
  auto        condition = _gsConditionList.Alloc();

  condition->Init();

  return ( condition );
}

void OsalBuilderImplWindows::FreeCondition( Condition *condition )
{
  condition->Uninit();

  if( condition != nullptr )
  {
    ScopedMutex scoped_mutex( _accessMutex );
    _gsConditionList.Free( (ConditionImplWindows *) condition );
  }
}

OsalBuilder* OSAL_GetBuilder()
{
  return ( &_gsOsalBuilderImplWindows );
}

Task * OsalBuilderImplWindows::AllocateTask()
{
  ScopedMutex scoped_mutex( _accessMutex );
  auto        task = _gsTaskList.Alloc();

  return ( task );
}


void OsalBuilderImplWindows::FreeTask( Task *task )
{
  if( task != nullptr )
  {
  ScopedMutex scoped_mutex( _accessMutex );
    _gsTaskList.Free( (TaskImplWindowsSem *) task );
  }
}

Semaphore * OsalBuilderImplWindows::alloc_semaphore()
{
  ScopedMutex scoped_mutex( _accessMutex );
  auto        semaphore = _gs_semaphore_list.Alloc();

  return ( semaphore );
}

void OsalBuilderImplWindows::free_semaphore( Semaphore *semaphore )
{
  if( semaphore != nullptr )
  {
    ScopedMutex scoped_mutex( _accessMutex );
    _gs_semaphore_list.Free( (SemaphoreImplWindows *) semaphore );
  }
}
}
}

#endif // _OS_WINDOWS_
