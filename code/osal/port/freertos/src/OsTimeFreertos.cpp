#ifdef _OS_FREERTOS_

#include <esp_system.h>
#include "OsTime.h"

namespace ja_iot {
  namespace osal {
    uint32_t OsTime::get_current_time_us()
    {
      return ( system_get_time() );
    }
  }
}

#endif /* _OS_FREERTOS_ */
