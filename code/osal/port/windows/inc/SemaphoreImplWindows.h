/*
 * SemaphoreImplWindows.h
 *
 *  Created on: Sep 5, 2017
 *      Author: psammand
 */

#pragma once

#include <Sem.h>
#include <windows.h>

#ifdef _OS_WINDOWS_

namespace ja_iot
{
  namespace osal
  {
    class SemaphoreImplWindows : public Semaphore
    {
    public:
      SemaphoreImplWindows();
      virtual ~SemaphoreImplWindows();

      OsalError Init(uint32_t initial_count, uint32_t access_count) override;
      OsalError Uninit() override;

      OsalError Wait() override;
			OsalError Wait(uint32_t timeout_ms)override;
      OsalError Post() override;

    private:
      HANDLE semaphore_hndl_ = INVALID_HANDLE_VALUE;
    };
  } // namespace osal
} // namespace ja_iot

#endif /* _OS_WINDOWS_ */
