/*
 * // * WindowsPlatformFactory.cpp
 *
 *  Created on: Jul 11, 2017
 *      Author: psammand
 */

#ifdef _OS_WINDOWS_

#include <port/windows/inc/platform_factory_win.h>
#include <port/windows/inc/interface_monitor_win.h>
#include <port/windows/inc/ip_adapter_win.h>
#include <port/windows/inc/udp_socket_win.h>

namespace ja_iot {
namespace network {
static InterfaceMonitorImplWindows gs_if_monitor_windows{};

#ifdef _IP_ADAPTER_
static IpAdapterImplWindows gs_ip_adapter_windows{};
#endif /*_IP_ADAPTER_*/

ja_iot::network::WindowsPlatformFactory::WindowsPlatformFactory ()
{
}

IInterfaceMonitor * ja_iot::network::WindowsPlatformFactory::GetInterfaceMonitor()
{
  return ( &gs_if_monitor_windows );
}

IAdapter * ja_iot::network::WindowsPlatformFactory::GetAdapter( AdapterType adapter_type )
{
#ifdef _IP_ADAPTER_

  if( adapter_type == AdapterType::IP )
  {
    return ( &gs_ip_adapter_windows );
  }

#endif

  return ( nullptr );
}

IUdpSocket * WindowsPlatformFactory::AllocSocket()
{
  return ( new UdpSocketImplWindows{} );
}

void WindowsPlatformFactory::FreeSocket( IUdpSocket *socket )
{
  if( socket != nullptr )
  {
    auto win_socket = (UdpSocketImplWindows *) socket;
    delete win_socket;
  }
}
}
}

#endif // #ifdef _OS_WINDOWS_