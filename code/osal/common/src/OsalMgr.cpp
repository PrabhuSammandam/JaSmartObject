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
static OsalBuilder *_gsOsalBuilder{ nullptr };
OsalMgr OsalMgr::_instance{};

OsalMgr * OsalMgr::Inst()
{
  return ( &_instance );
}

Mutex * OsalMgr::AllocMutex()
{
  return ( _gsOsalBuilder->AllocateMutex() );
}

void OsalMgr::FreeMutex( Mutex *mutex )
{
  _gsOsalBuilder->FreeMutex( mutex );
}

Condition * OsalMgr::AllocCondition()
{
  return ( _gsOsalBuilder->CreateCondition() );
}

void OsalMgr::FreeCondition( Condition *condition )
{
  _gsOsalBuilder->FreeCondition( condition );
}

Task * OsalMgr::AllocTask()
{
  return ( _gsOsalBuilder->AllocateTask() );
}

void OsalMgr::FreeTask( Task *task )
{
  _gsOsalBuilder->FreeTask( task );
}

OsalMgr::OsalMgr ()
{
}

void OsalMgr::Init()
{
  _gsOsalBuilder = OSAL_GetBuilder();
  _gsOsalBuilder->Init();
}
}
}