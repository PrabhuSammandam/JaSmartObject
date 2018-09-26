/*
 * WindowsPlatformFactory.h
 *
 *  Created on: Jul 11, 2017
 *      Author: psammand
 */

#pragma once

#ifdef _OS_WINDOWS_

#include <i_nwk_platform_factory.h>

namespace ja_iot
{
  namespace network
  {
    class WindowsPlatformFactory : public INetworkPlatformFactory
    {
    public:

      WindowsPlatformFactory();

      IInterfaceMonitor* get_interface_monitor() override;
      IAdapter* get_adapter(uint16_t adapter_type) override;
      IUdpSocket* AllocSocket() override;
      void free_socket(IUdpSocket* socket) override;
    };
  }
}
#endif //#ifdef _OS_WINDOWS_
