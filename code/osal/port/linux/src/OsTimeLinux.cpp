/*
 * OsTimeLinux.cpp
 *
 *  Created on: Jan 18, 2018
 *      Author: psammand
 *
 */

#ifdef _OS_LINUX_

#include <sys/time.h>
#include "OsTime.h"

namespace ja_iot {
namespace osal {
uint32_t OsTime::get_current_time_us()
{
  struct timeval currentTime {};

  gettimeofday( &currentTime, nullptr );
  return ( currentTime.tv_sec * (int) 1e6 + currentTime.tv_usec );
}
}
}

#endif /* _OS_LINUX_ */