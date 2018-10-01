/*
 * platform_factory.cpp
 *
 *  Created on: Jul 11, 2017
 *      Author: psammand
 */
#include <i_nwk_platform_factory.h>

#ifdef _OS_WINDOWS_
#include <port/windows/inc/platform_factory_win.h>
#endif

#ifdef _OS_FREERTOS_
#include "port/esp8266/inc/platform_factory_esp8266.h"
#endif /* _OS_FREERTOS_ */

#ifdef _OS_LINUX_
#include "port/linux/inc/platform_factory_lnx.h"
#endif

namespace ja_iot {
  namespace network {
#ifdef _OS_WINDOWS_
    WindowsPlatformFactory gs_windows_platform_factory {};
#endif /* _OS_WINDOWS_ */

#ifdef _OS_FREERTOS_
    Esp8266PlatformFactory gs_esp8266_platform_factory {};
#endif /* _OS_FREERTOS_ */

#ifdef _OS_LINUX_
    LinuxPlatformFactory     gs_linux_platform_factory {};
#endif /* _OS_LINUX_ */

    INetworkPlatformFactory *INetworkPlatformFactory::curr_factory_ { nullptr };

    INetworkPlatformFactory * INetworkPlatformFactory::create_factory( NetworkPlatform e_network_platform )
    {
#ifdef _OS_WINDOWS_

      if( e_network_platform == NetworkPlatform::kWindows )
      {
        return ( &gs_windows_platform_factory );
      }

#endif // _OS_WINDOWS_

#ifdef _OS_FREERTOS_

      if( e_network_platform == NetworkPlatform::kEsp8266 )
      {
        return ( &gs_esp8266_platform_factory );
      }

#endif /* _OS_FREERTOS_ */

#ifdef _OS_LINUX_

      if( e_network_platform == NetworkPlatform::kLinux )
      {
        return ( &gs_linux_platform_factory );
      }

#endif /* _OS_LINUX_ */


      return ( nullptr );
    }

    INetworkPlatformFactory * INetworkPlatformFactory::create_set_factory( NetworkPlatform platform )
    {
      auto network_factory = create_factory( platform );

      if( network_factory != nullptr )
      {
        set_curr_factory( network_factory );
      }

      return ( network_factory );
    }
  }
}
