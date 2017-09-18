/*
 * IpAdapterBase.h
 *
 *  Created on: Jul 3, 2017
 *      Author: psammand
 */

#ifndef NETWORK_EXPORT_IP_ADAPTER_BASE_H_
#define NETWORK_EXPORT_IP_ADAPTER_BASE_H_

#include <end_point.h>
#include <i_adapter.h>
#include <i_interface_monitor.h>
#include <i_udp_socket.h>
#include "ErrCode.h"
#include "Task.h"
#include "SimpleList.h"
#include "PtrMsgQ.h"
#include "Mutex.h"

constexpr uint16_t IP_ADAPTER_MSG_Q_CAPACITY = 10;

using ErrCode = ja_iot::base::ErrCode;

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

    ErrCode Initialize() override;
    ErrCode Terminate()  override;

    ErrCode StartAdapter() override;
    ErrCode StopAdapter()  override;

    ErrCode StartServer() override;
    ErrCode StopServer() override;

    ErrCode StartListening() override;
    ErrCode StopListening() override;

    int32_t SendUnicastData( Endpoint &end_point, const uint8_t *data, uint16_t data_length ) override;
    int32_t SendMulticastData( Endpoint &end_point, const uint8_t *data, uint16_t data_length ) override;

    AdapterType GetType() override { return ( AdapterType::IP ); }

    void SetAdapterHandler( IAdapterEventHandler *adapterHandler ) { _adapterHandler = adapterHandler; }

  protected:
    virtual ErrCode DoPreIntialize() { return ( ErrCode::OK ); }
    virtual ErrCode DoPostIntialize() { return ( ErrCode::OK ); }

    virtual ErrCode DoPreTerminate() { return ( ErrCode::OK ); }
    virtual ErrCode DoPostTerminate() { return ( ErrCode::OK ); }

    virtual ErrCode DoPreStartAdapter() { return ( ErrCode::OK ); }
    virtual ErrCode DoPostStartAdapter() { return ( ErrCode::OK ); }

    virtual ErrCode DoPreStopAdapter() { return ( ErrCode::OK ); }
    virtual ErrCode DoPostStopAdapter() { return ( ErrCode::OK ); }

    virtual ErrCode DoPreStartServer() { return ( ErrCode::OK ); }
    virtual ErrCode DoPostStartServer() { return ( ErrCode::OK ); }

    virtual ErrCode DoPreStopServer() { return ( ErrCode::OK ); }
    virtual ErrCode DoPostStopServer() { return ( ErrCode::OK ); }

    virtual void DoHandleSendMsg( IpAdapterQMsg *msg ) {}
    virtual void DoDeleteMsg( IpAdapterQMsg *msg ) {}
    virtual void DoHandleReceive() {}

    virtual void    DoInitFastShutdownMechanism() {}
    virtual void    DoInitAddressChangeNotifyMechanism() {}
    virtual void    DoUnInitAddressChangeNotifyMechanism() {}
    virtual ErrCode DoHandleInterfaceEvent( InterfaceEvent *interface_event ) { return ( ErrCode::OK ); }

  protected:
    ErrCode CreateAndStartReceiveTask();
    void    start_ipv4_mcast_at_interface( uint32_t if_index );
    void    start_ipv6_mcast_at_interface( uint32_t if_index );

  private:

    ErrCode InitInternal();
    ErrCode TerminateInternal();
    ErrCode StartInterfaceMonitor();
    ErrCode StopInterfaceMonitor();
    void    InitFastShutdownMechanism();
    void    InitAddressChangeNotifyMechanism();
    ErrCode CreateAndStartSendingThread();
    void    ReceiveDataRoutine( void *arg ) { DoHandleReceive(); }
    ErrCode CreateSockets();
    ErrCode OpenIPV6Sockets( bool open_ucast_sec, bool open_mcast, bool open_mcast_sec );
    ErrCode OpenIPV4Sockets( bool open_ucast_sec, bool open_mcast, bool open_mcast_sec );
    ErrCode OpenSocket( IpAddrFamily ip_addr_family, bool is_multicast, IUdpSocket *udp_socket, uint16_t port = 0 );
    ErrCode OpenSocket2( IpAddrFamily ip_addr_family, bool is_multicast, IUdpSocket *udp_socket, uint16_t &port );
    void    join_mcast_group( IUdpSocket *udp_socket, IpAddress &ip_multicast_addr, uint32_t if_index );
    ErrCode HandleInterfaceEvent( InterfaceEvent *interface_event );
    int32_t post_data_to_send_task( Endpoint &end_point, const uint8_t *data, uint16_t data_length, bool is_mcast );

    IUdpSocket* get_socket( IpAddrFamily ip_addr_family, bool is_multicast = false, bool is_secure = false );

    IpAdapterQMsg* CreateNewSendMsg( Endpoint const &end_point, const uint8_t *data, uint16_t data_length, bool is_multicast = false );

    void HandleMsg( void *msg );
    void DeleteMsg( void *msg );

    DECLARE_TASK_MSG_HANDLER_CLASS( IpSendTaskMsgHandler, IpAdapterBase, HandleMsg, DeleteMsg );
    DECLARE_TASK_ROUTINE_CLASS( IpReceiveTaskRoutine, IpAdapterBase, ReceiveDataRoutine );
    DECLARE_INTERFACE_EVENT_HANDLER_CLASS( InterfaceEventHandler, IpAdapterBase, HandleInterfaceEvent );

  protected:

    bool      is_started_     = false;
    bool      is_terminated_  = true;
    int32_t   select_timeout_ = -1;

    uint16_t   ipv4_unicast_port_        = 0;
    uint16_t   ipv4_unicast_secure_port_ = 0;
    uint16_t   ipv6_unicast_port_        = 0;
    uint16_t   ipv6_unicast_secure_port_ = 0;

    IUdpSocket * ipv4_unicast_socket_          = nullptr;
    IUdpSocket * ipv4_unicast_secure_socket_   = nullptr;
    IUdpSocket * ipv4_multicast_socket_        = nullptr;
    IUdpSocket * ipv4_multicast_secure_socket_ = nullptr;
    IUdpSocket * ipv6_unicast_socket_          = nullptr;
    IUdpSocket * ipv6_unicast_secure_socket_   = nullptr;
    IUdpSocket * ipv6_multicast_socket_        = nullptr;
    IUdpSocket * ipv6_multicast_secure_socket_ = nullptr;

    ja_iot::osal::Task * sender_task_ = nullptr;
    IpSendTaskMsgHandler sender_task_msg_q_handler_{ this };
    ja_iot::osal::Mutex * sender_task_mutex_ = nullptr;

    ja_iot::osal::Task * receiver_task_ = nullptr;
    IpReceiveTaskRoutine receive_task_routine_{ this };

    IAdapterEventHandler * _adapterHandler = nullptr;
    InterfaceEventHandler if_event_handler_{ this };

    ja_iot::base::SimpleList<IpAdapterQMsg, IP_ADAPTER_MSG_Q_CAPACITY> ip_adapter_msg_q_list_{};
    ja_iot::base::PtrMsgQ<IP_ADAPTER_MSG_Q_CAPACITY> ip_adapter_msg_q_{};
};
}
}


#endif /* NETORK_EXPORT_IPADAPTERBASE_H_ */
