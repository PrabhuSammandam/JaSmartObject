/*
 * IpAdapterWindows.cpp
 *
 *  Created on: Jul 3, 2017
 *      Author: psammand
 */

#include <config_network.h>
#include <cstring>


#ifdef _OS_WINDOWS_

#include <ip_addr.h>
#include <i_udp_socket.h>
#include <port/windows/inc/udp_socket_win.h>
#include <cstdio>
#include <end_point.h>
#include <guiddef.h>
#include <in6addr.h>
#include <inaddr.h>
#include <i_adapter.h>
#include <interface_addr.h>
#include <konstants_network.h>
#include <minwindef.h>
#include <mswsock.h>
#include <OsalError.h>
#include <psdk_inc/_ip_mreq1.h>
#include <psdk_inc/_ip_types.h>
#include <psdk_inc/_socket_types.h>
#include <psdk_inc/_wsadata.h>
#include <stddef.h>
#include <Task.h>
#include <data_types.h>
#include <winerror.h>
#include <ws2ipdef.h>
#include <ws2tcpip.h>
#include <climits>
#include <cstdint>
#include <base_consts.h>

#include <winsock2.h>
#include <common/inc/ip_adapter_base.h>
// #include "AdapterProxy.h"
#include <port/windows/inc/ip_adapter_win.h>
#include "OsalMgr.h"
#include <i_interface_monitor.h>
#include <PtrArray.h>
#include <i_nwk_platform_factory.h>
#include <ip_adapter_config.h>
#include <adapter_mgr.h>
#include <common/inc/logging_network.h>
#include <common/inc/common_utils.h>
#include <IMemAllocator.h>
#include <config_mgr.h>

using namespace ja_iot::osal;
using namespace ja_iot::base;

namespace ja_iot {
namespace network {
IpAdapterImplWindows::IpAdapterImplWindows ()
{
}

IpAdapterImplWindows::~IpAdapterImplWindows ()
{
}

ErrCode IpAdapterImplWindows::DoPreIntialize()
{
  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "IpAdapterImplWindows::DoPreIntialize:%d# ENTER", __LINE__ );

  WSADATA wsa_data{};

  if( WSAStartup( MAKEWORD( 2, 2 ), &wsa_data ) != 0 )
  {
    DBG_ERROR( "IpAdapterImplWindows::DoPreIntialize:%d# WSAStartup FAILED", __LINE__ );
    ret_status = ErrCode::ERR; goto exit_label_;
  }

  select_timeout_ = 1;   // 1 second

exit_label_:
  DBG_INFO( "IpAdapterImplWindows::DoPreIntialize:%d# EXIT status %d", __LINE__, (int) ret_status );

  return ( ret_status );
}

ErrCode IpAdapterImplWindows::DoPostStartServer()
{
  GUID                  GuidWSARecvMsg    = WSAID_WSARECVMSG;
  DWORD                 copied            = 0;
  UdpSocketImplWindows *udp_socket        = nullptr;
  auto                  ip_adapter_config = ConfigManager::Inst().get_ip_adapter_config();

  if( !ip_adapter_config->is_ipv4_enabled() && !ip_adapter_config->is_ipv6_enabled() )
  {
    return ( ErrCode::ERR );
  }

  if( ip_adapter_config->is_ipv4_enabled() )
  {
    udp_socket = (UdpSocketImplWindows *) ipv4_unicast_socket_;
  }
  else if( ip_adapter_config->is_ipv6_enabled() )
  {
    udp_socket = (UdpSocketImplWindows *) ipv6_unicast_socket_;
  }

  if( udp_socket != nullptr )
  {
    if( 0 != WSAIoctl( udp_socket->getSocket(), SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidWSARecvMsg, sizeof( GuidWSARecvMsg ), &( wsa_recv_msg_ ), sizeof( wsa_recv_msg_ ), &copied, 0, 0 ) )
    {
      return ( ErrCode::ERR );
    }
  }
  else
  {
    DBG_ERROR( "IpAdapterImplWindows::DoPostStartServer:%d# socket NULL", __LINE__ );
  }

  return ( ErrCode::OK );
}

ErrCode IpAdapterImplWindows::DoPreStopServer()
{
  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "IpAdapterImplWindows::DoPreStopServer:%d# ENTER", __LINE__ );

