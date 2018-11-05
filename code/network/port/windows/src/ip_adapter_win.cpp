/*
 * IpAdapterWindows.cpp
 *
 *  Created on: Jul 3, 2017
 *      Author: psammand
 */

#ifdef _OS_WINDOWS_

#include <cstring>
#include <vector>
#include <algorithm>
#include <cstdint>

#include <config_network.h>
#include <konstants_network.h>
#include <interface_addr.h>
#include <ip_addr.h>
#include <i_udp_socket.h>
#include <i_adapter.h>
#include <port/windows/inc/udp_socket_win.h>
#include <end_point.h>
#include <Task.h>
#include <base_consts.h>
#include <common/inc/ip_adapter_base.h>
#include <port/windows/inc/ip_adapter_win.h>
#include <i_interface_monitor.h>
#include <i_nwk_platform_factory.h>
#include <ip_adapter_config.h>
#include <common/inc/logging_network.h>
#include <common/inc/common_utils.h>
#include <IMemAllocator.h>
#include <config_mgr.h>
#include <base_utils.h>
#include "OsalMgr.h"

#ifdef __GNUC__
#include <guiddef.h>
#include <in6addr.h>
#include <inaddr.h>
#include <minwindef.h>
#include <mswsock.h>
#include <psdk_inc/_ip_mreq1.h>
#include <psdk_inc/_ip_types.h>
#include <psdk_inc/_socket_types.h>
#include <psdk_inc/_wsadata.h>
#include <winerror.h>
#include <ws2ipdef.h>
#include <ws2tcpip.h>
#include <winsock2.h>
#endif/*__GNUC__*/

#ifdef _MSC_VER
#include <MSWSock.h>
#include <WinSock2.h>
#include <ws2def.h>
#include <ws2ipdef.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>
#include <iptypes.h>

#endif /*_MSC_VER*/

#define __FILE_NAME__ "IpAdapterImplWindows"

using namespace ja_iot::osal;
using namespace ja_iot::base;

