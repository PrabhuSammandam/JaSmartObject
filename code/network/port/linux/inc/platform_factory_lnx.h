/*
 * LinuxPlatformFactory.h
 *
 *  Created on: 17-Sep-2017
 *      Author: prabhu
 */

#ifndef NETWORK_PORT_LINUX_INC_PLATFORM_FACTORY_LNX_H_
#define NETWORK_PORT_LINUX_INC_PLATFORM_FACTORY_LNX_H_

#ifdef _OS_LINUX_

#include <INetworkPlatformFactory.h>

namespace ja_iot {
namespace network {
class LinuxPlatformFactory : public INetworkPlatformFactory
{
  public:

	LinuxPlatformFactory ();

    IInterfaceMonitor* GetInterfaceMonitor()                  override;
    IAdapter*          GetAdapter( AdapterType adapter_type ) override;
    IUdpSocket*        AllocSocket() override;
    void               FreeSocket( IUdpSocket *socket ) override;
};
}
}
#endif /* _OS_LINUX_ */




#endif /* NETWORK_PORT_LINUX_INC_PLATFORM_FACTORY_LNX_H_ */