  // receive thread will stop immediately.
  if( !WSASetEvent( shutdown_event_ ) )
  {
    DBG_ERROR( "IpAdapterImplWindows::DoPreStopServer:%d# FAILED to signal the shutdown event", __LINE__ );
    ret_status = ErrCode::ERR; goto exit_label_;
  }

exit_label_:
  DBG_INFO( "IpAdapterImplWindows::DoPreStopServer:%d# EXIT status %d", __LINE__, (int) ret_status );

  return ( ret_status );
}

void IpAdapterImplWindows::DoInitFastShutdownMechanism()
{
  DBG_INFO( "IpAdapterImplWindows::DoInitFastShutdownMechanism:%d# ENTER", __LINE__ );
  // create the event for fast shutdown notification
  shutdown_event_ = WSACreateEvent();

  if( shutdown_event_ != WSA_INVALID_EVENT )
  {
    select_timeout_ = -1;
  }

  DBG_INFO( "IpAdapterImplWindows::DoInitFastShutdownMechanism:%d# EXIT", __LINE__ );
}

/***
 * In windows platform there are many sockets opened and because of that threads are used.
 * In threaded environment this api is not used.
 */
void IpAdapterImplWindows::ReadData()
{
  DBG_ERROR( "IpAdapterImplWindows::ReadData:%d# ERROR ReadData should not be called for WINDOWS platform", __LINE__ );
}

void IpAdapterImplWindows::DoInitAddressChangeNotifyMechanism()
{
  // create the event for interface address change notification
  addr_change_event_ = WSACreateEvent();
}

void IpAdapterImplWindows::DoUnInitAddressChangeNotifyMechanism()
{
  if( addr_change_event_ != WSA_INVALID_EVENT )
  {
    WSACloseEvent( addr_change_event_ );
    addr_change_event_ = WSA_INVALID_EVENT;
  }
}


/**********************************************************************************************************************/
/**************************************************PRIVATE FUNCTIONS***************************************************/
/**********************************************************************************************************************/

#define ADD_SOCKET_TO_EVENT_ARRAY( SOCKET ) if( SOCKET != nullptr ) \
  { \
    AddSocketToEventArray( (UdpSocketImplWindows *) SOCKET ); \
  }

