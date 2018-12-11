/*
 * AdapterMgr.h
 *
 *  Created on: Jun 29, 2017
 *      Author: psammand
 */
#pragma once

#include <functional>
#include <end_point.h>
#include <i_adapter.h>
#include <base_consts.h>
#include <base_datatypes.h>
#include <i_nwk_platform_factory.h>
#include <network_datatypes.h>
#include <cstdint>
#include <deque>
#include "SecureContext.h"

constexpr uint16_t ADAPTER_MGR_MAX_NO_OF_NETWORK_HANDLERS = 5;
constexpr uint8_t  ADAPTER_MGR_MAX_NO_ADAPTERS            = 6;

namespace ja_iot {
using namespace base;
namespace network {
class IAdapterMgrNetworkHandler
{
  public:
    virtual ~IAdapterMgrNetworkHandler () {}

    virtual void handle_adapter_changed( uint16_t adapter_type, bool is_enabled )    = 0;
    virtual void handle_conn_changed( Endpoint const &end_point, bool is_connected ) = 0;
};

class IAdapterMgrEventHandler
{
  public:
    IAdapterMgrEventHandler () {}
    virtual ~IAdapterMgrEventHandler () {}

    virtual void handle_packet_received( Endpoint const &end_point, const data_buffer_t &data_buffer )      = 0;
    virtual void handle_error( Endpoint const &end_point, const data_buffer_t &data_buffer, ErrCode error ) = 0;
};

typedef std::vector<IAdapter *> AdapterList;
typedef std::vector<IAdapterMgrNetworkHandler *> AdapterMgrNetworkHandlerArray;

class AdapterManager
{
  public:
    static AdapterManager& Inst();

    ErrCode initialize_adapters( uint16_t adapter_types );
    ErrCode terminate_adapters();

    ErrCode start_adapter( uint16_t adapter_type );
    ErrCode stop_adapter( uint16_t adapter_type );

    ErrCode start_servers() const;
    ErrCode stop_servers() const;

    ErrCode read_data() const;
    ErrCode send_unicast_data( Endpoint &end_point, const uint8_t *data, uint16_t data_length ) ;
    ErrCode send_multicast_data( Endpoint &end_point, const uint8_t *data, uint16_t data_length ) ;

    void add_adapter_network_handler( IAdapterMgrNetworkHandler *adapter_mgr_network_handler ) { _network_handlers_list.push_back( adapter_mgr_network_handler ); }
    void remove_adapter_network_handler( IAdapterMgrNetworkHandler *adapter_mgr_network_handler );

    uint16_t get_active_adapters() const { return ( _u16_selected_adapters ); }

    void                    handle_adapter_event( AdapterEvent *p_adapter_event );
    void                    handle_secure_context_event( SecureContextEvent &secure_context_event );
    std::vector<Endpoint *> get_endpoints_list();
    void                    set_event_handler( IAdapterMgrEventHandler *event_handler ) { _event_handler = event_handler; }

  private:
    AdapterManager ();
    ~AdapterManager ();
    AdapterManager( const AdapterManager & )               = delete;// prevent copy constructor
    AdapterManager & operator = ( const AdapterManager & ) = delete;// prevent assignment operator

    ErrCode   init_adapter( uint16_t req_adapter_type, uint16_t to_init_adapter_type );
    IAdapter* get_adapter_for_type( uint16_t adapter_type ) const;

    static AdapterManager *         _pcz_instance; // static class variable cannot be instantiated here.
    INetworkPlatformFactory *       _pcz_platform_factory  = nullptr;
    uint16_t                        _u16_selected_adapters = k_adapter_type_default;
    IAdapterMgrEventHandler *       _event_handler         = nullptr;
    AdapterMgrNetworkHandlerArray   _network_handlers_list;
    AdapterList                     _adapters_list;

#ifdef _ENABLE_SECURE_
    SecureContext   _secure_context;
#endif
    // void set_packet_received_cb( const pfn_adapter_mgr_pkt_recvd_cb pfn_packet_received_cb, void *pv_user_data ) { this->_cb_packet_received = pfn_packet_received_cb; this->_pv_pkt_recvd_cb_data = pv_user_data; }
    // void set_error_cb( const pfn_adapter_mgr_error_cb error_callback, void *pv_user_data ) { this->_cb_error = error_callback; this->_pv_error_cb_data = pv_user_data; }

    // pfn_adapter_mgr_pkt_recvd_cb   _cb_packet_received    = nullptr;
    // void *                         _pv_pkt_recvd_cb_data  = nullptr;
    // pfn_adapter_mgr_error_cb       _cb_error              = nullptr;
    // void *                         _pv_error_cb_data      = nullptr;
    friend class AdapterEventHandler;
    friend class SecureContextEventHandler;
};
}
}
