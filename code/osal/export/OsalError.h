/*
 * OsalError.h
 *
 *  Created on: Jun 27, 2017
 *      Author: psammand
 */

#ifndef OSAL_EXPORT_OSALERROR_H_
#define OSAL_EXPORT_OSALERROR_H_

namespace ja_iot {
namespace osal {
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

#endif /* OSAL_EXPORT_OSALERROR_H_ */