void IpAdapterImplWindows::DoHandleReceive()
{
  DWORD wsa_event_index = WSA_WAIT_EVENT_0;

  wsa_events_count_ = 0;

  ADD_SOCKET_TO_EVENT_ARRAY( ipv6_unicast_socket_ );
  ADD_SOCKET_TO_EVENT_ARRAY( ipv6_unicast_secure_socket_ );
  ADD_SOCKET_TO_EVENT_ARRAY( ipv4_unicast_socket_ );
  ADD_SOCKET_TO_EVENT_ARRAY( ipv4_unicast_secure_socket_ );
  ADD_SOCKET_TO_EVENT_ARRAY( ipv6_multicast_socket_ );
  ADD_SOCKET_TO_EVENT_ARRAY( ipv6_multicast_secure_socket_ );
  ADD_SOCKET_TO_EVENT_ARRAY( ipv4_multicast_socket_ );
  ADD_SOCKET_TO_EVENT_ARRAY( ipv4_multicast_secure_socket_ );

  if( shutdown_event_ != WSA_INVALID_EVENT )
  {
    wsa_events_array_[wsa_events_count_++] = shutdown_event_;
  }

  if( addr_change_event_ != WSA_INVALID_EVENT )
  {
    wsa_events_array_[wsa_events_count_++] = addr_change_event_;
  }

  while( !is_terminated_ )
  {
    DWORD received_event_hndl = WSAWaitForMultipleEvents( wsa_events_count_, wsa_events_array_, FALSE, WSA_INFINITE, FALSE );

    /* check the events within the range i.e 0 to max events count */
    if( !( ( received_event_hndl >= WSA_WAIT_EVENT_0 ) && ( received_event_hndl < ( WSA_WAIT_EVENT_0 + wsa_events_count_ ) ) ) )
    {
      continue;
    }

    DBG_INFO( "IpAdapterImplWindows::DoHandleReceive:%d# received event hndl[%d]", __LINE__, (int) received_event_hndl );

    switch( received_event_hndl )
    {
      case WSA_WAIT_FAILED:
      case WSA_WAIT_IO_COMPLETION:
      case WSA_WAIT_TIMEOUT:
      {
        break;
      }

      default:
      {
        wsa_event_index = received_event_hndl - WSA_WAIT_EVENT_0;

        DBG_INFO( "IpAdapterImplWindows::DoHandleReceive:%d# received event index[%d]", __LINE__, (int) wsa_event_index );

        if( ( wsa_event_index >= 0 ) && ( wsa_event_index < wsa_events_count_ ) )
        {
          /* reset the received event */
          if( false == WSAResetEvent( wsa_events_array_[wsa_event_index] ) )
          {
          }

          /* check for the address change event is received */
          if( ( addr_change_event_ != WSA_INVALID_EVENT ) && ( wsa_events_array_[wsa_event_index] == addr_change_event_ ) )
          {
            DBG_INFO( "IpAdapterImplWindows::DoHandleReceive:%d# received addr change event", __LINE__ );
            HandleAddressChangeEvent();
            break;
          }

          /* check for the shutdown event is received */
          if( ( shutdown_event_ != WSA_INVALID_EVENT ) && ( wsa_events_array_[wsa_event_index] == shutdown_event_ ) )
          {
            DBG_INFO( "IpAdapterImplWindows::DoHandleReceive:%d# received shutdown event", __LINE__ );
            break;
          }

          /* if it comes here then it is socket only */
          auto network_flag = GetNetworkFlagForSocket( socket_fd_array_[wsa_event_index] );

          DBG_INFO( "IpAdapterImplWindows::DoHandleReceive:%d# received socket event at socket[%d], nw_flag[0x%x]", __LINE__, (int) socket_fd_array_[wsa_event_index], (int) network_flag );

          HandleReceivedSocketData( socket_fd_array_[wsa_event_index], network_flag );
        }

        break;
      }
    }
  }

  for( int i = 0; i < wsa_events_count_; i++ )
  {
    if( wsa_events_array_[i] != addr_change_event_ )
    {
      WSACloseEvent( wsa_events_array_[i] );
    }
  }

  if( is_terminated_ )
  {
    shutdown_event_ = WSA_INVALID_EVENT;
    WSACleanup();
  }
}

void IpAdapterImplWindows::AddSocketToEventArray( UdpSocketImplWindows *udp_socket )
{
  DBG_INFO( "IpAdapterImplWindows::AddSocketToEventArray:%d# ENTER udp_socket[%p]", __LINE__, udp_socket );

  if( ( udp_socket != nullptr ) && ( udp_socket->getSocket() != INVALID_SOCKET ) )
  {
    auto wsa_new_event = WSACreateEvent();

    if( WSA_INVALID_EVENT != wsa_new_event )
    {
      if( WSAEventSelect( udp_socket->getSocket(), wsa_new_event, FD_READ ) != 0 )
      {
        WSACloseEvent( wsa_new_event );
      }
      else
      {
        wsa_events_array_[wsa_events_count_] = wsa_new_event;
        socket_fd_array_[wsa_events_count_]  = udp_socket->getSocket();
        wsa_events_count_++;
      }
    }
  }
  else
  {
    DBG_ERROR( "IpAdapterImplWindows::AddSocketToEventArray:%d# FAILED invalid args passed", __LINE__ );
  }

  DBG_INFO( "IpAdapterImplWindows::AddSocketToEventArray:%d# EXIT", __LINE__ );
}

ErrCode IpAdapterImplWindows::DoHandleInterfaceEvent( InterfaceEvent *interface_event )
{
  DBG_INFO( "IpAdapterImplWindows::DoHandleInterfaceEvent:%d# ENTER interface_event[%p]", __LINE__, interface_event );

  if( interface_event == nullptr )
  {
    goto exit_label_;
  }

  DBG_INFO( "IpAdapterImplWindows::DoHandleInterfaceEvent:%d# notify the interface modified", __LINE__ );

  /* notify about the interface change*/
  if( interface_event->getInterfaceEventType() == InterfaceEventType::kInterfaceModified )
  {
    WSASetEvent( addr_change_event_ );
  }

exit_label_:
  DBG_INFO( "IpAdapterImplWindows::DoHandleInterfaceEvent:%d# EXIT", __LINE__ );

  return ( ErrCode::OK );
}

