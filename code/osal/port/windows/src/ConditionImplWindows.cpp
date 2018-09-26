/*
 * ConditionImplWindows.cpp
 *
 *  Created on: Jun 27, 2017
 *      Author: psammand
 */

#ifdef _OS_WINDOWS_

#include "port/windows/inc/ConditionImplWindows.h"
#include "port/windows/inc/MutexImplWindows.h"

namespace ja_iot
{
  namespace osal
  {
		ConditionImplWindows::ConditionImplWindows() : _conditionVariable{}
    {
    }

    ConditionImplWindows::~ConditionImplWindows()
    {
    }

    OsalError ConditionImplWindows::Init()
    {
      InitializeConditionVariable(&_conditionVariable);
      return OsalError::OK;
    }

    OsalError ConditionImplWindows::Uninit()
    {
      return OsalError::OK;
    }

    OsalError ConditionImplWindows::Wait(Mutex* mutex, const u64 time_out_in_micro_sec)
    {
      auto mutex_impl = static_cast<MutexImplWindows*>(mutex);
      auto ret_value = OsalError::OK;
      DWORD milli = 0;

      if (time_out_in_micro_sec > 0)
      {
        milli = DWORD(time_out_in_micro_sec / 1000);
      }
      else
      {
        milli = INFINITE;
      }
      // Wait for the given time
      if (!SleepConditionVariableCS(&_conditionVariable, mutex_impl->GetMutexImpl(), milli))
      {
        if (GetLastError() == ERROR_TIMEOUT)
        {
          ret_value = OsalError::CONDITION_WAIT_TIMEOUT;
        }
        else
        {
          ret_value = OsalError::CONDITION_WAIT_INVALID;
        }
      }

      return ret_value;
    }

    void ConditionImplWindows::Signal()
    {
      WakeConditionVariable(&_conditionVariable);
    }

    void ConditionImplWindows::Broadcast()
    {
      WakeAllConditionVariable(&_conditionVariable);
    }
  }
}

#endif /*_OS_WINDOWS_*/
