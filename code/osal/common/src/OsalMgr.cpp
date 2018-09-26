/*
 * OsalMgr.cpp
 *
 *  Created on: Jun 27, 2017
 *      Author: psammand
 */

#include "OsalMgr.h"
#include "common/inc/OsalBuilder.h"
#include "common/inc/OsalTimerMgr.h"

namespace ja_iot {
namespace osal {
static OsalBuilder *gs_osal_builder = nullptr;
static OsalTimerMgr* gs_osal_timer_mgr = nullptr;

OsalMgr OsalMgr::_instance{};

OsalMgr * OsalMgr::Inst()
{
  return ( &_instance );
}

Mutex * OsalMgr::AllocMutex() const
{
  if( gs_osal_builder != nullptr )
  {
    return ( gs_osal_builder->AllocateMutex() );
  }

  return ( nullptr );
}

void OsalMgr::FreeMutex( Mutex *mutex )
{
  if( gs_osal_builder != nullptr )
  {
    gs_osal_builder->FreeMutex( mutex );
  }
}

Condition * OsalMgr::AllocCondition()
{
  return ( gs_osal_builder != nullptr ? gs_osal_builder->CreateCondition() : nullptr );
}

void OsalMgr::FreeCondition( Condition *condition )
{
  if( gs_osal_builder != nullptr )
  {
    gs_osal_builder->FreeCondition( condition );
  }
}

Task * OsalMgr::AllocTask()
{
  return ( gs_osal_builder != nullptr ? gs_osal_builder->AllocateTask() : nullptr );
}

void OsalMgr::FreeTask( Task *task )
{
  if( gs_osal_builder != nullptr )
  {
    gs_osal_builder->FreeTask( task );
  }
}

Semaphore * OsalMgr::alloc_semaphore()
{
  return ( gs_osal_builder != nullptr ? gs_osal_builder->alloc_semaphore() : nullptr );
}

void OsalMgr::free_semaphore( Semaphore *semaphore )
{
  if( gs_osal_builder != nullptr )
  {
    gs_osal_builder->free_semaphore( semaphore );
  }
}

OsalTimerMgr * OsalMgr::get_timer_mgr()
{
	return gs_osal_timer_mgr;
}

OsalMgr::OsalMgr ()
{
}

void OsalMgr::Init()
{
  if( is_inited_ == true )
  {
    return;
  }

  gs_osal_builder = OSAL_GetBuilder();

  if( gs_osal_builder != nullptr )
  {
    gs_osal_builder->Init();
  }

	gs_osal_timer_mgr = new OsalTimerMgr{};

	gs_osal_timer_mgr->initialize();

  is_inited_ = true;
}
}
}