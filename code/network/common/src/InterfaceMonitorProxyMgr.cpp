/*
 * InterfaceMonitorMgr.cpp
 *
 *  Created on: Jul 6, 2017
 *      Author: psammand
 */

#include <IInterfaceMonitor.h>

namespace ja_iot {
namespace network {
#if 0
InterfaceMonitorMgr *InterfaceMonitorMgr::p_instance_{ nullptr };

InterfaceMonitorMgr & network::InterfaceMonitorMgr::Inst()
{
   if( p_instance_ == nullptr )
   {
      static InterfaceMonitorMgr _instance{};
      p_instance_ = &_instance;
   }

   return ( *p_instance_ );
}

void InterfaceMonitorMgr::RegisterInterfaceMonitor( InterfaceMonitorProxyBase *interface_monitor_proxy )
{
   if( interface_monitor_proxy != nullptr )
   {
      interface_monitor_ = interface_monitor_proxy->GetInterfaceMonitor();
   }
}
#endif
}
}