void IpAdapterImplWindows::HandleAddressChangeEvent()
{
  DBG_INFO( "IpAdapterImplWindows::HandleAddressChangeEvent:%d# ENTER", __LINE__ );
  auto interface_monitor = INetworkPlatformFactory::GetCurrFactory()->GetInterfaceMonitor();

  if( interface_monitor )
  {
    ja_iot::base::StaticPtrArray<InterfaceAddress *, 10> if_addr_array{};
    interface_monitor->GetNewlyFoundInterface( if_addr_array );

    DBG_INFO( "IpAdapterImplWindows::HandleAddressChangeEvent:%d# new if_addr count[%d]", __LINE__, if_addr_array.Count() );

    if( if_addr_array.Count() > 0 )
    {
      for( int i = 0; i < if_addr_array.Count(); i++ )
      {
        auto if_addr = if_addr_array.GetItem( i );

        if( if_addr != nullptr )
        {
          if( if_addr->getFamily() == IpAddrFamily::IPV4 )
          {
            start_ipv4_mcast_at_interface( if_addr->getIndex() );
          }

          if( if_addr->getFamily() == IpAddrFamily::IPv6 )
          {
            start_ipv6_mcast_at_interface( if_addr->getIndex() );
          }
        }
      }
    }
  }
  else
  {
    DBG_INFO( "IpAdapterImplWindows::HandleAddressChangeEvent:%d# interface monitor NULL", __LINE__ );
  }

  DBG_INFO( "IpAdapterImplWindows::HandleAddressChangeEvent:%d# EXIT", __LINE__ );
}

