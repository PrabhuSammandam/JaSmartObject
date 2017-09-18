/*
 * InterfaceMonitorImplWindows.h
 *
 *  Created on: Jul 6, 2017
 *      Author: psammand
 */

#ifndef INTERFACEMONITORIMPLWINDOWS_H_
#define INTERFACEMONITORIMPLWINDOWS_H_

#ifdef _OS_WINDOWS_

#include <i_interface_monitor.h>
#include <interface_addr.h>
#include <data_types.h>
#include <cstdint>

constexpr uint16_t MAX_NO_OF_INTERFACE_EVENT_HANDLER = 2;
constexpr uint16_t MAX_NO_OF_INTERFACE_ADDRESS       = 10;

namespace ja_iot {
namespace network {
class InterfaceMonitorImplWindowsData;

class InterfaceMonitorImplWindows : public IInterfaceMonitor
{
  public:

    InterfaceMonitorImplWindows ();

    ErrCode StartMonitor( AdapterType adapter_type ) override;
    ErrCode StopMonitor( AdapterType adapter_type )  override;

    ErrCode GetInterfaceAddrList( InterfaceAddressPtrArray &interface_address_ptr_array, bool skip_if_down = true ) override;
    ErrCode GetNewlyFoundInterface( InterfaceAddressPtrArray &if_addr_ptr_array ) override;

    void AddInterfaceEventHandler( IInterfaceEventHandler *interface_event_handler )    override;
    void RemoveInterfaceEventHandler( IInterfaceEventHandler *interface_event_handler ) override;

  private:
    InterfaceMonitorImplWindowsData *pimpl_{ nullptr };
};
}
}

#endif //_OS_WINDOWS_
#endif /* INTERFACEMONITORIMPLWINDOWS_H_ */
