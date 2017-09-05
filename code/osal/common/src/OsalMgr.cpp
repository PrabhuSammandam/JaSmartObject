/*
 * OsalMgr.cpp
 *
 *  Created on: Jun 27, 2017
 *      Author: psammand
 */

#include "OsalMgr.h"
#include "inc/OsalBuilder.h"

namespace ja_iot {
namespace osal {
static OsalBuilder *gs_osal_builder = nullptr;
OsalMgr OsalMgr::_instance{};

OsalMgr * OsalMgr::Inst()
{
  return ( &_instance );
}

Mutex * OsalMgr::AllocMutex()
{
  return ( gs_osal_builder->AllocateMutex() );
}

void OsalMgr::FreeMutex( Mutex *mutex )
{
  gs_osal_builder->FreeMutex( mutex );
}

Condition * OsalMgr::AllocCondition()
{
  return ( gs_osal_builder->CreateCondition() );
}

void OsalMgr::FreeCondition( Condition *condition )
{
  gs_osal_builder->FreeCondition( condition );
}

Task * OsalMgr::AllocTask()
{
  return ( gs_osal_builder->AllocateTask() );
}

void OsalMgr::FreeTask( Task *task )
{
  gs_osal_builder->FreeTask( task );
}

Semaphore * OsalMgr::alloc_semaphore()
{
  return ( gs_osal_builder->alloc_semaphore() );
}

void OsalMgr::free_semaphore( Semaphore *semaphore )
{
  gs_osal_builder->free_semaphore( semaphore );
}

OsalMgr::OsalMgr ()
{
}

void OsalMgr::Init()
{
  gs_osal_builder = OSAL_GetBuilder();
  gs_osal_builder->Init();
}
}
}
