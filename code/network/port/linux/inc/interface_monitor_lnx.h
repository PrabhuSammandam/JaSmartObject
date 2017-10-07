/*
 * InterfaceMonitorImplLinux.h
 *
 *  Created on: 17-Sep-2017
 *      Author: prabhu
 */

#ifndef NETWORK_PORT_LINUX_INC_INTERFACE_MONITOR_LNX_H_
#define NETWORK_PORT_LINUX_INC_INTERFACE_MONITOR_LNX_H_

#include <i_interface_monitor.h>
#include <TransportType.h>

constexpr uint16_t MAX_NO_OF_INTERFACE_EVENT_HANDLER = 2;
constexpr uint16_t MAX_NO_OF_INTERFACE_ADDRESS       = 10;

namespace ja_iot {
namespace network {

class InterfaceMonitorImplLinuxData;

class InterfaceMonitorImplLinux: public IInterfaceMonitor {
public:
	InterfaceMonitorImplLinux();

	ErrCode StartMonitor(uint16_t adapter_type) override;
	ErrCode StopMonitor(uint16_t adapter_type) override;

	ErrCode GetInterfaceAddrList(
			InterfaceAddressPtrArray &interface_address_ptr_array,
			bool skip_if_down = true) override;
	ErrCode GetNewlyFoundInterface(InterfaceAddressPtrArray &if_addr_ptr_array)
			override;

	void AddInterfaceEventHandler(
			IInterfaceEventHandler *interface_event_handler) override;
	void RemoveInterfaceEventHandler(
			IInterfaceEventHandler *interface_event_handler) override;

private:
	InterfaceMonitorImplLinuxData *pimpl_{ nullptr };
};

}  // namespace network
}  // namespace ja_iot

#endif /* NETWORK_PORT_LINUX_INC_INTERFACE_MONITOR_LNX_H_ */
