/*
 * InterfaceMonitorImplEsp8266.h
 *
 *  Created on: Sep 11, 2017
 *      Author: psammand
 */

#ifndef NETWORK_PORT_ESP8266_INC_INTERFACEMONITORIMPLESP8266_H_
#define NETWORK_PORT_ESP8266_INC_INTERFACEMONITORIMPLESP8266_H_

#ifdef _OS_FREERTOS_

#include "ErrCode.h"
#include <interface_addr.h>
#include <i_interface_monitor.h>

namespace ja_iot {
namespace network {
class InterfaceMonitorImplEsp8266 : public IInterfaceMonitor
{
  public:
    InterfaceMonitorImplEsp8266 ();

    base::ErrCode start_monitor( uint16_t adapter_type ) override;
    base::ErrCode stop_monitor( uint16_t adapter_type )  override;

    std::vector<InterfaceAddress *> get_interface_addr_list(  bool skip_if_down = true ) override;
    std::vector<InterfaceAddress *> get_newly_found_interface( ) override;

    void add_interface_event_callback( pfn_interface_monitor_cb cz_if_monitor_cb, void* pv_user_data)    override;
    void remove_interface_event_callback( pfn_interface_monitor_cb cz_if_monitor_cb ) override;

  private:
    InterfaceAddress   if_addr_;
    bool               is_if_addr_valid_ = false;
};
}
}
#endif /* _OS_FREERTOS_ */

#endif /* NETWORK_PORT_ESP8266_INC_INTERFACEMONITORIMPLESP8266_H_ */
