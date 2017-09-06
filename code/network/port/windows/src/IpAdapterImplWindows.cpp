/*
 * IpAdapterWindows.cpp
 *
 *  Created on: Jul 3, 2017
 *      Author: psammand
 */


#ifdef _OS_WINDOWS_

#include <IpAddress.h>
#include <IUdpSocket.h>
#include <port/windows/inc/UdpSocketImplWindows.h>
#include <cstdio>
#include <EndPoint.h>
#include <guiddef.h>
#include <in6addr.h>
#include <inaddr.h>
#include <IAdapter.h>
#include <InterfaceAddress.h>
#include <konstants.h>
#include <minwindef.h>
#include <mswsock.h>
#include <OsalError.h>
#include <psdk_inc/_ip_mreq1.h>
#include <psdk_inc/_ip_types.h>
#include <psdk_inc/_socket_types.h>
#include <psdk_inc/_wsadata.h>
#include <stddef.h>
#include <Task.h>
#include <TransportType.h>
//#include <windows/inc/PrimitiveDataTypes.h>
#include <winerror.h>
#include <ws2ipdef.h>
#include <ws2tcpip.h>
#include <climits>
#include <cstdint>

#include <winsock2.h>
#include "IpAdapterBase.h"
#include "AdapterProxy.h"
#include "port/windows/inc/IpAdapterImplWindows.h"
#include "OsalMgr.h"
#include <IInterfaceMonitor.h>
#include <PtrArray.h>
#include <INetworkPlatformFactory.h>

using namespace ja_iot::osal;

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
  WSADATA wsa_data{};

  if( WSAStartup( MAKEWORD( 2, 2 ), &wsa_data ) != 0 )
  {
    return ( ErrCode::ERR );
  }

  select_timeout_ = 1;   // 1 second

  return ( ErrCode::OK );
}

ErrCode IpAdapterImplWindows::DoPostStartServer()
{
  GUID                  GuidWSARecvMsg = WSAID_WSARECVMSG;
  DWORD                 copied         = 0;
  UdpSocketImplWindows *udp_socket     = nullptr;

  if( !is_ipv4_enabled_ && !is_ipv6_enabled_ )
  {
    return ( ErrCode::ERR );
  }

  if( is_ipv4_enabled_ )
  {
    udp_socket = (UdpSocketImplWindows *) ipv4_unicast_socket_;
  }
  else if( is_ipv6_enabled_ )
  {
    udp_socket = (UdpSocketImplWindows *) ipv6_unicast_socket_;
  }

  if( 0 != WSAIoctl( udp_socket->getSocket(), SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidWSARecvMsg, sizeof( GuidWSARecvMsg ), &( wsa_recv_msg_ ), sizeof( wsa_recv_msg_ ), &copied, 0, 0 ) )
  {
    return ( ErrCode::ERR );
  }

  return ( ErrCode::OK );
}

ErrCode IpAdapterImplWindows::DoPreStopServer()
{
  // receive thread will stop immediately.
  if( !WSASetEvent( shutdown_event_ ) )
  {
  }

  return ( ErrCode::OK );
}

void IpAdapterImplWindows::DoInitFastShutdownMechanism()
{
  // create the event for fast shutdown notification
  shutdown_event_ = WSACreateEvent();

  if( shutdown_event_ != WSA_INVALID_EVENT )
  {
    select_timeout_ = -1;
  }
}

