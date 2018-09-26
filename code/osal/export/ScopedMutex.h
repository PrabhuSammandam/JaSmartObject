/*
 * ScopedMutex.h
 *
 *  Created on: Jul 1, 2017
 *      Author: psammand
 */

#pragma once

#include "Mutex.h"

namespace ja_iot
{
  namespace osal
  {
    class ScopedMutex
    {
      ScopedMutex(const ScopedMutex&) = delete;
      ScopedMutex& operator =(const ScopedMutex&) = delete;
      Mutex* _mutex;

    public:

      ScopedMutex(Mutex const* mutex) : _mutex{const_cast<Mutex *>(mutex)}
      {
        if (_mutex != nullptr)
        {
          _mutex->Lock();
        }
      }

      ~ScopedMutex()
      {
        if (_mutex != nullptr)
        {
          _mutex->Unlock();
        }
      }
    };
  }
}
