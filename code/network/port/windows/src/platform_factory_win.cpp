/*
 * // * WindowsPlatformFactory.cpp
 *
 *  Created on: Jul 11, 2017
 *      Author: psammand
 */

#ifdef _OS_WINDOWS_

#include <base_consts.h>
#include <port/windows/inc/platform_factory_win.h>
#include <port/windows/inc/interface_monitor_win.h>
#include <port/windows/inc/ip_adapter_win.h>
#include <port/windows/inc/udp_socket_win.h>

namespace ja_iot {
namespace network {
using namespace base;

static InterfaceMonitorImplWindows gs_if_monitor_windows{};

#ifdef _IP_ADAPTER_
static IpAdapterImplWindows gs_ip_adapter_windows{};
#endif /*_IP_ADAPTER_*/
WindowsPlatformFactory::WindowsPlatformFactory ()
{
}

IInterfaceMonitor * WindowsPlatformFactory::get_interface_monitor()
{
  return ( &gs_if_monitor_windows );
}

IAdapter * WindowsPlatformFactory::get_adapter( uint16_t u16_adapter_type )
{
#ifdef _IP_ADAPTER_

  if( u16_adapter_type == k_adapter_type_ip )
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

void WindowsPlatformFactory::free_socket( IUdpSocket *pcz_socket )
{
  if( pcz_socket != nullptr )
  {
    const auto pcz_win_socket = static_cast<UdpSocketImplWindows *>( pcz_socket );
    delete pcz_win_socket;
  }
}
}
}

#endif // #ifdef _OS_WINDOWS_