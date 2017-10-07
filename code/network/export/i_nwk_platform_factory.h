/*
 * AbstractPlatformFactory.h
 *
 *  Created on: Jul 11, 2017
 *      Author: psammand
 */

#ifndef INETWORKPLATFORMFACTORY_H_
#define INETWORKPLATFORMFACTORY_H_

#include <i_adapter.h>
#include <i_interface_monitor.h>
#include <i_udp_socket.h>

namespace ja_iot {
namespace network {
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

    virtual ~INetworkPlatformFactory () {}
    static INetworkPlatformFactory* CreateFactory( NetworkPlatform platform );
    static INetworkPlatformFactory* GetCurrFactory() { return ( curr_factory_ ); }
    static void                     SetCurrFactory( INetworkPlatformFactory *curr_factory ) { curr_factory_ = curr_factory; }

    virtual IInterfaceMonitor* GetInterfaceMonitor()                  = 0;
    virtual IAdapter*          GetAdapter( uint16_t adapter_type ) = 0;
    virtual IUdpSocket*        AllocSocket()                          = 0;
    virtual void               FreeSocket( IUdpSocket *socket )       = 0;

  private:
    static INetworkPlatformFactory * curr_factory_;
};
}
}



#endif /* INETWORKPLATFORMFACTORY_H_ */
