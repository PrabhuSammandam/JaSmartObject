/*
 * IpAdapterBase.h
 *
 *  Created on: Jul 3, 2017
 *      Author: psammand
 */

#ifndef NETWORK_EXPORT_IPADAPTERBASE_H_
#define NETWORK_EXPORT_IPADAPTERBASE_H_

#include <EndPoint.h>
#include <IInterfaceMonitor.h>
#include <IUdpSocket.h>
#include "ErrCode.h"
#include "IAdapter.h"
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
    uint8_t *_data{ nullptr };
    uint16_t _dataLength{ 0 };
    bool is_multicast{ false };
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

    int32_t SendUnicastData( Endpoint const &end_point, const uint8_t *data, uint16_t data_length ) override;
    int32_t SendMulticastData( Endpoint const &end_point, const uint8_t *data, uint16_t data_length ) override;

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
    virtual ErrCode DoHandleInterfaceEvent( InterfaceEvent *interface_event ) {return ( ErrCode::OK );}

  protected:
    ErrCode CreateAndStartReceiveTask();
    void    StartIpv4MulticastAtInterface( uint32_t if_index );
    void    StartIpv6MulticastAtInterface( uint32_t if_index );

  private:

    ErrCode InitInternal();
    ErrCode TerminateInternal();
    ErrCode StartInterfaceMonitor();
    ErrCode StopInterfaceMonitor();
    void    InitFastShutdownMechanism();
    void    InitAddressChangeNotifyMechanism();
    ErrCode CreateAndStartSendingThread();
    void    ReceiveDataRoutine() { DoHandleReceive(); }
    ErrCode CreateSockets();
    ErrCode OpenIPV6Sockets();
    ErrCode OpenIPV4Sockets();
    ErrCode OpenSocket( IpAddrFamily ip_addr_family, bool is_multicast, IUdpSocket *udp_socket, uint16_t port = 0 );
    ErrCode OpenSocket2( IpAddrFamily ip_addr_family, bool is_multicast, IUdpSocket *udp_socket, uint16_t &port );
    void    ApplyMulticastAtInterfaceByAddress( IUdpSocket *udp_socket, IpAddress &ip_multicast_addr, uint32_t if_index );
    ErrCode HandleInterfaceEvent( InterfaceEvent *interface_event );

    IpAdapterQMsg* CreateNewSendMsg( Endpoint const &end_point, const uint8_t *data, uint16_t data_length, bool is_multicast = false );

    void HandleMsg( IpAdapterQMsg *msg );
    void DeleteMsg( IpAdapterQMsg *msg );

    class IpSendTaskMsgHandler : public ja_iot::osal::ITaskMsgHandler
    {
      public:

        IpSendTaskMsgHandler( IpAdapterBase *ip_adapter_base ) : ip_adapter_base_{ ip_adapter_base } {}

        void HandleMsg( void *msg ) override { ip_adapter_base_->HandleMsg( (IpAdapterQMsg *) msg ); }

        void DeleteMsg( void *msg ) override { ip_adapter_base_->DeleteMsg( (IpAdapterQMsg *) msg ); }

      private:

        IpAdapterBase *ip_adapter_base_{ nullptr };
    };

    class IpReceiveTaskRoutine : public ja_iot::osal::ITaskRoutine
    {
      public:

        IpReceiveTaskRoutine( IpAdapterBase *ip_adapater_impl_windows ) : host_{ ip_adapater_impl_windows } {}
        void Run( void *arg ) { host_->ReceiveDataRoutine(); }

      private:
        IpAdapterBase   *host_;
    };

    class InterfaceEventHandler : public IInterfaceEventHandler
    {
      public:

        InterfaceEventHandler( IpAdapterBase *host ) : host_{ host } {}

        ErrCode HandleInterfaceEvent( InterfaceEvent *interface_event ) override { return ( host_->HandleInterfaceEvent( interface_event ) ); }

      private:
        IpAdapterBase   *host_;
    };

  protected:

    bool is_started_{ false };
    bool is_terminated_{ true };
    int32_t   select_timeout_ = -1;
    bool is_ipv6_enabled_{ false };
    bool is_ipv4_enabled_{ false };

    uint16_t ipv4_unicast_port_{ 0 };
    uint16_t ipv4_unicast_secure_port_{ 0 };
    uint16_t ipv6_unicast_port_{ 0 };
    uint16_t ipv6_unicast_secure_port_{ 0 };

    IUdpSocket *ipv4_unicast_socket_{ nullptr };
    IUdpSocket *ipv4_unicast_secure_socket_{ nullptr };

    IUdpSocket *ipv4_multicast_socket_{ nullptr };
    IUdpSocket *ipv4_multicast_secure_socket_{ nullptr };

    IUdpSocket *ipv6_unicast_socket_{ nullptr };
    IUdpSocket *ipv6_unicast_secure_socket_{ nullptr };

    IUdpSocket *ipv6_multicast_socket_{ nullptr };
    IUdpSocket *ipv6_multicast_secure_socket_{ nullptr };

    IpSendTaskMsgHandler _ipSendTaskMsgHandler{ this };
    IAdapterEventHandler *_adapterHandler{ nullptr };

    ja_iot::osal::Task *_msgSendtask{ nullptr };
    ja_iot::osal::Task *receive_task_{ nullptr };

    IpReceiveTaskRoutine receive_task_routine_{ this };
    InterfaceEventHandler if_event_handler_{ this };

    ja_iot::osal::Mutex *send_msg_q_mutex_{ nullptr };
    ja_iot::base::SimpleList<IpAdapterQMsg, IP_ADAPTER_MSG_Q_CAPACITY> _ipAdapterMsgList{};
    ja_iot::base::PtrMsgQ<IP_ADAPTER_MSG_Q_CAPACITY> _ipAdapterMsgQ{};
};
}
}


#endif /* NETORK_EXPORT_IPADAPTERBASE_H_ */
