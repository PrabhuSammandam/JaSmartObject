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
#include <i_nwk_platform_factory.h>
#include <cstdint>
#include <deque>

constexpr uint16_t ADAPTER_MGR_MAX_NO_OF_NETWORK_HANDLERS = 5;
constexpr uint8_t  ADAPTER_MGR_MAX_NO_ADAPTERS            = 6;

namespace ja_iot {
namespace network {
class IAdapterMgrNetworkHandler
{
  public:
    virtual ~IAdapterMgrNetworkHandler () { }

    virtual void handle_adapter_changed( uint16_t adapter_type, bool is_enabled )   = 0;
    virtual void handle_conn_changed( Endpoint const &end_point, bool is_connected ) = 0;
};

  class IAdapterMgrEventHandler
  {
  public:
    IAdapterMgrEventHandler(){}
    virtual ~IAdapterMgrEventHandler(){}

    virtual void handle_packet_received(Endpoint const &end_point, const uint8_t *data, uint16_t data_len) = 0;
    virtual void handle_error(Endpoint const &end_point, const uint8_t *data, uint16_t data_len, base::ErrCode error) = 0;
  };

typedef std::vector<IAdapter *> AdapterList;

typedef void ( *pfn_adapter_mgr_pkt_recvd_cb ) ( void *pv_user_data, Endpoint const &rcz_end_point, const uint8_t *pu8_data, uint16_t u16_data_len );
typedef void ( *pfn_adapter_mgr_error_cb ) ( void *pv_user_data, Endpoint const &rcz_end_point, const uint8_t *pu8_data, uint16_t u16_data_len, base::ErrCode error );

class AdapterManager
{
  public:
    static AdapterManager& Inst();

    base::ErrCode initialize_adapters( uint16_t adapter_types );
    base::ErrCode terminate_adapters();

    base::ErrCode start_adapter( uint16_t adapter_type );
    base::ErrCode stop_adapter( uint16_t adapter_type );

    base::ErrCode start_servers()const;
    base::ErrCode stop_servers()const;

    base::ErrCode read_data()const;
    base::ErrCode send_unicast_data( Endpoint &end_point, const uint8_t *data, uint16_t data_length ) const;
    base::ErrCode send_multicast_data( Endpoint &end_point, const uint8_t *data, uint16_t data_length ) const;

    void set_packet_received_cb(const pfn_adapter_mgr_pkt_recvd_cb pfn_packet_received_cb, void *pv_user_data ) { this->_cb_packet_received = pfn_packet_received_cb; this->_pv_pkt_recvd_cb_data = pv_user_data; }
    void set_error_cb(const pfn_adapter_mgr_error_cb error_callback, void *pv_user_data ) { this->_cb_error = error_callback; this->_pv_error_cb_data = pv_user_data; }

    void add_adapter_network_handler( IAdapterMgrNetworkHandler *adapter_mgr_network_handler ) { _network_handlers_list.push_back( adapter_mgr_network_handler ); }
    void remove_adapter_network_handler( IAdapterMgrNetworkHandler *adapter_mgr_network_handler );

    uint16_t get_active_adapters() const { return ( _u16_selected_adapters ); }

    void handle_adapter_event( AdapterEvent *p_adapter_event );
    std::deque<Endpoint *> get_endpoints_list();

  private:
    AdapterManager ();
    ~AdapterManager ();
    AdapterManager( const AdapterManager & )               = delete;// prevent copy constructor
    AdapterManager & operator = ( const AdapterManager & ) = delete;// prevent assignment operator

    base::ErrCode init_adapter( uint16_t req_adapter_type, uint16_t to_init_adapter_type );
    IAdapter*     get_adapter_for_type( uint16_t adapter_type ) const;

    static AdapterManager * _pcz_instance;
    INetworkPlatformFactory *_pcz_platform_factory{};
    uint16_t   _u16_selected_adapters = base::k_adapter_type_default;
    pfn_adapter_mgr_pkt_recvd_cb _cb_packet_received{};
    void *_pv_pkt_recvd_cb_data{};
    pfn_adapter_mgr_error_cb _cb_error{};
    void *_pv_error_cb_data{};

    std::vector<IAdapterMgrNetworkHandler *> _network_handlers_list{};
    AdapterList _adapters_list{};
};
}
}