void IpAdapterImplWindows::HandleReceivedSocketData( SOCKET socket_fd, uint16_t u16_network_flag )
{
  int                     level          = 0;
  int                     msg_type       = 0;
  int                     namelen        = 0;
  bool                    is_ipv6_packet = IsBitSet( u16_network_flag, kNetworkFlag_ipv6 );
  unsigned char *         pktinfo        = NULL;
  struct sockaddr_storage packet_src_addr {};

  packet_src_addr.ss_family = 0;

  DBG_INFO( "IpAdapterImplWindows::HandleReceivedSocketData:%d# ENTER socket[%d], nw_flag[0x%x]", __LINE__, (int) socket_fd, (int) u16_network_flag );

  union control
  {
    WSACMSGHDR cmsg;
    uint8_t    data[WSA_CMSG_SPACE( sizeof( IN6_PKTINFO ) )];
  } cmsg{};

  memset( &cmsg, 0, sizeof( cmsg ) );

  if( is_ipv6_packet )
  {
    namelen  = sizeof( struct sockaddr_in6 );
    level    = IPPROTO_IPV6;
    msg_type = IPV6_PKTINFO;
  }
  else
  {
    namelen  = sizeof( struct sockaddr_in );
    level    = IPPROTO_IP;
    msg_type = IP_PKTINFO;
  }

  WSABUF iov{};
  WSAMSG msg{};

  uint8_t *recvBuffer = (uint8_t*)mnew_g (COAP_MAX_PDU_SIZE);

  iov.len = COAP_MAX_PDU_SIZE;
  iov.buf = (char *) recvBuffer;

  msg.name          = (PSOCKADDR) &packet_src_addr;
  msg.namelen       = namelen;
  msg.lpBuffers     = &iov;
  msg.dwBufferCount = 1;
  msg.Control.buf   = (char *) cmsg.data;
  msg.Control.len   = sizeof( cmsg );

  uint32_t recvLen = 0;
  int      ret     = wsa_recv_msg_( socket_fd, &msg, (LPDWORD) &recvLen, 0, 0 );

  if( SOCKET_ERROR == ret )
  {
    DBG_ERROR( "IpAdapterImplWindows::HandleReceivedSocketData:%d# error reading socket, error[%d], errno[%d]", __LINE__, ret, WSAGetLastError() );
    mdelete_g(recvBuffer);
    return;
  }

  for( WSACMSGHDR *cmp = WSA_CMSG_FIRSTHDR( &msg ); cmp != NULL; cmp = WSA_CMSG_NXTHDR( &msg, cmp ) )
  {
    if( ( cmp->cmsg_level == level ) && ( cmp->cmsg_type == msg_type ) )
    {
      pktinfo = WSA_CMSG_DATA( cmp );
    }
  }

  if( !pktinfo )
  {
    DBG_ERROR( "IpAdapterImplWindows::HandleReceivedSocketData:%d# no packet info received", __LINE__ );
    mdelete_g(recvBuffer);
    return;
  }

  unsigned char ascii_addr[70];
  uint16_t      u16_port;
  Endpoint endpoint{};

  if( packet_src_addr.ss_family == AF_INET )
  {
    struct sockaddr_in *p_ipv4_addr = (struct sockaddr_in *) &packet_src_addr;
    inet_ntop( AF_INET, (void *) &p_ipv4_addr->sin_addr, (char *) &ascii_addr[0], sizeof( ascii_addr ) );
    u16_port = p_ipv4_addr->sin_port;
    endpoint.get_addr()->set_addr((uint8_t *)&p_ipv4_addr->sin_addr, IpAddrFamily::IPV4);
  }
  else
  {
    struct sockaddr_in6 *p_ipv6_addr = (struct sockaddr_in6 *) &packet_src_addr;
    inet_ntop( AF_INET6, (void *) &p_ipv6_addr->sin6_addr, (char *) &ascii_addr[0], sizeof( ascii_addr ) );
    u16_port = p_ipv6_addr->sin6_port;
    endpoint.get_addr()->set_addr((uint8_t *)&p_ipv6_addr->sin6_addr, IpAddrFamily::IPv6);
  }

  DBG_INFO( "IpAdapterImplWindows::HandleReceivedSocketData:%d# received packet from %s", __LINE__, &ascii_addr[0] );

  auto if_index = ( is_ipv6_packet ) ? ( (struct in6_pktinfo *) pktinfo )->ipi6_ifindex : ( (struct in_pktinfo *) pktinfo )->ipi_ifindex;

  endpoint.set_adapter_type( kAdapterType_ip );
  endpoint.set_network_flags( u16_network_flag );
  endpoint.set_if_index( if_index );
  endpoint.set_port( u16_port );

  if( _adapterHandler )
  {
    AdapterEvent adapter_event( AdapterEventType::kPacketReceived );
    adapter_event.set_end_point( &endpoint );
    adapter_event.set_data( recvBuffer );
    adapter_event.set_data_length( recvLen );
    adapter_event.set_adapter_type( kAdapterType_ip );

    _adapterHandler->handle_adapter_event( &adapter_event );
  }
}

uint16_t IpAdapterImplWindows::GetNetworkFlagForSocket( SOCKET socket_fd )
{
  if( ( ipv4_unicast_socket_ != nullptr ) && ( socket_fd == ( (UdpSocketImplWindows *) ipv4_unicast_socket_ )->getSocket() ) )
  {
    return ( ( kNetworkFlag_ipv4 ) );
  }
  else if( ( ipv4_unicast_secure_socket_ != nullptr ) && ( socket_fd == ( (UdpSocketImplWindows *) ipv4_unicast_secure_socket_ )->getSocket() ) )
  {
    return ( kNetworkFlag_ipv4 | kNetworkFlag_secure );
  }
  else if( ( ipv6_unicast_socket_ != nullptr ) && ( socket_fd == ( (UdpSocketImplWindows *) ipv6_unicast_socket_ )->getSocket() ) )
  {
    return ( kNetworkFlag_ipv6 );
  }
  else if( ( ipv6_unicast_secure_socket_ != nullptr ) && ( socket_fd == ( (UdpSocketImplWindows *) ipv6_unicast_secure_socket_ )->getSocket() ) )
  {
    return ( kNetworkFlag_ipv6 | kNetworkFlag_secure );
  }
  else if( ( ipv4_multicast_socket_ != nullptr ) && ( socket_fd == ( (UdpSocketImplWindows *) ipv4_multicast_socket_ )->getSocket() ) )
  {
    return ( kNetworkFlag_ipv4 | kNetworkFlag_multicast );
  }
  else if( ( ipv4_multicast_secure_socket_ != nullptr ) && ( socket_fd == ( (UdpSocketImplWindows *) ipv4_multicast_secure_socket_ )->getSocket() ) )
  {
    return ( kNetworkFlag_ipv4 | kNetworkFlag_secure | kNetworkFlag_multicast );
  }
  else if( ( ipv6_multicast_socket_ != nullptr ) && ( socket_fd == ( (UdpSocketImplWindows *) ipv6_multicast_socket_ )->getSocket() ) )
  {
    return ( kNetworkFlag_ipv6 | kNetworkFlag_multicast );
  }
  else if( ( ipv4_multicast_secure_socket_ != nullptr ) && ( socket_fd == ( (UdpSocketImplWindows *) ipv4_multicast_secure_socket_ )->getSocket() ) )
  {
    return ( kNetworkFlag_ipv6 | kNetworkFlag_secure | kNetworkFlag_multicast );
  }
  else
  {
    return ( ( kNetworkFlag_none ) );
  }
}

