/*
 * OsalBuilderImplWindows.cpp
 *
 *  Created on: Jun 27, 2017
 *      Author: psammand
 */

#include "windows/inc/OsalBuilderImplWindows.h"
#include "windows/inc/MutexImplWindows.h"
#include "windows/inc/ConditionImplWindows.h"
#include "windows/inc/TaskImplWindows.h"
#include "SimpleList.h"
#include "ScopedMutex.h"

namespace ja_iot {
namespace osal {
constexpr u16 MAX_NO_OF_MUTEX     = 10;
constexpr u16 MAX_NO_OF_CONDITION = 10;
constexpr u16 MAX_NO_OF_TASKS     = 10;

static OsalBuilderImplWindows _gsOsalBuilderImplWindows{};

static ja_iot::base::SimpleList<MutexImplWindows, MAX_NO_OF_MUTEX> _gsMutexList{};
static ja_iot::base::SimpleList<ConditionImplWindows, MAX_NO_OF_CONDITION> _gsConditionList{};
static ja_iot::base::SimpleList<TaskImplWindows, MAX_NO_OF_TASKS> _gsTaskList{};

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
  _accessMutex->Init();
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

  ScopedMutex scoped_mutex( _accessMutex );
  _gsMutexList.Free( (MutexImplWindows *) mutex );
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

  ScopedMutex scoped_mutex( _accessMutex );
  _gsConditionList.Free( (ConditionImplWindows *) condition );
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
  ScopedMutex scoped_mutex( _accessMutex );

  _gsTaskList.Free( (TaskImplWindows *) task );
}
}
}