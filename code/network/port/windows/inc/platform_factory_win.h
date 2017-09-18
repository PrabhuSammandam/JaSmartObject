/*
 * WindowsPlatformFactory.h
 *
 *  Created on: Jul 11, 2017
 *      Author: psammand
 */

#ifndef WINDOWSPLATFORMFACTORY_H_
#define WINDOWSPLATFORMFACTORY_H_

#ifdef _OS_WINDOWS_

#include <i_nwk_platform_factory.h>

namespace ja_iot {
namespace network {
class WindowsPlatformFactory : public INetworkPlatformFactory
{
  public:

    WindowsPlatformFactory ();

    IInterfaceMonitor* GetInterfaceMonitor()                  override;
    IAdapter*          GetAdapter( AdapterType adapter_type ) override;
    IUdpSocket*        AllocSocket() override;
    void               FreeSocket( IUdpSocket *socket ) override;
};
}
}
#endif //#ifdef _OS_WINDOWS_
#endif /* WINDOWSPLATFORMFACTORY_H_ */
