/*
 * MutexImplWindows.cpp
 *
 *  Created on: Jun 27, 2017
 *      Author: psammand
 */

#ifdef _OS_WINDOWS_

#include "windows/inc/MutexImplWindows.h"

namespace ja_iot {
namespace osal {
MutexImplWindows::MutexImplWindows ()
{
}

MutexImplWindows::~MutexImplWindows ()
{
}

OsalError MutexImplWindows::Init()
{
  InitializeCriticalSection( &_criticalSection );
  return ( OsalError::OK );
}

OsalError MutexImplWindows::Lock()
{
  EnterCriticalSection( &_criticalSection );
  return ( OsalError::OK );
}

OsalError MutexImplWindows::Unlock()
{
  LeaveCriticalSection( &_criticalSection );
  return ( OsalError::OK );
}

OsalError MutexImplWindows::Uninit()
{
  DeleteCriticalSection( &_criticalSection );
  return ( OsalError::OK );
}
}
}

#endif /*#ifdef _OS_WINDOWS_*/