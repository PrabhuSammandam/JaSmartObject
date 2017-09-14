/*
 * platform_factory.cpp
 *
 *  Created on: Jul 11, 2017
 *      Author: psammand
 */
#include <INetworkPlatformFactory.h>

#ifdef _OS_WINDOWS_
#include "port/windows/inc/WindowsPlatformFactory.h"
#endif

#ifdef _OS_FREERTOS_
#include "port/esp8266/inc/Esp8266PlatformFactory.h"
#endif /* _OS_FREERTOS_ */

namespace ja_iot {
namespace network {
#ifdef _OS_WINDOWS_
WindowsPlatformFactory gs_windows_platform_factory{};
#endif

#ifdef _OS_FREERTOS_
Esp8266PlatformFactory gs_esp8266_platform_factory{};
#endif /* _OS_FREERTOS_ */

INetworkPlatformFactory *INetworkPlatformFactory::curr_factory_{ nullptr };

INetworkPlatformFactory * INetworkPlatformFactory::GetFactory( NetworkPlatform platform )
{
#ifdef _OS_WINDOWS_

  if( platform == NetworkPlatform::kWindows )
  {
    return ( &gs_windows_platform_factory );
  }

#endif // _OS_WINDOWS_

#ifdef _OS_FREERTOS_

  if( platform == NetworkPlatform::kEsp8266 )
  {
    return ( &gs_esp8266_platform_factory );
  }

#endif /* _OS_FREERTOS_ */

  return ( nullptr );
}
}
}