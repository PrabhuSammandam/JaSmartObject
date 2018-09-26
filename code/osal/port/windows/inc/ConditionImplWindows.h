/*
 * ConditionImplWindows.h
 *
 *  Created on: Jun 27, 2017
 *      Author: psammand
 */

#pragma once

#ifdef _OS_WINDOWS_

#include "PrimitiveDataTypes.h"
#include "Condition.h"
#include "OsalError.h"
#include <windows.h>

namespace ja_iot
{
  namespace osal
  {
    class ConditionImplWindows : public Condition
    {
    public:
      ConditionImplWindows();
      virtual ~ConditionImplWindows();

      OsalError Init() override;
      OsalError Uninit() override;

      OsalError Wait(Mutex* mutex, u64 time_out_in_micro_sec = 0) override;

      void Signal() override;
      void Broadcast() override;

    private:
      CONDITION_VARIABLE _conditionVariable;
    };
  }
}

#endif // _OS_WINDOWS_