void IpAdapterImplWindows::DoHandleSendMsg( IpAdapterQMsg *ip_adapter_q_msg )
{
  DBG_INFO( "IpAdapterImplWindows::DoHandleSendMsg:%d# ENTER data[%p], data_len[%d], mcast[%d], port[%d], adapter[%x], if_index[%d], nw_flag[%x]", __LINE__, ip_adapter_q_msg->_data,
    ip_adapter_q_msg->_dataLength,
    ip_adapter_q_msg->is_multicast,
    ip_adapter_q_msg->end_point_.get_port(),
    ip_adapter_q_msg->end_point_.get_adapter_type(),
    ip_adapter_q_msg->end_point_.get_if_index(),
    ip_adapter_q_msg->end_point_.get_network_flags() );

  auto network_flag      = ip_adapter_q_msg->end_point_.get_network_flags();
  auto is_secure         = IsBitSetInNetworkFlag( network_flag, kNetworkFlag_secure );
  auto ip_adapter_config = ConfigManager::Inst().get_ip_adapter_config();

  if( ip_adapter_q_msg->is_multicast )
  {
    ip_adapter_q_msg->end_point_.set_port( is_secure ? COAP_SECURE_PORT : COAP_PORT );

    auto interface_monitor = INetworkPlatformFactory::GetCurrFactory()->GetInterfaceMonitor();

    if( interface_monitor != nullptr )
    {
      ja_iot::base::StaticPtrArray<InterfaceAddress *, 10> if_addr_array{};
      interface_monitor->GetInterfaceAddrList( if_addr_array );

      if( if_addr_array.Count() > 0 )
      {
        if( ip_adapter_config->is_ipv4_enabled() && IsBitSetInNetworkFlag( network_flag, kNetworkFlag_ipv4 ) )
        {
          IpAddress ip_addr{ 224, 0, 1, 187 };

          for( int i = 0; i < if_addr_array.Count(); i++ )
          {
            auto if_addr = if_addr_array.GetItem( i );

            if( ( if_addr != nullptr ) && ( if_addr->getFamily() == IpAddrFamily::IPV4 ) && ( ipv4_unicast_socket_ != nullptr ) )
            {
              ipv4_unicast_socket_->SelectMulticastInterface( ip_addr, if_addr->getIndex() );
              ipv4_unicast_socket_->SendData( ip_addr, ip_adapter_q_msg->end_point_.get_port(), ip_adapter_q_msg->_data, ip_adapter_q_msg->_dataLength );
            }
          }
        }

        if( ip_adapter_config->is_ipv6_enabled() && IsBitSetInNetworkFlag( network_flag, kNetworkFlag_ipv6 ) )
        {
          if( ip_adapter_q_msg->end_point_.set_ipv6_addr_by_scope( ip_adapter_q_msg->end_point_.get_network_flags() ) )
          {
            IpAddress ip_addr{ IpAddrFamily::IPv6 };

            for( int i = 0; i < if_addr_array.Count(); i++ )
            {
              auto if_addr = if_addr_array.GetItem( i );

              if( ( if_addr != nullptr ) && ( if_addr->getFamily() == IpAddrFamily::IPv6 ) )
              {
                ipv6_unicast_socket_->SelectMulticastInterface( ip_addr, if_addr->getIndex() );
                SendData( (UdpSocketImplWindows *) ipv6_unicast_socket_, ip_adapter_q_msg->end_point_, ip_adapter_q_msg->_data, ip_adapter_q_msg->_dataLength );
              }
            }
          }
        }
      }
      else
      {
        DBG_WARN( "IpAdapterImplWindows::DoHandleSendMsg:%d# no interfaces to send data", __LINE__ );
      }
    }
    else
    {
      DBG_ERROR( "IpAdapterImplWindows::DoHandleSendMsg:%d# if_monitor NULL", __LINE__ );
    }
  }
  else // it is unicast
  {
    UdpSocketImplWindows *udp_socket = nullptr;

    if( ip_adapter_q_msg->end_point_.get_port() == 0 )
    {
      ip_adapter_q_msg->end_point_.set_port( is_secure ? COAP_SECURE_PORT : COAP_PORT );
    }

    if( ip_adapter_config->is_ipv4_enabled() && IsBitSetInNetworkFlag( network_flag, kNetworkFlag_ipv4 ) )
    {
      udp_socket = ( is_secure ) ? (UdpSocketImplWindows *) ipv4_unicast_secure_socket_ : (UdpSocketImplWindows *) ipv4_unicast_socket_;

      SendData( udp_socket, ip_adapter_q_msg->end_point_, ip_adapter_q_msg->_data, ip_adapter_q_msg->_dataLength );
    }

    if( ip_adapter_config->is_ipv6_enabled() && IsBitSetInNetworkFlag( network_flag, kNetworkFlag_ipv6 ) )
    {
      udp_socket = ( is_secure ) ? (UdpSocketImplWindows *) ipv6_unicast_secure_socket_ : (UdpSocketImplWindows *) ipv6_unicast_socket_;

      SendData( udp_socket, ip_adapter_q_msg->end_point_, ip_adapter_q_msg->_data, ip_adapter_q_msg->_dataLength );
    }
  }
}

