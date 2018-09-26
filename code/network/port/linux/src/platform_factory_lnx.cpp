/*
 * LinuxPlatformFactory.cpp
 *
 *  Created on: 17-Sep-2017
 *      Author: prabhu
 */

#ifdef _OS_LINUX_
#include "base_consts.h"
#include "port/linux/inc/platform_factory_lnx.h"
#include "port/linux/inc/ip_adapter_lnx.h"
#include "port/linux/inc/udp_socket_lnx.h"

namespace ja_iot {
namespace network {
using namespace ja_iot::base;

#ifdef _IP_ADAPTER_
static IpAdapterImplLinux gs_ip_adapter_linux{};
#endif /*_IP_ADAPTER_*/
ja_iot::network::LinuxPlatformFactory::LinuxPlatformFactory ()
{
}

IInterfaceMonitor * ja_iot::network::LinuxPlatformFactory::get_interface_monitor()
{
  return ( nullptr );
}

IAdapter * ja_iot::network::LinuxPlatformFactory::get_adapter( uint16_t adapter_type )
{
#ifdef _IP_ADAPTER_

  if( adapter_type == k_adapter_type_ip )
  {
    return ( &gs_ip_adapter_linux );
  }

#endif
  return nullptr;
}

IUdpSocket * ja_iot::network::LinuxPlatformFactory::AllocSocket()
{
  return ( new UdpSocketImplLinux{} );
}

void ja_iot::network::LinuxPlatformFactory::free_socket( IUdpSocket *socket )
{
  if( socket != nullptr )
  {
    const auto win_socket = static_cast<UdpSocketImplLinux *>( socket );
    delete win_socket;
  }
}
}  // namespace network
}  // namespace ja_iot

#endif /* _OS_LINUX_ */
