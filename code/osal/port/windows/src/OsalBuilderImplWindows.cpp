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

static base::SimpleList<MutexImplWindows, MAX_NO_OF_MUTEX> _gsMutexList{};
static base::SimpleList<SemaphoreImplWindows, MAX_NO_OF_SEMAPHORES> _gs_semaphore_list{};
static base::SimpleList<ConditionImplWindows, MAX_NO_OF_CONDITION> _gsConditionList{};
static base::SimpleList<TaskImplWindows, MAX_NO_OF_TASKS> _gsTaskList{};
OsalBuilderImplWindows::OsalBuilderImplWindows ()
{
}
OsalBuilderImplWindows::~OsalBuilderImplWindows ()
{
  _gsMutexList.Free( static_cast<MutexImplWindows *>( _access_mutex ) );
  _access_mutex = nullptr;
}

void OsalBuilderImplWindows::Init()
{
  _access_mutex = _gsMutexList.Alloc();

  if( _access_mutex != nullptr )
  {
    _access_mutex->Init();
  }
}

Mutex * OsalBuilderImplWindows::AllocateMutex()
{
  ScopedMutex scoped_mutex( _access_mutex );
  auto        mutex = _gsMutexList.Alloc();

  mutex->Init();

  return ( mutex );
}

void OsalBuilderImplWindows::FreeMutex( Mutex *mutex )
{
  if( mutex != nullptr )
  {
    mutex->Uninit();
    ScopedMutex scoped_mutex( _access_mutex );
    _gsMutexList.Free( static_cast<MutexImplWindows *>( mutex ) );
  }
}

Condition * OsalBuilderImplWindows::CreateCondition()
{
  ScopedMutex scoped_mutex( _access_mutex );
  auto        condition = _gsConditionList.Alloc();

  condition->Init();

  return ( condition );
}

void OsalBuilderImplWindows::FreeCondition( Condition *condition )
{
  if( condition != nullptr )
  {
    condition->Uninit();
    ScopedMutex scoped_mutex( _access_mutex );
    _gsConditionList.Free( static_cast<ConditionImplWindows *>( condition ) );
  }
}

OsalBuilder* OSAL_GetBuilder()
{
  return ( &_gsOsalBuilderImplWindows );
}

Task * OsalBuilderImplWindows::AllocateTask()
{
  ScopedMutex scoped_mutex( _access_mutex );
  const auto  task = _gsTaskList.Alloc();

  return ( task );
}


void OsalBuilderImplWindows::FreeTask( Task *task )
{
  if( task != nullptr )
  {
    ScopedMutex scoped_mutex( _access_mutex );
    _gsTaskList.Free( static_cast<TaskImplWindows *>( task ) );
  }
}

Semaphore * OsalBuilderImplWindows::alloc_semaphore()
{
  ScopedMutex scoped_mutex( _access_mutex );
  const auto  semaphore = _gs_semaphore_list.Alloc();

  return ( semaphore );
}

void OsalBuilderImplWindows::free_semaphore( Semaphore *semaphore )
{
  if( semaphore != nullptr )
  {
    ScopedMutex scoped_mutex( _access_mutex );
    _gs_semaphore_list.Free( static_cast<SemaphoreImplWindows *>( semaphore ) );
  }
}
}
}

#endif // _OS_WINDOWS_