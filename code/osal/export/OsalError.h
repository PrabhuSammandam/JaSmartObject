/*
 * OsalError.h
 *
 *  Created on: Jun 27, 2017
 *      Author: psammand
 */

#pragma once

namespace ja_iot
{
  namespace osal
  {
    enum class OsalError
    {
      OK = 0,
      ERR,
      OUT_OF_MEMORY,
      INVALID_ARGS,
      NULL_PARAMS,
      CONDITION_WAIT_TIMEOUT,
      CONDITION_WAIT_INVALID
    };
  }
}
