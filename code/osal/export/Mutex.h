/*
 * Mutex.h
 *
 *  Created on: Jun 27, 2017
 *      Author: psammand
 */

#pragma once

#include "OsalError.h"

namespace ja_iot
{
  namespace osal
  {
    class Mutex
    {
    public:

      Mutex()
      {
      }

      virtual ~Mutex()
      {
      }

      virtual OsalError Init() = 0;
      virtual OsalError Uninit() = 0;

      virtual OsalError Lock() = 0;
      virtual OsalError Unlock() = 0;
    };
  }
}
