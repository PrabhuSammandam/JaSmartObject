/*
 * InterfaceMonitorImplEsp8266.h
 *
 *  Created on: Sep 11, 2017
 *      Author: psammand
 */

#ifndef NETWORK_PORT_ESP8266_INC_INTERFACEMONITORIMPLESP8266_H_
#define NETWORK_PORT_ESP8266_INC_INTERFACEMONITORIMPLESP8266_H_

#ifdef _OS_FREERTOS_

#include <interface_addr.h>
#include <i_interface_monitor.h>

namespace ja_iot {
namespace network {
class InterfaceMonitorImplEsp8266 : public IInterfaceMonitor
{
  public:

    InterfaceMonitorImplEsp8266 ();

    ErrCode StartMonitor( uint16_t adapter_type ) override;
    ErrCode StopMonitor( uint16_t adapter_type )  override;

    ErrCode GetInterfaceAddrList( InterfaceAddressPtrArray &interface_address_ptr_array, bool skip_if_down = true ) override;
    ErrCode GetNewlyFoundInterface( InterfaceAddressPtrArray &if_addr_ptr_array ) override;

    void AddInterfaceEventHandler( IInterfaceEventHandler *interface_event_handler )    override;
    void RemoveInterfaceEventHandler( IInterfaceEventHandler *interface_event_handler ) override;

  private:
    InterfaceAddress   if_addr_;
    bool               is_if_addr_valid_ = false;
};
}
}
#endif /* _OS_FREERTOS_ */

#endif /* NETWORK_PORT_ESP8266_INC_INTERFACEMONITORIMPLESP8266_H_ */
