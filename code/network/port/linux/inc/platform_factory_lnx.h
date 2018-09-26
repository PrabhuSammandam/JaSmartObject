/*
 * LinuxPlatformFactory.h
 *
 *  Created on: 17-Sep-2017
 *      Author: prabhu
 */

#ifdef _OS_LINUX_

#pragma once

#include <i_nwk_platform_factory.h>

namespace ja_iot {
namespace network {
class LinuxPlatformFactory : public INetworkPlatformFactory
{
  public:
    LinuxPlatformFactory ();

    IInterfaceMonitor* get_interface_monitor()                  override;
    IAdapter*          get_adapter( uint16_t adapter_type ) override;
    IUdpSocket*        AllocSocket() override;
    void               free_socket( IUdpSocket *socket ) override;
};
}
}
#endif /* _OS_LINUX_ */