namespace ja_iot {
namespace network {
std::unique_ptr<IP_ADAPTER_ADDRESSES> get_adapters();
IpAdapterImplWindows::IpAdapterImplWindows ()
{
}
IpAdapterImplWindows::~IpAdapterImplWindows ()
{
}

ErrCode IpAdapterImplWindows::do_pre_intialize()
{
  auto ret_status = ErrCode::OK;

  DBG_INFO2( "ENTER" );

  WSADATA wsa_data{};

  if( WSAStartup( MAKEWORD( 2, 2 ), &wsa_data ) != 0 )
  {
    DBG_ERROR2( "WSAStartup FAILED" );
    ret_status = ErrCode::ERR;
    goto exit_label_;
  }

  select_timeout_ = 1;     // 1 second

exit_label_:
  DBG_INFO2( "EXIT status %d", int(ret_status) );

  return ( ret_status );
}

ErrCode IpAdapterImplWindows::do_post_start_server()
{
  GUID  guid_wsa_recv_msg = WSAID_WSARECVMSG;                // UUID to get the recv extension function
  DWORD copied            = 0;
  auto  temp_socket       = socket( AF_INET, SOCK_DGRAM, 0 );

  if( temp_socket == INVALID_SOCKET )
  {
    return ( ErrCode::ERR );
  }

  /* get the function pointer for receiving data from socket */
  auto ioctl_status = WSAIoctl( temp_socket,
      SIO_GET_EXTENSION_FUNCTION_POINTER,
      &guid_wsa_recv_msg,
      sizeof guid_wsa_recv_msg,
      &_pfn_wsa_recv_msg_cb,
      sizeof _pfn_wsa_recv_msg_cb,
      &copied, nullptr, nullptr );

  closesocket( temp_socket );

  if( ioctl_status == SOCKET_ERROR )
  {
    return ( ErrCode::ERR );
  }

  return ( ErrCode::OK );
}

ErrCode IpAdapterImplWindows::do_pre_stop_server()
{
  DBG_INFO2( "ENTER" );

  // receive thread will stop immediately.
  if( !WSASetEvent( _h_shutdown_event ) )
  {
    DBG_ERROR2( "FAILED to signal the shutdown event" );
    return ( ErrCode::ERR );
  }

  DBG_INFO2( "EXIT" );

  return ( ErrCode::OK );
}

void IpAdapterImplWindows::do_init_fast_shutdown_mechanism()
{
  DBG_INFO2( "ENTER" );
  // create the event for fast shutdown notification
  _h_shutdown_event = WSACreateEvent();

  DBG_INFO2( "EXIT" );
}

/***
 * In windows platform there are many sockets opened and because of that threads are used.
 * In threaded environment this api is not used.
 */
void IpAdapterImplWindows::read_data()
{
  DBG_ERROR2( "ERROR ReadData should not be called for WINDOWS platform" );
}

void IpAdapterImplWindows::do_init_address_change_notify_mechanism()
{
  _interface_monitor_socket = WSASocketW( AF_INET6, SOCK_DGRAM, 0, nullptr, 0, WSA_FLAG_OVERLAPPED );

  if( _interface_monitor_socket == INVALID_SOCKET )
  {
    DBG_ERROR2( "N/W IF change socket creation failed" );
    return;
  }

  // Put socket into dual IPv4/IPv6 mode.
  auto ipv6_only = FALSE;

  if( SOCKET_ERROR == setsockopt( _interface_monitor_socket,
    IPPROTO_IPV6, IPV6_V6ONLY,
    reinterpret_cast<char *>( &ipv6_only ),
    sizeof ipv6_only ) )
  {
    closesocket( _interface_monitor_socket );
    DBG_ERROR2( "N/W IF change socket option set failed" );
    return;
  }

  _interface_monitor_event.hEvent = CreateEvent( nullptr, TRUE, FALSE, nullptr );

  if( nullptr == _interface_monitor_event.hEvent )
  {
    closesocket( _interface_monitor_socket );
    DBG_ERROR2( "N/W IF change event creation failed" );
    return;
  }

  if( WSAEventSelect( _interface_monitor_socket,
    _interface_monitor_event.hEvent,
    FD_ADDRESS_LIST_CHANGE ) == SOCKET_ERROR )
  {
    DBG_ERROR2( "N/W IF change event binding failed" );
    return;
  }

  DWORD bytes_returned = 0;

  if( WSAIoctl( _interface_monitor_socket,
    SIO_ADDRESS_LIST_CHANGE, NULL, 0, NULL, 0,
    &bytes_returned, &_interface_monitor_event, NULL ) == SOCKET_ERROR )
  {
    DWORD err = GetLastError();

    if( err != ERROR_IO_PENDING )
    {
      DBG_ERROR2( "set SIO_ADDRESS_LIST_CHANGE failed" );
      return;
    }
  }
}

void IpAdapterImplWindows::do_un_init_address_change_notify_mechanism()
{
  if( _interface_monitor_event.hEvent != WSA_INVALID_EVENT )
  {
    WSACloseEvent( _interface_monitor_event.hEvent );
    _interface_monitor_event.hEvent = WSA_INVALID_EVENT;
  }

  if( _interface_monitor_socket != INVALID_SOCKET )
  {
    closesocket( _interface_monitor_socket );
    _interface_monitor_socket = INVALID_SOCKET;
  }
}

/**********************************************************************************************************************/
/**************************************************PRIVATE FUNCTIONS***************************************************/
/**********************************************************************************************************************/

void IpAdapterImplWindows::add_socket_to_event_array( UdpSocketImplWindows *udp_socket )
{
  DBG_INFO2( "ENTER pcz_udp_socket[%p]", udp_socket );

  if( ( udp_socket != nullptr ) && ( udp_socket->get_socket() != INVALID_SOCKET ) )
  {
    const auto wsa_new_event = WSACreateEvent();

    if( WSA_INVALID_EVENT != wsa_new_event )
    {
      if( WSAEventSelect( udp_socket->get_socket(), wsa_new_event, FD_READ ) != 0 )
      {
        WSACloseEvent( wsa_new_event );
      }
      else
      {
        _ah_wsa_events_array[_u16_wsa_events_count] = wsa_new_event;
        _ah_socket_fd_array[_u16_wsa_events_count]  = udp_socket->get_socket();
        _u16_wsa_events_count++;
      }
    }
  }
  else
  {
    DBG_ERROR2( "FAILED invalid args passed" );
  }

  DBG_INFO2( "EXIT" );
}


void IpAdapterImplWindows::do_handle_receive()
{
  auto wsa_event_index = WSA_WAIT_EVENT_0;

  _u16_wsa_events_count = 0;

  for( auto &socket : _sockets )
  {
    add_socket_to_event_array( (UdpSocketImplWindows *) socket );
  }

  /* add shutdown event to listening array */
  if( _h_shutdown_event != WSA_INVALID_EVENT )
  {
    _ah_wsa_events_array[_u16_wsa_events_count++] = _h_shutdown_event;
  }

  /* add address change event to listening array */
  if( _interface_monitor_event.hEvent != WSA_INVALID_EVENT )
  {
    _ah_wsa_events_array[_u16_wsa_events_count++] = _interface_monitor_event.hEvent;
  }

  /* main while loop for receiving the incoming msg_data */
  while( !is_terminated_ )
  {
    const auto dw_rcvd_event_hndl = WSAWaitForMultipleEvents( _u16_wsa_events_count, _ah_wsa_events_array, FALSE, WSA_INFINITE, FALSE );

    /* check the events within the range i.e 0 to max events count */
    if( !( ( dw_rcvd_event_hndl >= WSA_WAIT_EVENT_0 ) && ( dw_rcvd_event_hndl < WSA_WAIT_EVENT_0 + _u16_wsa_events_count ) ) )
    {
      continue;
    }

    // DBG_INFO2( "received event hndl[%d]", int(dw_rcvd_event_hndl) );

    switch( dw_rcvd_event_hndl )
    {
      case WSA_WAIT_FAILED:
      case WSA_WAIT_IO_COMPLETION:
      case WSA_WAIT_TIMEOUT:
      {
        break;
      }

      default:
      {
        wsa_event_index = dw_rcvd_event_hndl - WSA_WAIT_EVENT_0;

        // DBG_INFO2( "received event index[%d]", int(wsa_event_index) );

        if( ( wsa_event_index >= 0 ) && ( wsa_event_index < _u16_wsa_events_count ) )
        {
          /* reset the received event */
          if( false == WSAResetEvent( _ah_wsa_events_array[wsa_event_index] ) )
          {
          }

          /* check for the address change event is received */
          if( ( _interface_monitor_event.hEvent != WSA_INVALID_EVENT ) && ( _ah_wsa_events_array[wsa_event_index] == _interface_monitor_event.hEvent ) )
          {
            DBG_INFO2( "received addr change event" );
            handle_address_change_event();

            DWORD bytes_returned = 0;

            if( WSAIoctl( _interface_monitor_socket,
              SIO_ADDRESS_LIST_CHANGE, NULL, 0, NULL, 0,
              &bytes_returned, &_interface_monitor_event, NULL ) == SOCKET_ERROR )
            {
              DWORD err = GetLastError();

              if( err != ERROR_IO_PENDING )
              {
                DBG_ERROR2( "reset SIO_ADDRESS_LIST_CHANGE failed" );
              }
            }

            break;
          }

          /* check for the shutdown event is received */
          if( ( _h_shutdown_event != WSA_INVALID_EVENT ) && ( _ah_wsa_events_array[wsa_event_index] == _h_shutdown_event ) )
          {
            DBG_INFO2( "received shutdown event" );
            break;
          }

          /* if it comes here then it is socket data only */
          const auto u16_network_flag = get_network_flag_for_socket( _ah_socket_fd_array[wsa_event_index] );

          // DBG_INFO2( "received socket event at socket[%d], nw_flag[0x%x]", (int) _ah_socket_fd_array[wsa_event_index], (int) u16_network_flag );

          handle_received_socket_data( _ah_socket_fd_array[wsa_event_index], u16_network_flag );
        }

        break;
      }
    }
  }

  for( auto i = 0; i < _u16_wsa_events_count; i++ )
  {
    WSACloseEvent( _ah_wsa_events_array[i] );
  }

  if( is_terminated_ )
  {
    _h_shutdown_event = WSA_INVALID_EVENT;
    WSACleanup();
  }
}

std::vector<InterfaceAddress *> IpAdapterImplWindows::get_interface_address_for_index( uint8_t u8_index )
{
  std::vector<InterfaceAddress *> if_addr_array{};

  get_interface_address_list( if_addr_array );

  return ( if_addr_array );
}

void IpAdapterImplWindows::handle_received_socket_data( const SOCKET socket_fd, const uint16_t u16_network_flag )
{
  int                     level          = IPPROTO_IP;
  int                     msg_type       = IP_PKTINFO;
  int                     namelen        = sizeof( struct sockaddr_in );
  const auto              is_ipv6_packet = is_bit_set( u16_network_flag, k_network_flag_ipv6 );

  struct sockaddr_storage packet_src_addr {};

  packet_src_addr.ss_family = 0;

  DBG_INFO2( "ENTER socket[%d], nw_flag[0x%x]", int(socket_fd), int(u16_network_flag) );

  union control
  {
    WSACMSGHDR cmsg;
    uint8_t    data[WSA_CMSG_SPACE( sizeof( IN6_PKTINFO ) )];
  } cmsg{};

  memset( &cmsg, 0, sizeof cmsg );

  if( is_ipv6_packet )
  {
    namelen  = sizeof( struct sockaddr_in6 );
    level    = IPPROTO_IPV6;
    msg_type = IPV6_PKTINFO;
  }

  WSABUF iov;
  WSAMSG msg{};

  iov.len = COAP_MAX_PDU_SIZE;
  iov.buf = reinterpret_cast<char *>( &_pu8_receive_buffer[0] );

  msg.name          = PSOCKADDR( &packet_src_addr );
  msg.namelen       = namelen;
  msg.lpBuffers     = &iov;
  msg.dwBufferCount = 1;
  msg.Control.buf   = reinterpret_cast<char *>( cmsg.data );
  msg.Control.len   = sizeof cmsg;

  uint32_t   recv_len        = 0;
  const auto recv_msg_result = _pfn_wsa_recv_msg_cb( socket_fd, &msg, LPDWORD( &recv_len ), nullptr, nullptr );

  if( SOCKET_ERROR == recv_msg_result )
  {
    DBG_ERROR2( "error reading socket, error[%d], errno[%d]", recv_msg_result, WSAGetLastError() );
    return;
  }

  unsigned char *pu8_pkt_info{};

  for( auto cmp = WSA_CMSG_FIRSTHDR( &msg ); cmp != nullptr; cmp = WSA_CMSG_NXTHDR( &msg, cmp ) )
  {
    if( ( cmp->cmsg_level == level ) && ( cmp->cmsg_type == msg_type ) )
    {
      pu8_pkt_info = WSA_CMSG_DATA( cmp );
    }
  }

  if( !pu8_pkt_info )
  {
    DBG_ERROR2( "no packet info received" );
    return;
  }

  uint16_t u16_recvd_port;
  IpAddress ip_addr{};

  if( packet_src_addr.ss_family == AF_INET )
  {
    auto pst_ipv4_addr = reinterpret_cast<struct sockaddr_in *>( &packet_src_addr );
    u16_recvd_port = ntohs( pst_ipv4_addr->sin_port );
    ip_addr.set_addr( (uint8_t *) &pst_ipv4_addr->sin_addr, IpAddrFamily::IPv4 );
  }
  else
  {
    auto pst_ipv6_addr = reinterpret_cast<struct sockaddr_in6 *>( &packet_src_addr );
    u16_recvd_port = ntohs( pst_ipv6_addr->sin6_port );
    ip_addr.set_addr( (uint8_t *) &pst_ipv6_addr->sin6_addr, IpAddrFamily::IPv6 );
  }

#ifdef _DEBUG_
  unsigned char ac_ascii_addr[70];
  ip_addr.to_string( &ac_ascii_addr[0], 70 );

  DBG_INFO2( "received packet from %s", &ac_ascii_addr[0] );
#endif

  const auto u32_if_index = is_ipv6_packet
    ? reinterpret_cast<struct in6_pktinfo *>( pu8_pkt_info )->ipi6_ifindex
    : reinterpret_cast<struct in_pktinfo *>( pu8_pkt_info )->ipi_ifindex;


  if( this->_adapter_event_callback )
  {
    Endpoint endpoint{ k_adapter_type_ip, u16_network_flag, u16_recvd_port, u32_if_index, ip_addr };
    auto         received_data = new uint8_t[recv_len];
    memcpy( &received_data[0], &_pu8_receive_buffer[0], recv_len );

    AdapterEvent adapter_event( ADAPTER_EVENT_TYPE_PACKET_RECEIVED, &endpoint, &received_data[0], recv_len, k_adapter_type_ip );

    this->_adapter_event_callback( &adapter_event, _adapter_event_cb_data );
  }
}

uint16_t IpAdapterImplWindows::get_network_flag_for_socket( const SOCKET socket_fd ) const
{
  for( auto &socket : _sockets )
  {
    auto udp_socket = (UdpSocketImplWindows *) socket;

    if( udp_socket->get_socket() == socket_fd )
    {
      return ( udp_socket->get_flags() );
    }
  }

  return ( k_network_flag_none );
}

void IpAdapterImplWindows::do_handle_send_msg( IpAdapterQMsg *ip_adapter_q_msg )
{
  if( ip_adapter_q_msg == nullptr )
  {
    return;
  }

  auto       &endpoint        = ip_adapter_q_msg->end_point_;
  const auto pu8_data         = ip_adapter_q_msg->_data;
  const auto u16_data_len     = ip_adapter_q_msg->_dataLength;
  const auto network_flag     = endpoint.get_network_flags();
  const auto is_secure        = is_bit_set( network_flag, k_network_flag_secure );
  const auto is_ipv4_transfer = is_bit_set( network_flag, k_network_flag_ipv4 );
  const auto is_ipv6_transfer = is_bit_set( network_flag, k_network_flag_ipv6 );

  DBG_INFO2(
    "ENTER data_len[%d], mcast[%d], port[%d], adapter[%x], if_index[%d], nw_flag[%x]",
    u16_data_len,
    ip_adapter_q_msg->is_multicast,
    endpoint.get_port(),
    endpoint.get_adapter_type(),
    endpoint.get_if_index(),
    endpoint.get_network_flags() );

  if( ip_adapter_q_msg->is_multicast )
  {
    auto ipv4_socket = ( is_ipv4_transfer ) ? get_socket_by_mask( ( is_secure ) ? k_network_flag_ipv4_secure_ucast : k_network_flag_ipv4 ) : nullptr;
    auto ipv6_socket = ( is_ipv6_transfer ) ? get_socket_by_mask( ( is_secure ) ? k_network_flag_ipv6_secure_ucast : k_network_flag_ipv6 ) : nullptr;

    endpoint.set_port( is_secure ? COAP_SECURE_PORT : COAP_PORT );

    std::vector<InterfaceAddress *> if_addr_array = get_interface_address_for_index( 0 );

    for( auto &if_addr : if_addr_array )
    {
      if( ( ipv4_socket != nullptr ) && if_addr->is_ipv4() )
      {
        ipv4_socket->SelectMulticastInterface( endpoint.get_addr(), if_addr->get_index() );
        ipv4_socket->SendData( endpoint.get_addr(), endpoint.get_port(), pu8_data, u16_data_len );
      }

      if( ( ipv6_socket != nullptr ) && if_addr->is_ipv6() )
      {
        ipv6_socket->SelectMulticastInterface( endpoint.get_addr(), if_addr->get_index() );
        ipv6_socket->SendData( endpoint.get_addr(), endpoint.get_port(), pu8_data, u16_data_len );
      }
    }
  }
  else     // it is unicast
  {
    if( endpoint.get_port() == 0 )
    {
      endpoint.set_port( is_secure ? COAP_SECURE_PORT : COAP_PORT );
    }

    clear_bit( (uint16_t) network_flag, k_network_flag_multicast );
    auto socket = get_socket_by_mask( network_flag );

    if( socket != nullptr )
    {
      send_data( (UdpSocketImplWindows *) socket, endpoint, pu8_data, u16_data_len );
    }
  }
}

void IpAdapterImplWindows::send_data( UdpSocketImplWindows *pcz_udp_socket, Endpoint &endpoint, const uint8_t *data, const uint16_t data_length ) const
{
  if( pcz_udp_socket == nullptr )
  {
    return;
  }

  const auto send_status = pcz_udp_socket->SendData( endpoint.get_addr(), endpoint.get_port(), const_cast<uint8_t *>( data ), data_length );

  if( send_status != SocketError::OK )
  {
    AdapterEvent cz_adapter_event{ ADAPTER_EVENT_TYPE_ERROR, &endpoint, (uint8_t *) data, data_length, k_adapter_type_ip };
    cz_adapter_event.set_error_code( ErrCode::SEND_DATA_FAILED );

    if( _adapter_event_callback != nullptr )
    {
      _adapter_event_callback( &cz_adapter_event, _adapter_event_cb_data );
    }
  }
}

void IpAdapterImplWindows::get_interface_address_list( std::vector<InterfaceAddress *> &interface_addr_list )
{
  const auto p_adapters_list = get_adapters();

  if( !p_adapters_list )
  {
    DBG_ERROR2( "FAILED to get adapters list" );
    return;
  }

  for( IP_ADAPTER_ADDRESSES *p_cur_adapter_addr = p_adapters_list.get();
    p_cur_adapter_addr != nullptr;
    p_cur_adapter_addr = p_cur_adapter_addr->Next )
  {
    DBG_INFO2( "adapter index[%d], if_type[%d], if_status[%d]",
      uint32_t( p_cur_adapter_addr->IfIndex ),
      uint32_t( p_cur_adapter_addr->IfType ),
      p_cur_adapter_addr->OperStatus );

    if( ( p_cur_adapter_addr->IfType & IF_TYPE_SOFTWARE_LOOPBACK )
      || ( ( p_cur_adapter_addr->OperStatus & IfOperStatusUp ) == 0 ) )
    {
      continue;
    }

    for( auto p_unicast_adapter_addr = p_cur_adapter_addr->FirstUnicastAddress;
      p_unicast_adapter_addr != nullptr;
      p_unicast_adapter_addr = p_unicast_adapter_addr->Next )
    {
      if( p_unicast_adapter_addr->Address.lpSockaddr->sa_family == AF_INET )
      {
        char temp_addr_buf[64] = { 0 };

        auto p_ipv4_addr = reinterpret_cast<struct sockaddr_in *>( p_unicast_adapter_addr->Address.lpSockaddr );
        auto if_addr     = new InterfaceAddress{ (uint32_t) p_cur_adapter_addr->IfIndex, IFF_UP, IpAddrFamily::IPv4, (const char *) &p_ipv4_addr->sin_addr };
        inet_ntop( AF_INET, static_cast<void *>( &p_ipv4_addr->sin_addr ), (char *) &temp_addr_buf[0], sizeof( temp_addr_buf ) );
        DBG_INFO2( "Found address %s", &temp_addr_buf[0] );
        interface_addr_list.push_back( if_addr );
      }
    }

    struct sockaddr_in6 *v6addr = nullptr;

    for( auto p_unicast_adapter_addr = p_cur_adapter_addr->FirstUnicastAddress;
      p_unicast_adapter_addr != nullptr;
      p_unicast_adapter_addr = p_unicast_adapter_addr->Next )
    {
      if( p_unicast_adapter_addr->Address.lpSockaddr->sa_family == AF_INET6 )
      {
        struct sockaddr_in6 *addr = (struct sockaddr_in6 *) p_unicast_adapter_addr->Address.lpSockaddr;

        /* If we see a non link-local and non DNS_ELIGIBLE address, ignore it. */
        if( !IN6_IS_ADDR_LINKLOCAL( &addr->sin6_addr ) &&
          !( p_unicast_adapter_addr->Flags & IP_ADAPTER_ADDRESS_DNS_ELIGIBLE ) )
        {
          char dotname[NI_MAXHOST] = { 0 };
          getnameinfo( (const SOCKADDR *) addr, sizeof( struct sockaddr_in6 ), dotname, sizeof( dotname ), NULL, 0, NI_NUMERICHOST );
          DBG_INFO2( "%s is not IN6_IS_ADDR_LINKLOCAL and not IP_ADAPTER_ADDRESS_DNS_ELIGIBLE, skipped.\n", dotname );
          continue;
        }

        if( ( v6addr == nullptr ) && IN6_IS_ADDR_LINKLOCAL( &addr->sin6_addr ) )
        {
          v6addr = addr;
          break;
        }

        /* non link-local and DNS_ELIGIBLE address, */
        if( !IN6_IS_ADDR_LINKLOCAL( &addr->sin6_addr ) &&
          ( p_unicast_adapter_addr->Flags & IP_ADAPTER_ADDRESS_DNS_ELIGIBLE ) )
        {
          /* If this is the first address we're seeing, save it. */
          if( v6addr == nullptr )
          {
            v6addr = addr;
          }

          uint8_t b = addr->sin6_addr.u.Byte[0];

          if( !( ( b & 0xfc ) == b ) && !( ( b & 0xfd ) == b ) )
          {
            /* We've gotten a non-private global address
             * which we could use. So, break.
             */
            v6addr = addr;
            break;
          }
          else
          {
            /* We've gotten a private IPv6 address in global scope. */
            /* If the saved address is link-local, substitute that with this. */
            if( IN6_IS_ADDR_LINKLOCAL( &v6addr->sin6_addr ) )
            {
              v6addr = addr;
            }

            /* Process the remaining addresses on this interface to see if we
             * can find the global address assigned by our ISP. */
            continue;
          }
        }
      }
    }// for loop adapter addresses
		if (v6addr != nullptr)
		{
			char temp_addr_buf[64] = { 0 };
			auto if_addr = new InterfaceAddress{ (uint32_t)p_cur_adapter_addr->IfIndex, IFF_UP, IpAddrFamily::IPv6, (const char *)&v6addr->sin6_addr };
			inet_ntop(AF_INET6, static_cast<void *>(&v6addr->sin6_addr), (char *)&temp_addr_buf[0], sizeof(temp_addr_buf));
			DBG_INFO2("Found address %s", &temp_addr_buf[0]);
			interface_addr_list.push_back(if_addr);
		}
  }// for loop adapters
}

std::unique_ptr<IP_ADAPTER_ADDRESSES> get_adapters()
{
  ULONG                 u32_adapter_addr_buf_len = 0;
  IP_ADAPTER_ADDRESSES *p_adapter_adrr_start_ptr = nullptr;

  /*We don't need most of the default information, so optimize this call by not asking for them. */
  const ULONG           flags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_SKIP_FRIENDLY_NAME;

  DBG_INFO2( "ENTER" );

  /*
   *   Call up to 3 times: once to get the size, once to get the msg_data, and once more
   *   just in case there was an increase in length in between the first two. If the
   *   length is still increasing due to more addresses being added, even this may fail
   *   and we'll have to wait for the next IP address change notification.
   */
  for( int i = 0; i < 3; i++ )
  {
    DBG_INFO2( "Calling GetAdaptersAddresses [%d] times", i );
    const auto ret = GetAdaptersAddresses( AF_UNSPEC, flags, nullptr, p_adapter_adrr_start_ptr, &u32_adapter_addr_buf_len );

    if( ERROR_BUFFER_OVERFLOW == ret )
    {
      // Redo with updated length.
      if( p_adapter_adrr_start_ptr != nullptr )
      {
        delete[] p_adapter_adrr_start_ptr;
      }

      p_adapter_adrr_start_ptr = reinterpret_cast<IP_ADAPTER_ADDRESSES *>( new unsigned char[u32_adapter_addr_buf_len] );

      if( p_adapter_adrr_start_ptr == nullptr )
      {
        DBG_ERROR2( "FAILED to alloc memory" );
        break;
      }

      continue;
    }

    if( NO_ERROR != ret )
    {
      break;
    }

    DBG_INFO2( "EXIT SUCCESS" );
    // Succeeded getting adapters
    return ( std::unique_ptr<IP_ADAPTER_ADDRESSES>( p_adapter_adrr_start_ptr ) );
  }

  if( p_adapter_adrr_start_ptr != nullptr )
  {
    delete[] p_adapter_adrr_start_ptr;
  }

  DBG_INFO2( "EXIT FAILED" );

  return ( nullptr );
}
#if 0
static void convert_ascii_addr_to_ip_addr( const char *pc_ascii_addr, IpAddress &rcz_ip_addr )
{
  if( pc_ascii_addr == nullptr )
  {
    return;
  }

  struct addrinfo *addr_info_list {};

  struct addrinfo  hints {};

  hints.ai_family = AF_UNSPEC;
  hints.ai_flags  = AI_NUMERICHOST;

  if( !getaddrinfo( pc_ascii_addr, nullptr, &hints, &addr_info_list ) )
  {
    if( addr_info_list[0].ai_family == AF_INET6 )
    {
      auto pst_ipv6_sock_addr = reinterpret_cast<struct sockaddr_in6 *>( addr_info_list[0].ai_addr );

      memcpy( rcz_ip_addr.get_addr(), &pst_ipv6_sock_addr->sin6_addr, sizeof( struct in6_addr ) );
      rcz_ip_addr.set_addr_family( IpAddrFamily::IPv6 );
      rcz_ip_addr.set_scope_id( uint8_t( pst_ipv6_sock_addr->sin6_scope_id ) );
    }
    else
    {
      auto pst_ipv4_sock_addr = reinterpret_cast<struct sockaddr_in *>( addr_info_list[0].ai_addr );

      memcpy( rcz_ip_addr.get_addr(), &pst_ipv4_sock_addr->sin_addr, sizeof( struct in_addr ) );
      rcz_ip_addr.set_addr_family( IpAddrFamily::IPv4 );
    }
  }

  if( nullptr != addr_info_list )
  {
    freeaddrinfo( addr_info_list );
  }
}

#endif
}
}
#endif/*_OS_WINDOWS_*/
