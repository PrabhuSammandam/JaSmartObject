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
#include <port/windows/inc/udp_socket_win.h>
#include <climits>

namespace ja_iot {
namespace network {

using namespace ja_iot::base;

UdpSocketImplWindows::UdpSocketImplWindows ()
{
  WSADATA wsa_data{};

  WSAStartup( MAKEWORD( 2, 2 ), &wsa_data );
}

SocketError UdpSocketImplWindows::OpenSocket( IpAddrFamily ip_addr_family )
{
  ip_addr_family_ = ip_addr_family;
  socket_fd_      = socket( ( ip_addr_family == IpAddrFamily::IPV4 ) ? PF_INET : PF_INET6, SOCK_DGRAM, IPPROTO_UDP );

  if( socket_fd_ == INVALID_SOCKET )
  {
    return ( SocketError::SOCKET_NOT_VALID );
  }

  return ( SocketError::OK );
}

SocketError UdpSocketImplWindows::BindSocket( IpAddress &ip_address, uint16_t port )
{
  if( ( socket_fd_ == INVALID_SOCKET ) || ( ip_addr_family_ != ip_address.get_addr_family() ) )
  {
    return ( SocketError::SOCKET_NOT_VALID );
  }

  const sockaddr *p_sock_addr  = nullptr;
  int             sockaddr_len = 0;

  if( ip_addr_family_ == IpAddrFamily::IPV4 )
  {
    struct sockaddr_in ipv4_sock_addr = { 0 };

    ipv4_sock_addr.sin_family           = AF_INET;
    ipv4_sock_addr.sin_port             = htons( port );
    ipv4_sock_addr.sin_addr.S_un.S_addr = htonl( ip_address.as_u32() );             // already taken care about network byte order

    p_sock_addr  = (const sockaddr *) &ipv4_sock_addr;
    sockaddr_len = sizeof( struct sockaddr_in );
  }
  else
  {
    struct sockaddr_in6 ipv6_sock_addr = { 0 };

    ipv6_sock_addr.sin6_family = AF_INET6;
    ipv6_sock_addr.sin6_port   = htons( port );
    memcpy( &ipv6_sock_addr.sin6_addr, ip_address.get_addr(), sizeof( ipv6_sock_addr.sin6_addr ) );

    p_sock_addr  = (const sockaddr *) &ipv6_sock_addr;
    sockaddr_len = sizeof( struct sockaddr_in6 );
  }

  SocketError ret_status = SocketError::OK;

  if( bind( socket_fd_, p_sock_addr, sockaddr_len ) == SOCKET_ERROR )
  {
    ret_status = SocketError::BIND_FAILED;
  }

  return ( ret_status );
}

SocketError UdpSocketImplWindows::CloseSocket()
{
  if( socket_fd_ != INVALID_SOCKET )
  {
    closesocket( socket_fd_ );
    socket_fd_ = INVALID_SOCKET;
  }

  return ( SocketError::OK );
}

SocketError UdpSocketImplWindows::JoinMulticastGroup( IpAddress &group_address, uint32_t if_index )
{
  if( socket_fd_ == INVALID_SOCKET )
  {
    return ( SocketError::SOCKET_NOT_VALID );
  }

  if( ip_addr_family_ == IpAddrFamily::IPV4 )
  {
    struct ip_mreq mreq {};

    mreq.imr_multiaddr.s_addr = htonl( group_address.as_u32() );
    mreq.imr_interface.s_addr = htonl( if_index );

    auto ret_status = setsockopt( socket_fd_, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char *) &mreq, sizeof( mreq ) );

    if( ret_status )
    {
      // auto error = WSAGetLastError();
      return ( SocketError::OPTION_SET_FAILED );
    }
  }
  else
  {
    struct ipv6_mreq mreq {};

    memcpy( &mreq.ipv6mr_multiaddr, group_address.get_addr(), sizeof( mreq.ipv6mr_multiaddr ) );
    mreq.ipv6mr_interface = if_index;

    if( setsockopt( socket_fd_, IPPROTO_IPV6, IPV6_JOIN_GROUP, (const char *) &mreq, sizeof( mreq ) ) )
    {
      return ( SocketError::OPTION_SET_FAILED );
    }
  }