static void convert_ascii_addr_to_ip_addr( const char *ascii_addr, IpAddress &ip_addr )
{
  if( ascii_addr == nullptr )
  {
    return;
  }

  struct addrinfo *addr_info_list = NULL;
  struct addrinfo  hints {};

  hints.ai_family = AF_UNSPEC;
  hints.ai_flags  = AI_NUMERICHOST;

  if( !getaddrinfo( ascii_addr, NULL, &hints, &addr_info_list ) )
  {
    if( addr_info_list[0].ai_family == AF_INET6 )
    {
      struct sockaddr_in6 *sock_addr = (struct sockaddr_in6 *) addr_info_list[0].ai_addr;

      memcpy( ip_addr.get_addr(), &sock_addr->sin6_addr, sizeof( struct in6_addr ) );
      ip_addr.set_addr_family( IpAddrFamily::IPv6 );
      ip_addr.set_scope_id( sock_addr->sin6_scope_id );
    }
    else
    {
      struct sockaddr_in *ipv4_addr = (struct sockaddr_in *) addr_info_list[0].ai_addr;

      memcpy( ip_addr.get_addr(), &ipv4_addr->sin_addr, sizeof( struct in_addr ) );
      ip_addr.set_addr_family( IpAddrFamily::IPV4 );
    }
  }

  if( NULL != addr_info_list )
  {
    freeaddrinfo( addr_info_list );
  }
}


void IpAdapterImplWindows::SendData( UdpSocketImplWindows *udp_socket, Endpoint &endpoint, const uint8_t *data, const uint16_t data_length )
{
  IpAddress ip_addr{};

  if( udp_socket != nullptr )
  {
    convert_ascii_addr_to_ip_addr( (const char *) endpoint.get_addr(), ip_addr );
    udp_socket->SendData( ip_addr, endpoint.get_port(), (uint8_t *) data, data_length );
  }
}
}
}
#endif/*_OS_WINDOWS_*/
