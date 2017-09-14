/*
 * Esp8266PlatformFactory.cpp
 *
 *  Created on: Sep 10, 2017
 *      Author: psammand
 */

#ifdef _OS_FREERTOS_

#include <port/esp8266/inc/Esp8266PlatformFactory.h>
#include <port/esp8266/inc/InterfaceMonitorImplEsp8266.h>
#include <port/esp8266/inc/IpAdapterImplEsp8266.h>
#include <port/esp8266/inc/UdpSocketImplEsp8266.h>
#include <TransportType.h>

namespace ja_iot {
namespace network {
//static InterfaceMonitorImplEsp8266 gs_if_monitor_esp8266{};

#ifdef _IP_ADAPTER_
static IpAdapterImplEsp8266 gs_ip_adapter_esp8266{};
#endif /*_IP_ADAPTER_*/

Esp8266PlatformFactory::Esp8266PlatformFactory ()
{
}

IInterfaceMonitor * Esp8266PlatformFactory::GetInterfaceMonitor()
{
  return nullptr;
}

IAdapter * Esp8266PlatformFactory::GetAdapter( AdapterType adapter_type )
{
#ifdef _IP_ADAPTER_

  if( adapter_type == AdapterType::IP )
  {
    return ( &gs_ip_adapter_esp8266 );
  }

#endif

  return ( nullptr );
}

IUdpSocket * Esp8266PlatformFactory::AllocSocket()
{
  return ( new UdpSocketImplEsp8266{} );
}

void Esp8266PlatformFactory::FreeSocket( IUdpSocket *socket )
{
  if( socket != nullptr )
  {
    auto esp8266_scoket = (UdpSocketImplEsp8266 *) socket;
    delete esp8266_scoket;
  }
}
}
}
#endif /* _OS_FREERTOS_ */
