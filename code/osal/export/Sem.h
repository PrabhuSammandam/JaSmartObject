/*
 * Semaphore.h
 *
 *  Created on: 05-Sep-2017
 *      Author: prabhu
 */

#pragma once

#include <cstdint>
#include "OsalError.h"

namespace ja_iot
{
  namespace osal
  {
    class Semaphore
    {
    public:
      Semaphore()
      {
      }

      virtual ~Semaphore()
      {
      }

      virtual OsalError Init(uint32_t initial_count, uint32_t access_count) = 0;
      virtual OsalError Uninit() = 0;

      virtual OsalError Wait() = 0;
			virtual OsalError Wait(uint32_t timeout_ms) = 0;
      virtual OsalError Post() = 0;
    };
  }
}
