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

namespace ja_iot {
namespace network {
#ifdef _OS_WINDOWS_
WindowsPlatformFactory gs_windows_platform_factory{};
#endif

INetworkPlatformFactory *INetworkPlatformFactory::curr_factory_{ nullptr };

INetworkPlatformFactory * INetworkPlatformFactory::GetFactory( NetworkPlatform platform )
{
#ifdef _OS_WINDOWS_

  if( platform == NetworkPlatform::kWindows )
  {
    return ( &gs_windows_platform_factory );
  }

#endif // _OS_WINDOWS_
  return ( nullptr );
}
}
}
