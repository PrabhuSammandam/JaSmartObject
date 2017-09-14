/*
 * Esp8266PlatformFactory.h
 *
 *  Created on: Sep 10, 2017
 *      Author: psammand
 */

#ifndef NETWORK_PORT_ESP8266_INC_ESP8266PLATFORMFACTORY_H_
#define NETWORK_PORT_ESP8266_INC_ESP8266PLATFORMFACTORY_H_

#ifdef _OS_FREERTOS_

#include <INetworkPlatformFactory.h>

namespace ja_iot {
namespace network {
class Esp8266PlatformFactory : public INetworkPlatformFactory
{
  public:

    Esp8266PlatformFactory ();

    IInterfaceMonitor* GetInterfaceMonitor()                  override;
    IAdapter*          GetAdapter( AdapterType adapter_type ) override;
    IUdpSocket*        AllocSocket() override;
    void               FreeSocket( IUdpSocket *socket ) override;
};
}
}


#endif /* _OS_FREERTOS_ */

#endif /* NETWORK_PORT_ESP8266_INC_ESP8266PLATFORMFACTORY_H_ */