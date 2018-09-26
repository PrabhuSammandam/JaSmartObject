
/*
 * UdpSocketImplEsp8266.cpp
 *
 *  Created on: Sep 11, 2017
 *      Author: psammand
 */
#ifdef _OS_FREERTOS_

#include <functional>
#include <lwip/err.h>
#include <string.h>
#include <climits>
#include <lwip/sockets.h>

#include "i_udp_socket.h"
#include "ip_addr.h"
#include "port/esp8266/inc/udp_socket_esp8266.h"

namespace ja_iot {
namespace network {
using namespace ja_iot::base;
UdpSocketImplEsp8266::UdpSocketImplEsp8266 ()
{
}

SocketError UdpSocketImplEsp8266::OpenSocket( IpAddrFamily ip_addr_family )
{
  ip_addr_family_ = ip_addr_family;
  socket_fd_      = socket( ( ip_addr_family == IpAddrFamily::IPv4 ) ? PF_INET : PF_INET6, SOCK_DGRAM, IPPROTO_UDP );

  if( socket_fd_ < 0 )
  {
    return ( SocketError::SOCKET_NOT_VALID );
  }

  return ( SocketError::OK );
}

SocketError UdpSocketImplEsp8266::BindSocket( IpAddress &ip_address, uint16_t port )
{
  if( ( socket_fd_ < 0 ) || ( ip_addr_family_ != ip_address.get_addr_family() ) )
  {
    return ( SocketError::SOCKET_NOT_VALID );
  }

  const sockaddr *p_sock_addr  = nullptr;
  int             sockaddr_len = 0;

  if( ip_addr_family_ == IpAddrFamily::IPv4 )
  {
    struct sockaddr_in ipv4_sock_addr = { 0 };

    ipv4_sock_addr.sin_family      = AF_INET;
    ipv4_sock_addr.sin_port        = htons( port );
    ipv4_sock_addr.sin_addr.s_addr = htonl( ip_address.as_u32() );                     // already taken care about network byte order

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

  if( bind( socket_fd_, p_sock_addr, sockaddr_len ) < 0 )
  {
    ret_status = SocketError::BIND_FAILED;
  }

  return ( ret_status );
}

SocketError UdpSocketImplEsp8266::CloseSocket()
{
  if( socket_fd_ >= 0 )
  {
    closesocket( socket_fd_ );
    socket_fd_ = -1;
  }

  return ( SocketError::OK );
}

SocketError UdpSocketImplEsp8266::JoinMulticastGroup( IpAddress &group_address, uint32_t if_index )
{
  if( socket_fd_ < 0 )
  {
    return ( SocketError::SOCKET_NOT_VALID );
  }

  if( ip_addr_family_ == IpAddrFamily::IPv4 )
  {
    struct ip_mreq mreq {};

    mreq.imr_multiaddr.s_addr = htonl( group_address.as_u32() );
    mreq.imr_interface.s_addr = htonl( if_index );

    auto ret_status = setsockopt( socket_fd_, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char *) &mreq, sizeof( mreq ) );

    if( ret_status )
    {
      return ( SocketError::OPTION_SET_FAILED );
    }
  }

  return ( SocketError::OK );
}

SocketError UdpSocketImplEsp8266::LeaveMulticastGroup( IpAddress &group_address, uint32_t if_index )
{
  if( socket_fd_ < 0 )
  {
    return ( SocketError::ERR );
  }

  if( ip_addr_family_ == IpAddrFamily::IPv4 )
  {
    struct ip_mreq mreq {};

    mreq.imr_multiaddr.s_addr = htonl( group_address.as_u32() );
    mreq.imr_interface.s_addr = htonl( if_index );

    if( setsockopt( socket_fd_, IPPROTO_IP, IP_DROP_MEMBERSHIP, (const char *) &mreq, sizeof( mreq ) ) )
    {
      return ( SocketError::OPTION_SET_FAILED );
    }
  }

  return ( SocketError::OK );
}

SocketError UdpSocketImplEsp8266::SelectMulticastInterface( IpAddress &group_address, uint32_t if_index )
{
  if( socket_fd_ < 0 )
  {
    return ( SocketError::ERR );
  }

  if( ip_addr_family_ == IpAddrFamily::IPv4 )
  {
    struct ip_mreq mreq {};

    mreq.imr_multiaddr.s_addr = htonl( group_address.as_u32() );
    mreq.imr_interface.s_addr = htonl( if_index );

    if( setsockopt( socket_fd_, IPPROTO_IP, IP_MULTICAST_IF, (const char *) &mreq, sizeof( mreq ) ) )
    {
      return ( SocketError::OPTION_SET_FAILED );
    }
  }

  return ( SocketError::ERR );
}

SocketError UdpSocketImplEsp8266::ReceiveData( IpAddress &remote_addr, uint16_t &port, uint8_t *data, int16_t &data_length )
{
  SocketError ret_status = SocketError::OK;

  if( ( data == nullptr ) || ( socket_fd_ < 0 ) )
  {
    return ( SocketError::ERR );
  }

  // printf("UdpSocketImplEsp8266::ReceiveData: ReadData socked_fd[%d]\n", socket_fd_);

  struct sockaddr_storage sock_store = { 0 };

  unsigned int            sockaddr_len   = sizeof( struct sockaddr_storage );
  auto                    received_bytes = recvfrom( socket_fd_, (void *) &data[0], data_length, 0, (sockaddr *) &sock_store, &sockaddr_len );

  if( received_bytes > 0 )
  {
    // printf("UdpSocketImplEsp8266::ReceiveData: ReadData data_len[%d]\n", received_bytes);
    if( sock_store.ss_family == AF_INET )
    {
      struct sockaddr_in *p_ipv4_addr = (struct sockaddr_in *) &sock_store;
      remote_addr.set_addr_family( IpAddrFamily::IPv4 );
      remote_addr.set_addr( (uint8_t *) &p_ipv4_addr->sin_addr.s_addr, IpAddrFamily::IPv4 );
      port = ntohs( p_ipv4_addr->sin_port );
    }
    else if( sock_store.ss_family == AF_INET6 )
    {
      struct sockaddr_in6 *p_ipv6_addr = (struct sockaddr_in6 *) &sock_store;
      remote_addr.set_addr_family( IpAddrFamily::IPv6 );
      remote_addr.set_addr( (uint8_t *) &p_ipv6_addr->sin6_addr, IpAddrFamily::IPv6 );
      port = ntohs( p_ipv6_addr->sin6_port );
    }
  }

  data_length = received_bytes;

  return ( ret_status );
}

SocketError UdpSocketImplEsp8266::SendData( IpAddress &ip_address, uint16_t port, uint8_t *data, uint16_t data_length )
{
  if( ( data == nullptr ) || ( data_length == 0 )
    || ( socket_fd_ < 0 ) )
  {
    return ( SocketError::ERR );
  }

  const sockaddr *p_sock_addr{ nullptr };
  struct sockaddr_in  sock_addr = { 0 };

  struct sockaddr_in6 sock_addr6 = { 0 };

  int sockaddr_len{ 0 };

  if( ip_address.get_addr_family() == IpAddrFamily::IPv4 )
  {
    sock_addr.sin_family      = AF_INET;
    sock_addr.sin_port        = htons( port );
    sock_addr.sin_addr.s_addr = htonl( ip_address.as_u32() );

    p_sock_addr  = (const sockaddr *) &sock_addr;
    sockaddr_len = sizeof( struct sockaddr_in );
  }
  else
  {
    sock_addr6.sin6_family = AF_INET6;
    sock_addr6.sin6_port   = htons( port );
    memcpy( &sock_addr6.sin6_addr, ip_address.get_addr(),
      sizeof( sock_addr6.sin6_addr ) );

    p_sock_addr  = (const sockaddr *) &sock_addr6;
    sockaddr_len = sizeof( struct sockaddr_in6 );
  }

  auto len = sendto( socket_fd_, reinterpret_cast<char *>( data ), data_length, 0, p_sock_addr, sockaddr_len );

  return ( ( -1 == len ) ? SocketError::SEND_FAILED : SocketError::OK );
}

SocketError UdpSocketImplEsp8266::EnableMulticastLoopback( bool is_enabled )
{
  if( socket_fd_ < 0 )
  {
    return ( SocketError::ERR );
  }

  if( ip_addr_family_ == IpAddrFamily::IPv4 )
  {
    int on = ( is_enabled ) ? 1 : 0;

    if( setsockopt( socket_fd_, IPPROTO_IP, IP_MULTICAST_LOOP, (const char *) &on, sizeof( on ) ) )
    {
      return ( SocketError::OPTION_SET_FAILED );
    }
  }

  return ( SocketError::OK );
}

SocketError UdpSocketImplEsp8266::EnableReuseAddr( bool is_enabled )
{
  if( socket_fd_ < 0 )
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

SocketError UdpSocketImplEsp8266::EnableIpv6Only( bool is_enabled )
{
  if( ( socket_fd_ < 0 ) || ( ip_addr_family_ != IpAddrFamily::IPv6 ) )
  {
    return ( SocketError::ERR );
  }

  int on = ( is_enabled ) ? 1 : 0;

  if( setsockopt( socket_fd_, IPPROTO_IPV6, IPV6_V6ONLY, (const char *) &on, sizeof( on ) ) )
  {
    return ( SocketError::OPTION_SET_FAILED );
  }

  return ( SocketError::OK );
}

SocketError UdpSocketImplEsp8266::EnablePacketInfo( bool is_enabled )
{
  return ( SocketError::OK );
}

SocketError UdpSocketImplEsp8266::SetBlocking( bool is_blocked )
{
  int flags = fcntl( socket_fd_, F_GETFL, 0 );

  fcntl( socket_fd_, F_SETFL, ( is_blocked ) ? ( flags &= ( ~O_NONBLOCK ) ) : ( flags |= O_NONBLOCK ) );

  return ( SocketError::OK );
}

uint16_t UdpSocketImplEsp8266::GetLocalPort()
{
  if( socket_fd_ < 0 )
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

IpAddrFamily UdpSocketImplEsp8266::GetAddrFamily()
{
  return ( ip_addr_family_ );
}
}
}
#endif /* _OS_FREERTOS_ */
