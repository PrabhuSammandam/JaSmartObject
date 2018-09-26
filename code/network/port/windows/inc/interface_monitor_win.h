/*
 * InterfaceMonitorImplWindows.h
 *
 *  Created on: Jul 6, 2017
 *      Author: psammand
 */

#pragma once

#ifdef _OS_WINDOWS_

#include <memory>
#include <i_interface_monitor.h>
#include <interface_addr.h>
#include <cstdint>

constexpr uint16_t MAX_NO_OF_INTERFACE_EVENT_HANDLER = 2;
constexpr uint16_t MAX_NO_OF_INTERFACE_ADDRESS = 10;

namespace ja_iot
{
  namespace network
  {
    class InterfaceMonitorImplWindowsData;

    class InterfaceMonitorImplWindows : public IInterfaceMonitor
    {
    public:

      InterfaceMonitorImplWindows();
      ~InterfaceMonitorImplWindows();

      base::ErrCode start_monitor(uint16_t adapter_type) override;
      base::ErrCode stop_monitor(uint16_t adapter_type) override;

      std::vector<InterfaceAddress*> get_interface_addr_list(bool skip_if_down = true) override;
      std::vector<InterfaceAddress*> get_newly_found_interface() override;

      void add_interface_event_callback(pfn_interface_monitor_cb cz_if_monitor_cb, void* pv_user_data) override;
      void remove_interface_event_callback(pfn_interface_monitor_cb cz_if_monitor_cb) override;

    private:
      std::unique_ptr<InterfaceMonitorImplWindowsData> pimpl_;
    };
  }
}

#endif //_OS_WINDOWS_