  return ( SocketError::OK );
}

SocketError UdpSocketImplWindows::LeaveMulticastGroup( IpAddress &group_address, uint32_t if_index )
{
  if( socket_fd_ == INVALID_SOCKET )
  {
    return ( SocketError::ERR );
  }

  if( ip_addr_family_ == IpAddrFamily::IPV4 )
  {
    struct ip_mreq mreq {};

    mreq.imr_multiaddr.s_addr = htonl( group_address.as_u32() );
    mreq.imr_interface.s_addr = htonl( if_index );

    if( setsockopt( socket_fd_, IPPROTO_IP, IP_DROP_MEMBERSHIP, (const char *) &mreq, sizeof( mreq ) ) )
    {
      return ( SocketError::OPTION_SET_FAILED );
    }
  }
  else
  {
    struct ipv6_mreq mreq {};

    mreq.ipv6mr_interface = if_index;
    memcpy( &mreq.ipv6mr_multiaddr, group_address.get_addr(), sizeof( mreq.ipv6mr_multiaddr ) );

    if( setsockopt( socket_fd_, IPPROTO_IPV6, IPV6_LEAVE_GROUP, (const char *) &mreq, sizeof( mreq ) ) )
    {
      return ( SocketError::OPTION_SET_FAILED );
    }
  }

  return ( SocketError::OK );
}

SocketError UdpSocketImplWindows::SelectMulticastInterface( IpAddress &group_address, uint32_t if_index )
{
  if( socket_fd_ == INVALID_SOCKET )
  {
    return ( SocketError::ERR );
  }

  if( ip_addr_family_ == IpAddrFamily::IPV4 )
  {
    struct ip_mreq mreq {};

    mreq.imr_multiaddr.s_addr = htonl( group_address.as_u32() );
    mreq.imr_interface.s_addr = htonl( if_index );

    if( setsockopt( socket_fd_, IPPROTO_IP, IP_MULTICAST_IF, (const char *) &mreq, sizeof( mreq ) ) )
    {
      return ( SocketError::OPTION_SET_FAILED );
    }
  }
  else
  {
    int index = if_index;

    if( setsockopt( socket_fd_, IPPROTO_IPV6, IPV6_MULTICAST_IF, (const char *) &index, sizeof( index ) ) )
    {
      return ( SocketError::OPTION_SET_FAILED );
    }
  }

  return ( SocketError::ERR );
}

SocketError UdpSocketImplWindows::EnableMulticastLoopback( bool is_enabled )
{
  if( socket_fd_ == INVALID_SOCKET )
  {
    return ( SocketError::ERR );
  }

  auto level = ( ip_addr_family_ == IpAddrFamily::IPV4 ) ? IPPROTO_IP : IPPROTO_IPV6;
  int  on    = ( is_enabled ) ? 1 : 0;

  if( setsockopt( socket_fd_, level, IP_MULTICAST_LOOP, (const char *) &on, sizeof( on ) ) )
  {
    return ( SocketError::OPTION_SET_FAILED );
  }

  return ( SocketError::OK );
}

SocketError UdpSocketImplWindows::ReceiveData( IpAddress &remote_addr, uint16_t &port, uint8_t *data, int16_t &data_length )
{
  SocketError ret_status = SocketError::OK;

  if( ( data == nullptr ) || ( socket_fd_ == INVALID_SOCKET ) )
  {
    return ( SocketError::ERR );
  }

  struct sockaddr_storage sock_store = { 0 };

  int                     sockaddr_len = sizeof( struct sockaddr_storage );

  auto                    received_bytes = recvfrom( socket_fd_, (char *) &data[0], data_length, 0, (struct sockaddr *) &sock_store, &sockaddr_len );

  data_length = received_bytes;

  return ( ret_status );
}


SocketError UdpSocketImplWindows::SendData( IpAddress &ip_address, uint16_t port, uint8_t *data, uint16_t data_length )
{
  SocketError ret_status = SocketError::OK;

  if( ( data == nullptr ) || ( data_length == 0 )
    || ( socket_fd_ == INVALID_SOCKET ) )
  {
    return ( SocketError::ERR );
  }

  const sockaddr *p_sock_addr{ nullptr };
  int sockaddr_len{ 0 };

  if( ip_address.get_addr_family() == IpAddrFamily::IPV4 )
  {
    struct sockaddr_in sock_addr = { 0 };

    sock_addr.sin_family           = AF_INET;
    sock_addr.sin_port             = htons( port );
    sock_addr.sin_addr.S_un.S_addr = htonl( ip_address.as_u32() );     // already taken care about network byte order

    p_sock_addr  = (const sockaddr *) &sock_addr;
    sockaddr_len = sizeof( struct sockaddr_in );
  }
  else
  {
    struct sockaddr_in6 sock_addr6 = { 0 };

    sock_addr6.sin6_family = AF_INET6;
    sock_addr6.sin6_port   = htons( port );
    memcpy( &sock_addr6.sin6_addr, ip_address.get_addr(),
      sizeof( sock_addr6.sin6_addr ) );

    p_sock_addr  = (const sockaddr *) &sock_addr6;
    sockaddr_len = sizeof( struct sockaddr_in6 );
  }

  int err{ 0 };
  int len{ 0 };
  size_t currently_no_of_bytes_sent = 0;

  do
  {
    int remaining_bytes_to_send = ( ( data_length - currently_no_of_bytes_sent ) > INT_MAX ) ? INT_MAX : (int) ( data_length - currently_no_of_bytes_sent );

    len = sendto( socket_fd_, ( (char *) data ) + currently_no_of_bytes_sent,
        remaining_bytes_to_send, 0, p_sock_addr, sockaddr_len );

    if( SOCKET_ERROR == len )
    {
      err = WSAGetLastError();

      if( ( WSAEWOULDBLOCK != err ) && ( WSAENOBUFS != err ) )
      {
        ret_status = SocketError::SEND_FAILED;
      }
    }
    else
    {
      currently_no_of_bytes_sent += len;

      if( currently_no_of_bytes_sent != (size_t) len )
      {
      }
      else
      {
      }
    }
  } while( ( ( SOCKET_ERROR == len ) && ( WSAEWOULDBLOCK == err ) )
         || ( WSAENOBUFS == err ) || ( currently_no_of_bytes_sent < data_length ) );

  return ( ret_status );
}

SocketError UdpSocketImplWindows::EnableReuseAddr( bool is_enabled )
{
  if( socket_fd_ == INVALID_SOCKET )
  {
    return ( SocketError::SOCKET_NOT_VALID );
  }

  int on = ( is_enabled ) ? 1 : 0;

  if( setsockopt( socket_fd_, SOL_SOCKET, SO_REUSEADDR, (const char *) &on, sizeof( on ) ) )
  {
    return ( SocketError::OPTION_SET_FAILED );
  }

  return ( SocketError::OK );
}

SocketError UdpSocketImplWindows::EnableIpv6Only( bool is_enabled )
{
  if( ( socket_fd_ == INVALID_SOCKET ) || ( ip_addr_family_ != IpAddrFamily::IPv6 ) )
  {
    return ( SocketError::SOCKET_NOT_VALID );
  }

  int on = ( is_enabled ) ? 1 : 0;

  if( setsockopt( socket_fd_, IPPROTO_IPV6, IPV6_V6ONLY, (const char *) &on, sizeof( on ) ) )
  {
    return ( SocketError::OPTION_SET_FAILED );
  }

  return ( SocketError::OK );
}

SocketError UdpSocketImplWindows::EnablePacketInfo( bool is_enabled )
{
  if( socket_fd_ == INVALID_SOCKET )
  {
    return ( SocketError::SOCKET_NOT_VALID );
  }

  int on = ( is_enabled ) ? 1 : 0;

  if( ip_addr_family_ == IpAddrFamily::IPv6 )
  {
    if( setsockopt( socket_fd_, IPPROTO_IPV6, IPV6_PKTINFO, (const char *) &on, sizeof( on ) ) )
    {
      return ( SocketError::OPTION_SET_FAILED );
    }
  }
  else
  {
    if( setsockopt( socket_fd_, IPPROTO_IP, IP_PKTINFO, (const char *) &on, sizeof( on ) ) )
    {
      return ( SocketError::OPTION_SET_FAILED );
    }
  }

  return ( SocketError::OK );
}

uint16_t UdpSocketImplWindows::GetLocalPort()
{
  if( socket_fd_ == INVALID_SOCKET )
  {
    return ( 0 );
  }

  struct sockaddr_storage sa {};

  socklen_t               socklen = ( ip_addr_family_ == IpAddrFamily::IPv6 ) ? sizeof( struct sockaddr_in6 ) : sizeof( struct sockaddr_in );

  if( getsockname( socket_fd_, (struct sockaddr *) &sa, &socklen ) )
  {
    return ( 0 );
  }

  uint16_t binded_port = 0;

  if( sa.ss_family == AF_INET6 )
  {
    binded_port = ( ( (struct sockaddr_in6 *) &sa )->sin6_port );
  }
  else
  {
    binded_port = ( ( (struct sockaddr_in *) &sa )->sin_port );
  }

  return ( ntohs( binded_port ) );
}

SocketError UdpSocketImplWindows::SetBlocking( bool is_blocked )
{
  return ( SocketError::OK );
}

IpAddrFamily UdpSocketImplWindows::GetAddrFamily()
{
  return ( ip_addr_family_ );
}
}
}


#endif// #ifdef _OS_WINDOWS_