void IpAdapterImplWindows::ReadData()
{
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

void IpAdapterImplWindows::DoHandleReceive()
{
  DWORD wsa_event_index = WSA_WAIT_EVENT_0;

  wsa_events_count_ = 0;

  AddSocketToEventArray( (UdpSocketImplWindows *) ipv6_unicast_socket_ );
  AddSocketToEventArray( (UdpSocketImplWindows *) ipv6_unicast_secure_socket_ );
  AddSocketToEventArray( (UdpSocketImplWindows *) ipv4_unicast_socket_ );
  AddSocketToEventArray( (UdpSocketImplWindows *) ipv4_unicast_secure_socket_ );

  AddSocketToEventArray( (UdpSocketImplWindows *) ipv6_multicast_socket_ );
  AddSocketToEventArray( (UdpSocketImplWindows *) ipv6_multicast_secure_socket_ );
  AddSocketToEventArray( (UdpSocketImplWindows *) ipv4_multicast_socket_ );
  AddSocketToEventArray( (UdpSocketImplWindows *) ipv4_multicast_secure_socket_ );

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
    DWORD ret_status = WSAWaitForMultipleEvents( wsa_events_count_, wsa_events_array_, FALSE, WSA_INFINITE, FALSE );

    if( !( ( ret_status >= WSA_WAIT_EVENT_0 ) && ( ret_status < ( WSA_WAIT_EVENT_0 + wsa_events_count_ ) ) ) )
    {
      continue;
    }

    switch( ret_status )
    {
      case WSA_WAIT_FAILED:
      case WSA_WAIT_IO_COMPLETION:
      case WSA_WAIT_TIMEOUT:
      {
        break;
      }

      default:
      {
        wsa_event_index = ret_status - WSA_WAIT_EVENT_0;

        if( ( wsa_event_index >= 0 ) && ( wsa_event_index < wsa_events_count_ ) )
        {
          if( false == WSAResetEvent( wsa_events_array_[wsa_event_index] ) )
          {
          }

          if( ( addr_change_event_ != WSA_INVALID_EVENT ) && ( wsa_events_array_[wsa_event_index] == addr_change_event_ ) )
          {
            HandleAddressChangeEvent();
            break;
          }

          if( ( shutdown_event_ != WSA_INVALID_EVENT ) && ( wsa_events_array_[wsa_event_index] == shutdown_event_ ) )
          {
            break;
          }

          auto network_flag = GetNetworkFlagForSocket( socket_fd_array_[wsa_event_index] );

          HandleReceivedSocketData( socket_fd_array_[wsa_event_index], (NetworkFlag) network_flag );
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
  if( udp_socket != nullptr )
  {
    auto socket_fd = udp_socket->getSocket();

    if( socket_fd != INVALID_SOCKET )
    {
      auto wsa_new_event = WSACreateEvent();

      if( WSA_INVALID_EVENT != wsa_new_event )
      {
        if( WSAEventSelect( socket_fd, wsa_new_event, FD_READ ) != 0 )
        {
          WSACloseEvent( wsa_new_event );
        }
        else
        {
          wsa_events_array_[wsa_events_count_] = wsa_new_event;
          socket_fd_array_[wsa_events_count_]  = socket_fd;
          wsa_events_count_++;
        }
      }
    }
  }
}

ErrCode IpAdapterImplWindows::DoHandleInterfaceEvent( InterfaceEvent *interface_event )
{
  if( interface_event == nullptr )
  {
    return ( ErrCode::OK );
  }

  if( interface_event->getInterfaceEventType() == InterfaceEventType::kInterfaceModified )
  {
    WSASetEvent( addr_change_event_ );
  }

  return ( ErrCode::OK );
}

void IpAdapterImplWindows::HandleAddressChangeEvent()
{
  auto interface_monitor = INetworkPlatformFactory::GetCurrFactory()->GetInterfaceMonitor();

  if( interface_monitor )
  {
    ja_iot::base::StaticPtrArray<InterfaceAddress *, 10> if_addr_array{};
    interface_monitor->GetNewlyFoundInterface( if_addr_array );

    if( if_addr_array.Count() > 0 )
    {
      for( auto &if_addr : if_addr_array )
      {
        if( if_addr != nullptr )
        {
          if( if_addr->getFamily() == IpAddrFamily::IPV4 )
          {
            StartIpv4MulticastAtInterface( if_addr->getIndex() );
          }

          if( if_addr->getFamily() == IpAddrFamily::IPv6 )
          {
            StartIpv6MulticastAtInterface( if_addr->getIndex() );
          }
        }
      }
    }
  }
}

void IpAdapterImplWindows::HandleReceivedSocketData( SOCKET socket_fd, NetworkFlag network_flag )
{
  uint8_t recvBuffer[COAP_MAX_PDU_SIZE] = { 0 };
  int level{ 0 };
  int msg_type{ 0 };
  int namelen{ 0 };
  bool                    is_ipv6_packet = IsBitSetInNetworkFlag( network_flag, NetworkFlag::IPV6 );
  unsigned char *         pktinfo        = NULL;
  struct sockaddr_storage packet_src_addr {};

  packet_src_addr.ss_family = 0;

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

  iov.len = sizeof( recvBuffer );
  iov.buf = (char *) &recvBuffer[0];

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
    return;
  }

  unsigned char ascii_addr[70];

  if( packet_src_addr.ss_family == AF_INET )
  {
    struct sockaddr_in *p_ipv4_addr = (struct sockaddr_in *) &packet_src_addr;
    inet_ntop( AF_INET, (void *) &p_ipv4_addr->sin_addr, (char *) &ascii_addr[0], sizeof( ascii_addr ) );
  }
  else
  {
    struct sockaddr_in6 *p_ipv6_addr = (struct sockaddr_in6 *) &packet_src_addr;
    inet_ntop( AF_INET6, (void *) &p_ipv6_addr->sin6_addr, (char *) &ascii_addr[0], sizeof( ascii_addr ) );
  }

  printf( "Received packet from %s\n", &ascii_addr[0] );

  auto if_index = ( is_ipv6_packet ) ? ( (struct in6_pktinfo *) pktinfo )->ipi6_ifindex : ( (struct in_pktinfo *) pktinfo )->ipi_ifindex;

  Endpoint endpoint{};

  endpoint.setAdapterType( AdapterType::IP );
  endpoint.setNetworkFlags( network_flag );
  endpoint.setIfIndex( if_index );
  // TODO
  // need to set the received address in endpoint

  if( _adapterHandler )
  {
	  AdapterEvent adapter_event(AdapterEventType::kPacketReceived);
	  adapter_event.set_end_point(&endpoint);
	  adapter_event.set_data(recvBuffer);
	  adapter_event.set_data_length(recvLen);

    _adapterHandler->handle_adapter_event(&adapter_event);
  }
}

uint16_t IpAdapterImplWindows::GetNetworkFlagForSocket( SOCKET socket_fd )
{
  if( socket_fd == ( (UdpSocketImplWindows *) ipv4_unicast_socket_ )->getSocket() )
  {
    return ( (uint16_t) ( NetworkFlag::IPV4 ) );
  }
  else if( socket_fd == ( (UdpSocketImplWindows *) ipv4_unicast_secure_socket_ )->getSocket() )
  {
    return ( (uint16_t) NetworkFlag::IPV4 | (uint16_t) NetworkFlag::SECURE );
  }
  else if( socket_fd == ( (UdpSocketImplWindows *) ipv6_unicast_socket_ )->getSocket() )
  {
    return ( (uint16_t) NetworkFlag::IPV6 );
  }
  else if( socket_fd == ( (UdpSocketImplWindows *) ipv6_unicast_secure_socket_ )->getSocket() )
  {
    return ( (uint16_t) NetworkFlag::IPV6 | (uint16_t) NetworkFlag::SECURE );
  }
  else if( socket_fd == ( (UdpSocketImplWindows *) ipv4_multicast_socket_ )->getSocket() )
  {
    return ( (uint16_t) NetworkFlag::IPV4 | (uint16_t) NetworkFlag::MULTICAST );
  }
  else if( socket_fd == ( (UdpSocketImplWindows *) ipv4_multicast_secure_socket_ )->getSocket() )
  {
    return ( (uint16_t) NetworkFlag::IPV4 | (uint16_t) NetworkFlag::SECURE | (uint16_t) NetworkFlag::MULTICAST );
  }
  else if( socket_fd == ( (UdpSocketImplWindows *) ipv6_multicast_socket_ )->getSocket() )
  {
    return ( (uint16_t) NetworkFlag::IPV6 | (uint16_t) NetworkFlag::MULTICAST );
  }
  else if( socket_fd == ( (UdpSocketImplWindows *) ipv4_multicast_secure_socket_ )->getSocket() )
  {
    return ( (uint16_t) NetworkFlag::IPV6 | (uint16_t) NetworkFlag::SECURE | (uint16_t) NetworkFlag::MULTICAST );
  }
  else
  {
    return ( (uint16_t) ( NetworkFlag::DEFAULT ) );
  }
}

// ஒவ்வொரு port இக்கும் இந்த function ஐ செயல்படுத்த வேண்டும்.
void IpAdapterImplWindows::DoHandleSendMsg( IpAdapterQMsg *ip_adapter_q_msg )
{
  auto network_flag = ip_adapter_q_msg->end_point_.getNetworkFlags();
  auto is_secure    = IsBitSetInNetworkFlag( network_flag, NetworkFlag::SECURE );

  if( ip_adapter_q_msg->is_multicast )
  {
    ip_adapter_q_msg->end_point_.setPort( is_secure ? COAP_SECURE_PORT : COAP_PORT );

    auto interface_monitor = INetworkPlatformFactory::GetCurrFactory()->GetInterfaceMonitor();

    if( interface_monitor != nullptr )
    {
      ja_iot::base::StaticPtrArray<InterfaceAddress *, 10> if_addr_array{};
      interface_monitor->GetInterfaceAddrList( if_addr_array );

      if( if_addr_array.Count() > 0 )
      {
        if( is_ipv4_enabled_ && IsBitSetInNetworkFlag( network_flag, NetworkFlag::IPV4 ) )
        {
          IpAddress ip_addr{ 224, 0, 1, 187 };

          for( auto &if_addr : if_addr_array )
          {
            if( ( if_addr != nullptr ) && ( if_addr->getFamily() == IpAddrFamily::IPV4 ) )
            {
              ipv4_unicast_socket_->SelectMulticastInterface( ip_addr, if_addr->getIndex() );
              ipv4_unicast_socket_->SendData( ip_addr, ip_adapter_q_msg->end_point_.getPort(), ip_adapter_q_msg->_data, ip_adapter_q_msg->_dataLength );
            }
          }
        }

        if( is_ipv6_enabled_ && IsBitSetInNetworkFlag( network_flag, NetworkFlag::IPV6 ) )
        {
          if( ip_adapter_q_msg->end_point_.SetIpv6AddrByScope( ip_adapter_q_msg->end_point_.getNetworkFlags() ) )
          {
            IpAddress ip_addr{ IpAddrFamily::IPv6 };

            for( auto &if_addr : if_addr_array )
            {
              if( ( if_addr != nullptr ) && ( if_addr->getFamily() == IpAddrFamily::IPv6 ) )
              {
                ipv6_unicast_socket_->SelectMulticastInterface( ip_addr, if_addr->getIndex() );
                SendData( (UdpSocketImplWindows *) ipv6_unicast_socket_, ip_adapter_q_msg->end_point_, ip_adapter_q_msg->_data, ip_adapter_q_msg->_dataLength );
              }
            }
          }
        }
      }
    }
  }
  else // it is unicast
  {
    UdpSocketImplWindows *udp_socket = nullptr;

    if( ip_adapter_q_msg->end_point_.getPort() == 0 )
    {
      ip_adapter_q_msg->end_point_.setPort( is_secure ? COAP_SECURE_PORT : COAP_PORT );
    }

    if( is_ipv4_enabled_ && IsBitSetInNetworkFlag( network_flag, NetworkFlag::IPV4 ) )
    {
      udp_socket = ( is_secure ) ? (UdpSocketImplWindows *) ipv4_unicast_secure_socket_ : (UdpSocketImplWindows *) ipv4_unicast_socket_;

      SendData( udp_socket, ip_adapter_q_msg->end_point_, ip_adapter_q_msg->_data, ip_adapter_q_msg->_dataLength );
    }

    if( is_ipv6_enabled_ && IsBitSetInNetworkFlag( network_flag, NetworkFlag::IPV6 ) )
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

      memcpy( ip_addr.GetAddr(), &sock_addr->sin6_addr, sizeof( struct in6_addr ) );
      ip_addr.setAddrFamily( IpAddrFamily::IPv6 );
      ip_addr.setScopeId( sock_addr->sin6_scope_id );
    }
    else
    {
      struct sockaddr_in *ipv4_addr = (struct sockaddr_in *) addr_info_list[0].ai_addr;

      memcpy( ip_addr.GetAddr(), &ipv4_addr->sin_addr, sizeof( struct in_addr ) );
      ip_addr.setAddrFamily( IpAddrFamily::IPV4 );
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

  convert_ascii_addr_to_ip_addr( (const char *) endpoint.getAddr(), ip_addr );
  udp_socket->SendData( ip_addr, endpoint.getPort(), (uint8_t *) data, data_length );
}
}
}
#endif/*_OS_WINDOWS_*/
