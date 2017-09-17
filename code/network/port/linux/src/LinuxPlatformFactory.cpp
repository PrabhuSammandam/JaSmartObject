/*
 * LinuxPlatformFactory.cpp
 *
 *  Created on: 17-Sep-2017
 *      Author: prabhu
 */

#ifdef _OS_LINUX_
#include <port/linux/inc/LinuxPlatformFactory.h>
#include <port/linux/inc/InterfaceMonitorImplLinux.h>
#include <port/linux/inc/IpAdapterImplLinux.h>
#include <port/linux/inc/UdpSocketImplLinux.h>

namespace ja_iot {
namespace network {

static InterfaceMonitorImplLinux gs_if_monitor_linux { };

#ifdef _IP_ADAPTER_
#endif /* _IP_ADAPTER_ */

ja_iot::network::LinuxPlatformFactory::LinuxPlatformFactory() {
}

IInterfaceMonitor* ja_iot::network::LinuxPlatformFactory::GetInterfaceMonitor() {
	return &gs_if_monitor_linux;
}

IAdapter* ja_iot::network::LinuxPlatformFactory::GetAdapter(
		AdapterType adapter_type) {

}

IUdpSocket* ja_iot::network::LinuxPlatformFactory::AllocSocket() {
}

void ja_iot::network::LinuxPlatformFactory::FreeSocket(IUdpSocket* socket) {
}

}  // namespace network
}  // namespace ja_iot

#endif /* _OS_LINUX_ */
