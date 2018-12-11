/*
 * IpAdapterBase.h
 *
 *  Created on: Jul 3, 2017
 *      Author: psammand
 */

#pragma once

#include <end_point.h>
#include <i_adapter.h>
#include <i_interface_monitor.h>
#include <i_udp_socket.h>
#include "ErrCode.h"
#include "Task.h"
#include "SimpleList.h"
#include "PtrMsgQ.h"
#include "base_datatypes.h"
#include <base_consts.h>
#include "Mutex.h"
#include <vector>
#include <deque>

constexpr uint16_t IP_ADAPTER_MSG_Q_CAPACITY = 10;

namespace ja_iot {
namespace network {
class IpAdapterQMsg
{
  public:

    Endpoint   end_point_;
    uint8_t *  _data        = nullptr;
    uint16_t   _dataLength  = 0;
    bool       is_multicast = false;
};

class IpAdapterBase : public IAdapter
{
  public:
    IpAdapterBase ();
    ~IpAdapterBase ();

    base::ErrCode initialize() override;
    base::ErrCode terminate() override;

    base::ErrCode start_adapter() override;
    base::ErrCode stop_adapter() override;

    base::ErrCode start_server() override;
    base::ErrCode stop_server() override;

    base::ErrCode start_listening() override;
    base::ErrCode stop_listening() override;

    int32_t send_unicast_data( Endpoint &end_point, const uint8_t *data, uint16_t data_length ) override;
    int32_t send_multicast_data( Endpoint &end_point, const uint8_t *data, uint16_t data_length ) override;

    uint16_t get_type() override { return ( base::k_adapter_type_ip ); }

//    void          set_adapter_event_cb( const pfn_adapter_event_cb pfn_adapter_event_callback, void *pv_user_data ) override { this->_adapter_event_callback = pfn_adapter_event_callback; this->_adapter_event_cb_data = pv_user_data; }
    void          set_event_handler( IAdapterEventHandler *pcz_event_handler ) override { _event_handler = pcz_event_handler; }
    base::ErrCode get_endpoints_list( std::vector<Endpoint *> &rcz_endpoint_list ) override;
    void          SEND_TASK_handle_msg( void *msg );
    void          SEND_TASK_delete_msg( void *msg );

  protected:
    virtual base::ErrCode do_pre_intialize() { return ( base::ErrCode::OK ); }
    virtual base::ErrCode do_post_intialize() { return ( base::ErrCode::OK ); }

    virtual base::ErrCode do_pre_terminate() { return ( base::ErrCode::OK ); }
    virtual base::ErrCode do_post_terminate() { return ( base::ErrCode::OK ); }

    virtual base::ErrCode do_pre_start_adapter() { return ( base::ErrCode::OK ); }
    virtual base::ErrCode do_post_start_adapter() { return ( base::ErrCode::OK ); }

    virtual base::ErrCode do_pre_stop_adapter() { return ( base::ErrCode::OK ); }
    virtual base::ErrCode do_post_stop_adapter() { return ( base::ErrCode::OK ); }

    virtual base::ErrCode do_pre_start_server() { return ( base::ErrCode::OK ); }
    virtual base::ErrCode do_post_start_server() { return ( base::ErrCode::OK ); }

    virtual base::ErrCode do_pre_stop_server() { return ( base::ErrCode::OK ); }
    virtual base::ErrCode do_post_stop_server();

    virtual void          do_handle_send_msg( IpAdapterQMsg *msg ) {}
    virtual void          do_delete_msg( IpAdapterQMsg *msg ) {}
    virtual void          do_handle_receive() {}
    virtual void          do_init_fast_shutdown_mechanism() {}
    virtual void          do_init_address_change_notify_mechanism() {}
    virtual void          do_un_init_address_change_notify_mechanism() {}
    virtual base::ErrCode do_handle_interface_event( InterfaceEvent *interface_event ) { return ( base::ErrCode::OK ); }

    virtual std::vector<InterfaceAddress *> get_interface_address_for_index( uint8_t u8_index ) = 0;

    base::ErrCode start_receive_task();
    base::ErrCode start_sending_thread();
    void          start_ipv4_mcast_at_interface( uint32_t if_index ) const;
    void          start_ipv6_mcast_at_interface( uint32_t if_index ) const;
    IUdpSocket*   get_socket_by_mask( uint16_t socket_type_mask );
    void          handle_address_change_event();
    void          update_interface_listening( std::vector<InterfaceAddress *> &cz_interface_addr_list );
    void          refresh_end_point_list( std::vector<InterfaceAddress *> &cz_interface_addr_list );
    void          refresh_end_point_list();
    void          send_data_to_socket( IUdpSocket *pcz_udp_socket, Endpoint &endpoint, const data_buffer_t &data_buffer ) const;

  private:

    base::ErrCode        terminate_internal();
    base::ErrCode        open_ipv6_sockets();
    base::ErrCode        open_ipv4_sockets();
    static base::ErrCode open_socket( base::IpAddrFamily ip_addr_family, bool is_multicast, IUdpSocket *udp_socket, uint16_t port = 0 );
    base::ErrCode        open_socket_with_retry( base::IpAddrFamily ip_addr_family, bool is_multicast, IUdpSocket *udp_socket, uint16_t &port ) const;
    base::ErrCode        add_socket( uint16_t socket_type, uint16_t port );

    static void    receive_data_routine( void *arg ) { static_cast<IpAdapterBase *>( arg )->do_handle_receive(); }
    int32_t        post_data_to_send_task( Endpoint &end_point, const uint8_t *data, uint16_t data_length, bool is_mcast );
    IpAdapterQMsg* create_new_send_msg( Endpoint const &end_point, const uint8_t *data, uint16_t data_length, bool is_multicast = false );

  protected:

    bool      is_started_     = false;
    bool      is_terminated_  = true;
    int32_t   select_timeout_ = -1;

    std::vector<IUdpSocket *>   _sockets;

    osal::Task *  sender_task_       = nullptr;
    osal::Mutex * sender_task_mutex_ = nullptr;
    osal::Task *  _receive_task      = nullptr;

//    pfn_adapter_event_cb   _adapter_event_callback = nullptr;
//    void *                 _adapter_event_cb_data  = nullptr;

    IAdapterEventHandler * _event_handler = nullptr;

    base::SimpleList<IpAdapterQMsg, IP_ADAPTER_MSG_Q_CAPACITY> ip_adapter_msg_q_list_{};
    base::PtrMsgQ<IP_ADAPTER_MSG_Q_CAPACITY> ip_adapter_msg_q_{};

    osal::Mutex * _access_mutex = nullptr;
    EndpointList _cz_end_points{};
};
}
}
