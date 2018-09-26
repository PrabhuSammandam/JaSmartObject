/*
 * MutexImplWindows.h
 *
 *  Created on: Jun 27, 2017
 *      Author: psammand
 */

#pragma once

#ifdef _OS_WINDOWS_

#include <windows.h>
#include "Mutex.h"

namespace ja_iot
{
  namespace osal
  {
    class MutexImplWindows : public Mutex
    {
    public:

      MutexImplWindows();

      virtual ~MutexImplWindows();

      OsalError Init() override;
      OsalError Lock() override;
      OsalError Unlock() override;
      OsalError Uninit() override;

      CRITICAL_SECTION* GetMutexImpl()
      {
        return &_criticalSection;
      }

    private:
      CRITICAL_SECTION _criticalSection{};
    };
  }
}
#endif /*_OS_WINDOWS_ */
