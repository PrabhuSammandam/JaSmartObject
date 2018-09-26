/*
 * AbstractPlatformFactory.h
 *
 *  Created on: Jul 11, 2017
 *      Author: psammand
 */

#pragma once

#include <i_adapter.h>
#include <i_interface_monitor.h>
#include <i_udp_socket.h>

namespace ja_iot
{
  namespace network
  {
    enum class NetworkPlatform
    {
      kWindows,
      kLinux,
      kAndroid,
      kArduino,
      kEsp8266
    };

    class INetworkPlatformFactory
    {
    public:

      virtual ~INetworkPlatformFactory()
      {
      }

      static INetworkPlatformFactory* create_factory(NetworkPlatform platform);
      static INetworkPlatformFactory* GetCurrFactory() { return curr_factory_; }
      static void set_curr_factory(INetworkPlatformFactory* curr_factory) { curr_factory_ = curr_factory; }

      virtual IInterfaceMonitor* get_interface_monitor() = 0;
      virtual IAdapter* get_adapter(uint16_t adapter_type) = 0;
      virtual IUdpSocket* AllocSocket() = 0;
      virtual void free_socket(IUdpSocket* socket) = 0;

    private:
      static INetworkPlatformFactory* curr_factory_;
    };
  }
}
