/*
 * IInterfaceMonitor.h
 *
 *  Created on: Jul 6, 2017
 *      Author: psammand
 */

#pragma once

#include <vector>
#include <interface_addr.h>
#include "ErrCode.h"
#include <base_consts.h>
#include <data_types.h>

namespace ja_iot {
namespace network {
enum class InterfaceEventType
{
  kInterfaceAdded,
  kInterfaceRemoved,
  kInterfaceModified,
  kInterfaceStateChanged
};

/**
 * Event class for the interface monitor. The InterfaceMonitor class will inform
 * the clients by calling the handler function with this InterfaceEvent object.
 */
class InterfaceEvent
{
  public:
    InterfaceEvent( const InterfaceEventType event_type ) : _event_type{ event_type }
    {
    }

    InterfaceEventType  get_event_type() const { return ( _event_type ); }
    void                set_event_type( const InterfaceEventType interface_event_type ) { _event_type = interface_event_type; }
    uint16_t            get_adapter_type() const { return ( _adapter_type ); }
    void                set_adapter_type( const uint16_t adapter_type ) { _adapter_type = adapter_type; }
    InterfaceStatusFlag get_status_flag() const { return ( _status_flag ); }
    void                set_status_flag( const InterfaceStatusFlag interface_status_flag ) { _status_flag = interface_status_flag; }

  private:
    InterfaceEventType    _event_type;
    uint16_t              _adapter_type = base::k_adapter_type_default;
    InterfaceStatusFlag   _status_flag  = InterfaceStatusFlag::DOWN;
};

typedef void ( *pfn_interface_monitor_cb ) ( const InterfaceEvent *pcz_interface_event, void *pv_user_data );

class IInterfaceMonitor
{
  public:
    virtual ~IInterfaceMonitor ()
    {
    }

    virtual base::ErrCode start_monitor( uint16_t adapter_type ) = 0;
    virtual base::ErrCode stop_monitor( uint16_t adapter_type )  = 0;

    /**
     * This function will return all the found interfaces in the system.
     * @param skip_if_down					- this flag will control whether to
     * include only the interfaces which are UP.
     * @return
     */
    virtual std::vector<InterfaceAddress *> get_interface_addr_list( bool skip_if_down = true ) = 0;
    /**
     * This function should be called only after getting the kInterfaceModified
     * event notification. Calling this function
     * will return all the newly found interfaces.
     * @return
     */
    virtual std::vector<InterfaceAddress *> get_newly_found_interface() = 0;

    virtual void add_interface_event_callback( pfn_interface_monitor_cb cz_if_monitor_cb, void *pv_user_data ) = 0;
    virtual void remove_interface_event_callback( pfn_interface_monitor_cb cz_if_monitor_cb )                  = 0;
};
}
}