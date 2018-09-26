/*
 * UdpSocketImplWindows.cpp
 *
 *  Created on: Jul 15, 2017
 *      Author: psammand
 */

#ifdef _OS_WINDOWS_

#include <winsock2.h>
#include <windows.h>
#include <ws2ipdef.h>
#include <ws2tcpip.h>
#include <climits>

#include "ip_addr.h"
#include "i_udp_socket.h"
#include "port/windows/inc/udp_socket_win.h"

namespace ja_iot {
namespace network {
using namespace base;
UdpSocketImplWindows::UdpSocketImplWindows ()
{
  WSADATA wsa_data{};

  WSAStartup( MAKEWORD( 2, 2 ), &wsa_data );
}

SocketError UdpSocketImplWindows::OpenSocket( const IpAddrFamily e_addr_family )
{
  _e_addr_family = e_addr_family;
  _u32_socket_fd = socket( e_addr_family == IpAddrFamily::IPv4 ? PF_INET : PF_INET6, SOCK_DGRAM, IPPROTO_UDP );

  return ( ( _u32_socket_fd == INVALID_SOCKET ) ? SocketError::SOCKET_NOT_VALID : SocketError::OK );
}

int select_sock_addr( IpAddrFamily e_addr_family, sockaddr * &pst_sock_addr, sockaddr_in *pst_ipv4_sock_addr, sockaddr_in6 *pst_ipv6_sock_addr, const uint16_t u16_port, IpAddress &ip_address )
{
  if( e_addr_family == IpAddrFamily::IPv4 )
  {
    pst_ipv4_sock_addr->sin_family           = AF_INET;
    pst_ipv4_sock_addr->sin_port             = htons( u16_port );
    pst_ipv4_sock_addr->sin_addr.S_un.S_addr = htonl( ip_address.as_u32() );             // already taken care about network byte order

    pst_sock_addr = reinterpret_cast<sockaddr *>( pst_ipv4_sock_addr );
    return ( sizeof( struct sockaddr_in ) );
  }
  else
  {
    pst_ipv6_sock_addr->sin6_family = AF_INET6;
    pst_ipv6_sock_addr->sin6_port   = htons( u16_port );
    memcpy( &pst_ipv6_sock_addr->sin6_addr, ip_address.get_addr(), sizeof pst_ipv6_sock_addr->sin6_addr );

    pst_sock_addr = reinterpret_cast<sockaddr *>( pst_ipv6_sock_addr );
    return ( sizeof( struct sockaddr_in6 ) );
  }
}

SocketError UdpSocketImplWindows::BindSocket( IpAddress &ip_address, const uint16_t u16_port )
{
  if( ( _u32_socket_fd == INVALID_SOCKET ) || ( _e_addr_family != ip_address.get_addr_family() ) )
  {
    return ( SocketError::SOCKET_NOT_VALID );
  }

  sockaddr *          pst_sock_addr     = nullptr;
  struct sockaddr_in  st_ipv4_sock_addr = { 0 };

  struct sockaddr_in6 st_ipv6_sock_addr = { 0 };

  int                 i32_sockaddr_len = select_sock_addr( _e_addr_family, pst_sock_addr, &st_ipv4_sock_addr, &st_ipv6_sock_addr, u16_port, ip_address );

  SocketError         ret_status = SocketError::OK;

  if( bind( _u32_socket_fd, pst_sock_addr, i32_sockaddr_len ) == SOCKET_ERROR )
  {
    auto const socket_error = WSAGetLastError();
    ret_status = SocketError::BIND_FAILED;
  }

  return ( ret_status );
}

SocketError UdpSocketImplWindows::CloseSocket()
{
  if( _u32_socket_fd != INVALID_SOCKET )
  {
    closesocket( _u32_socket_fd );
    _u32_socket_fd = INVALID_SOCKET;
  }

  return ( SocketError::OK );
}

SocketError UdpSocketImplWindows::JoinMulticastGroup( IpAddress &rcz_group_address, const uint32_t u32_if_index )
{
  if( _u32_socket_fd == INVALID_SOCKET )
  {
    return ( SocketError::SOCKET_NOT_VALID );
  }

  if( _e_addr_family == IpAddrFamily::IPv4 )
  {
    struct ip_mreq st_ipv4_mreq {};

    st_ipv4_mreq.imr_multiaddr.s_addr = htonl( rcz_group_address.as_u32() );
    st_ipv4_mreq.imr_interface.s_addr = htonl( u32_if_index );

    const auto ret_status = setsockopt( _u32_socket_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, reinterpret_cast<const char *>( &st_ipv4_mreq ), sizeof st_ipv4_mreq );

    if( ret_status )
    {
      // auto error = WSAGetLastError();
      return ( SocketError::OPTION_SET_FAILED );
    }
  }
  else
  {
    struct ipv6_mreq st_ipv6_mreq {};

    memcpy( &st_ipv6_mreq.ipv6mr_multiaddr, rcz_group_address.get_addr(), sizeof st_ipv6_mreq.ipv6mr_multiaddr );
    st_ipv6_mreq.ipv6mr_interface = u32_if_index;

    if( setsockopt( _u32_socket_fd, IPPROTO_IPV6, IPV6_JOIN_GROUP, reinterpret_cast<const char *>( &st_ipv6_mreq ), sizeof st_ipv6_mreq ) )
    {
      return ( SocketError::OPTION_SET_FAILED );
    }
  }

  return ( SocketError::OK );
}

SocketError UdpSocketImplWindows::LeaveMulticastGroup( IpAddress &rcz_group_address, const uint32_t u32_if_index )
{
  if( _u32_socket_fd == INVALID_SOCKET )
  {
    return ( SocketError::ERR );
  }

  if( _e_addr_family == IpAddrFamily::IPv4 )
  {
    struct ip_mreq st_ipv4_mreq {};

    st_ipv4_mreq.imr_multiaddr.s_addr = htonl( rcz_group_address.as_u32() );
    st_ipv4_mreq.imr_interface.s_addr = htonl( u32_if_index );

    if( setsockopt( _u32_socket_fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, reinterpret_cast<const char *>( &st_ipv4_mreq ), sizeof st_ipv4_mreq ) )
    {
      return ( SocketError::OPTION_SET_FAILED );
    }
  }
  else
  {
    struct ipv6_mreq st_ipv6_mreq {};

    st_ipv6_mreq.ipv6mr_interface = u32_if_index;
    memcpy( &st_ipv6_mreq.ipv6mr_multiaddr, rcz_group_address.get_addr(), sizeof st_ipv6_mreq.ipv6mr_multiaddr );

    if( setsockopt( _u32_socket_fd, IPPROTO_IPV6, IPV6_LEAVE_GROUP, reinterpret_cast<const char *>( &st_ipv6_mreq ), sizeof st_ipv6_mreq ) )
    {
      return ( SocketError::OPTION_SET_FAILED );
    }
  }

  return ( SocketError::OK );
}

SocketError UdpSocketImplWindows::SelectMulticastInterface( IpAddress &rcz_group_address, const uint32_t u32_if_index )
{
  if( _u32_socket_fd == INVALID_SOCKET )
  {
    return ( SocketError::ERR );
  }

  if( _e_addr_family == IpAddrFamily::IPv4 )
  {
    struct ip_mreq st_ipv4_mreq {};

    st_ipv4_mreq.imr_multiaddr.s_addr = htonl( rcz_group_address.as_u32() );
    st_ipv4_mreq.imr_interface.s_addr = htonl( u32_if_index );

    if( setsockopt( _u32_socket_fd, IPPROTO_IP, IP_MULTICAST_IF, reinterpret_cast<const char *>( &st_ipv4_mreq ), sizeof st_ipv4_mreq ) )
    {
      return ( SocketError::OPTION_SET_FAILED );
    }
  }
  else
  {
    int i32_temp_if_index = u32_if_index;

    if( setsockopt( _u32_socket_fd, IPPROTO_IPV6, IPV6_MULTICAST_IF, reinterpret_cast<const char *>( &i32_temp_if_index ),
      sizeof i32_temp_if_index ) )
    {
      return ( SocketError::OPTION_SET_FAILED );
    }
  }

  return ( SocketError::ERR );
}

SocketError UdpSocketImplWindows::EnableMulticastLoopback( const bool is_enabled )
{
  if( _u32_socket_fd == INVALID_SOCKET )
  {
    return ( SocketError::ERR );
  }

  const auto level = _e_addr_family == IpAddrFamily::IPv4 ? IPPROTO_IP : IPPROTO_IPV6;
  int        on    = is_enabled ? 1 : 0;

  if( setsockopt( _u32_socket_fd, level, IP_MULTICAST_LOOP, reinterpret_cast<const char *>( &on ), sizeof on ) )
  {
    return ( SocketError::OPTION_SET_FAILED );
  }

  return ( SocketError::OK );
}

SocketError UdpSocketImplWindows::ReceiveData( IpAddress &rcz_remote_addr, uint16_t &u16_port, uint8_t *pu8_data, int16_t &ru16_data_length )
{
  SocketError ret_status = SocketError::OK;

  if( ( pu8_data == nullptr ) || ( _u32_socket_fd == INVALID_SOCKET ) )
  {
    return ( SocketError::ERR );
  }

  struct sockaddr_storage st_sock_storage = { 0 };

  int                     i32_sockaddr_len   = sizeof( struct sockaddr_storage );
  auto                    i32_received_bytes = recvfrom( _u32_socket_fd,
      reinterpret_cast<char *>( &pu8_data[0] ),
      ru16_data_length,
      0,
      reinterpret_cast<struct sockaddr *>( &st_sock_storage ),
      &i32_sockaddr_len );

  ru16_data_length = i32_received_bytes;

  return ( ret_status );
}


SocketError UdpSocketImplWindows::SendData( IpAddress &rcz_ip_address, const uint16_t u16_port, uint8_t *pu8_data, const uint16_t u16_data_length )
{
  SocketError ret_status = SocketError::OK;

  if( ( pu8_data == nullptr ) || ( u16_data_length == 0 )
    || ( _u32_socket_fd == INVALID_SOCKET ) )
  {
    return ( SocketError::ERR );
  }

  sockaddr *pst_sock_addr{};
  struct sockaddr_in  st_ipv4_sock_addr = { 0 };

  struct sockaddr_in6 st_ipv6_sock_addr = { 0 };

  int                 i32_sockaddr_len = select_sock_addr( _e_addr_family, pst_sock_addr, &st_ipv4_sock_addr, &st_ipv6_sock_addr, u16_port, rcz_ip_address );

  auto                i32_sent_len = sendto( _u32_socket_fd, reinterpret_cast<char *>( pu8_data ), u16_data_length, 0, pst_sock_addr, i32_sockaddr_len );

  return ( ( SOCKET_ERROR == i32_sent_len ) ? SocketError::SEND_FAILED : SocketError::OK );
}

SocketError UdpSocketImplWindows::EnableReuseAddr( const bool is_enabled )
{
  if( _u32_socket_fd == INVALID_SOCKET )
  {
    return ( SocketError::SOCKET_NOT_VALID );
  }

  auto on = is_enabled ? 1 : 0;

  if( setsockopt( _u32_socket_fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>( &on ), sizeof on ) )
  {
    return ( SocketError::OPTION_SET_FAILED );
  }

  return ( SocketError::OK );
}

SocketError UdpSocketImplWindows::EnableIpv6Only( const bool is_enabled )
{
  if( ( _u32_socket_fd == INVALID_SOCKET ) || ( _e_addr_family != IpAddrFamily::IPv6 ) )
  {
    return ( SocketError::SOCKET_NOT_VALID );
  }

  auto on = is_enabled ? 1 : 0;

  if( setsockopt( _u32_socket_fd, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<const char *>( &on ), sizeof on ) )
  {
    return ( SocketError::OPTION_SET_FAILED );
  }

  return ( SocketError::OK );
}

SocketError UdpSocketImplWindows::EnablePacketInfo( const bool is_enabled )
{
  if( _u32_socket_fd == INVALID_SOCKET )
  {
    return ( SocketError::SOCKET_NOT_VALID );
  }

  int on           = is_enabled ? 1 : 0;
  int i32_level    = ( _e_addr_family == IpAddrFamily::IPv6 ) ? IPPROTO_IPV6 : IPPROTO_IP;
  int i32_opt_name = ( _e_addr_family == IpAddrFamily::IPv6 ) ? IPV6_PKTINFO : IP_PKTINFO;

  if( setsockopt( _u32_socket_fd, i32_level, i32_opt_name, reinterpret_cast<const char *>( &on ), sizeof on ) )
  {
    return ( SocketError::OPTION_SET_FAILED );
  }

  return ( SocketError::OK );
}

uint16_t UdpSocketImplWindows::GetLocalPort()
{
  if( _u32_socket_fd == INVALID_SOCKET )
  {
    return ( 0 );
  }

  struct sockaddr_storage st_sockaddr_storage {};

  socklen_t               i32_sockaddr_len = _e_addr_family == IpAddrFamily::IPv6 ? sizeof( struct sockaddr_in6 ) : sizeof( struct sockaddr_in );

  if( getsockname( _u32_socket_fd, reinterpret_cast<struct sockaddr *>( &st_sockaddr_storage ), &i32_sockaddr_len ) )
  {
    return ( 0 );
  }

  uint16_t u16_binded_port = 0;

  if( st_sockaddr_storage.ss_family == AF_INET6 )
  {
    u16_binded_port = reinterpret_cast<struct sockaddr_in6 *>( &st_sockaddr_storage )->sin6_port;
  }
  else
  {
    u16_binded_port = reinterpret_cast<struct sockaddr_in *>( &st_sockaddr_storage )->sin_port;
  }

  return ( ntohs( u16_binded_port ) );
}

SocketError UdpSocketImplWindows::SetBlocking( bool is_blocked )
{
  return ( SocketError::OK );
}

IpAddrFamily UdpSocketImplWindows::GetAddrFamily()
{
  return ( _e_addr_family );
}
}
}


#endif// #ifdef _OS_WINDOWS_