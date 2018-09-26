/*
 * SemaphoreImplWindows.cpp
 *
 *  Created on: Sep 5, 2017
 *      Author: psammand
 */

#ifdef _OS_WINDOWS_

#include "port/windows/inc/SemaphoreImplWindows.h"

namespace ja_iot
{
  namespace osal
  {
    SemaphoreImplWindows::SemaphoreImplWindows()
    {
    }

    SemaphoreImplWindows::~SemaphoreImplWindows()
    {
    }

    OsalError SemaphoreImplWindows::Init(const uint32_t initial_count, const uint32_t access_count)
    {
      semaphore_hndl_ = CreateSemaphore(
        nullptr, // default security attributes
        initial_count, // initial count
        access_count, // maximum count
        nullptr); // unnamed semaphore

      if (semaphore_hndl_ == nullptr)
      {
        return OsalError::ERR;
      }

      return OsalError::OK;
    }

    OsalError SemaphoreImplWindows::Uninit()
    {
      if (semaphore_hndl_ != nullptr)
      {
        CloseHandle(semaphore_hndl_);
      }

      semaphore_hndl_ = INVALID_HANDLE_VALUE;

      return OsalError::OK;
    }

    OsalError SemaphoreImplWindows::Wait()
    {
      const auto dw_wait_result = WaitForSingleObject(semaphore_hndl_, INFINITE);

      if (dw_wait_result == WAIT_OBJECT_0)
      {
        return OsalError::OK;
      }

      return OsalError::ERR;
    }

		OsalError SemaphoreImplWindows::Wait(uint32_t timeout_ms)
		{
			if (timeout_ms == 0)
			{
				return OsalError::ERR;
			}

			const auto dw_wait_result = WaitForSingleObject(semaphore_hndl_, timeout_ms);

			if (dw_wait_result == WAIT_OBJECT_0 || dw_wait_result == WAIT_TIMEOUT)
			{
				return OsalError::OK;
			}

			return OsalError::ERR;
		}

    OsalError SemaphoreImplWindows::Post()
    {
      if (semaphore_hndl_ != nullptr)
      {
        if (!ReleaseSemaphore(semaphore_hndl_, 1, nullptr))
        {
          return OsalError::ERR;
        }

        return OsalError::OK;
      }

      return OsalError::ERR;
    }
  } // namespace osal
} // namespace ja_iot

#endif /* _OS_WINDOWS_ */
