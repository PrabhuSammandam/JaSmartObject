/*
 * Esp8266PlatformFactory.cpp
 *
 *  Created on: Sep 10, 2017
 *      Author: psammand
 */

#ifdef _OS_FREERTOS_

#include <port/esp8266/inc/platform_factory_esp8266.h>
#include <port/esp8266/inc/interface_monitor_esp8266.h>
#include <port/esp8266/inc/ip_adapter_esp8266.h>
#include <port/esp8266/inc/udp_socket_esp8266.h>
#include <base_consts.h>

namespace ja_iot {
namespace network {
using namespace ja_iot::base;
//static InterfaceMonitorImplEsp8266 gs_if_monitor_esp8266{};

#ifdef _IP_ADAPTER_
static IpAdapterImplEsp8266 gs_ip_adapter_esp8266{};
#endif /*_IP_ADAPTER_*/

Esp8266PlatformFactory::Esp8266PlatformFactory ()
{
}

IInterfaceMonitor * Esp8266PlatformFactory::get_interface_monitor()
{
  return nullptr;
}

IAdapter * Esp8266PlatformFactory::get_adapter( uint16_t adapter_type )
{
#ifdef _IP_ADAPTER_

  if( adapter_type == k_adapter_type_ip )
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

void Esp8266PlatformFactory::free_socket( IUdpSocket *socket )
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
