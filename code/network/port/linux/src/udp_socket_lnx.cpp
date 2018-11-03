/*
 * UdpSocketImplLinux.cpp
 *
 *  Created on: 17-Sep-2017
 *      Author: prabhu
 */
#ifdef _OS_LINUX_

#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <bits/stdc++.h>
#include <fcntl.h>
#include "OsalError.h"
#include "port/linux/inc/udp_socket_lnx.h"
#include "ip_addr.h"

using namespace ja_iot::base;

namespace ja_iot {
namespace network {
UdpSocketImplLinux::UdpSocketImplLinux ()
{
}

SocketError UdpSocketImplLinux::OpenSocket( IpAddrFamily ip_addr_family )
{
  ip_addr_family_ = ip_addr_family;
  socket_fd_      = socket( ( ip_addr_family == IpAddrFamily::IPv4 ) ? PF_INET : PF_INET6, SOCK_DGRAM, IPPROTO_UDP );

  if( socket_fd_ < 0 )
  {
    return ( SocketError::SOCKET_NOT_VALID );
  }

  EnableMulticastLoopback(false);

  return ( SocketError::OK );
}

SocketError UdpSocketImplLinux::BindSocket( IpAddress &ip_address, uint16_t port )
{
  if( ( socket_fd_ < 0 ) || ( ip_addr_family_ != ip_address.get_addr_family() ) )
  {
    return ( SocketError::SOCKET_NOT_VALID );
  }

  const sockaddr *    p_sock_addr    = nullptr;
  struct sockaddr_in  ipv4_sock_addr = { 0 };

  struct sockaddr_in6 ipv6_sock_addr = { 0 };

  int                 sockaddr_len = 0;

  if( ip_addr_family_ == IpAddrFamily::IPv4 )
  {
    ipv4_sock_addr.sin_family      = AF_INET;
    ipv4_sock_addr.sin_port        = htons( port );
    ipv4_sock_addr.sin_addr.s_addr = htonl( ip_address.as_u32() );                     // already taken care about network byte order

    p_sock_addr  = (const sockaddr *) &ipv4_sock_addr;
    sockaddr_len = sizeof( struct sockaddr_in );
  }
  else
  {
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

SocketError UdpSocketImplLinux::CloseSocket()
{
  if( socket_fd_ >= 0 )
  {
    close( socket_fd_ );
    socket_fd_ = -1;
  }

  return ( SocketError::OK );
}

SocketError UdpSocketImplLinux::JoinMulticastGroup( IpAddress &group_address, uint32_t if_index )
{
  if( socket_fd_ < 0 )
  {
    return ( SocketError::SOCKET_NOT_VALID );
  }

  if( ip_addr_family_ == IpAddrFamily::IPv4 )
  {
    struct ip_mreqn mreq {};

    mreq.imr_multiaddr.s_addr = htonl( group_address.as_u32() );
    mreq.imr_address.s_addr = INADDR_ANY;
    mreq.imr_ifindex = if_index;

    auto ret_status = setsockopt( socket_fd_, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char *) &mreq, sizeof( mreq ) );

    if( ret_status )
    {
      return ( SocketError::OPTION_SET_FAILED );
    }
  }
  else
  {
    struct ipv6_mreq mreq {};

    memcpy( &mreq.ipv6mr_multiaddr, group_address.get_addr(), sizeof mreq.ipv6mr_multiaddr );
    mreq.ipv6mr_interface = if_index;

    if( setsockopt( socket_fd_, IPPROTO_IPV6, IPV6_JOIN_GROUP, reinterpret_cast<const char *>( &mreq ), sizeof mreq ) )
    {
      return ( SocketError::OPTION_SET_FAILED );
    }
  }

  return ( SocketError::OK );
}

SocketError UdpSocketImplLinux::LeaveMulticastGroup( IpAddress &group_address, uint32_t if_index )
{
  if( socket_fd_ < 0 )
  {
    return ( SocketError::ERR );
  }

  if( ip_addr_family_ == IpAddrFamily::IPv4 )
  {
	struct ip_mreqn mreq {};

	mreq.imr_multiaddr.s_addr = htonl( group_address.as_u32() );
	mreq.imr_address.s_addr = INADDR_ANY;
	mreq.imr_ifindex = if_index;

    if( setsockopt( socket_fd_, IPPROTO_IP, IP_DROP_MEMBERSHIP, (const char *) &mreq, sizeof( mreq ) ) )
    {
      return ( SocketError::OPTION_SET_FAILED );
    }
  }
  else
  {
    struct ipv6_mreq mreq {};

    mreq.ipv6mr_interface = if_index;
    memcpy( &mreq.ipv6mr_multiaddr, group_address.get_addr(), sizeof mreq.ipv6mr_multiaddr );

    if( setsockopt( socket_fd_, IPPROTO_IPV6, IPV6_LEAVE_GROUP, reinterpret_cast<const char *>( &mreq ), sizeof mreq ) )
    {
      return ( SocketError::OPTION_SET_FAILED );
    }
  }

  return ( SocketError::OK );
}

SocketError UdpSocketImplLinux::SelectMulticastInterface( IpAddress &rcz_group_address, uint32_t if_index )
{
  if( socket_fd_ < 0 )
  {
    return ( SocketError::ERR );
  }

  if( ip_addr_family_ == IpAddrFamily::IPv4 )
  {
	struct ip_mreqn mreq {};

	mreq.imr_multiaddr.s_addr = htonl( rcz_group_address.as_u32() );
	mreq.imr_address.s_addr = INADDR_ANY;
	mreq.imr_ifindex = if_index;

    if( setsockopt( socket_fd_, IPPROTO_IP, IP_MULTICAST_IF, (const char *) &mreq, sizeof( mreq ) ) )
    {
      return ( SocketError::OPTION_SET_FAILED );
    }
  }
  else
  {
    int index = if_index;

    if( setsockopt( socket_fd_, IPPROTO_IPV6, IPV6_MULTICAST_IF, reinterpret_cast<const char *>( &index ),
      sizeof index ) )
    {
      return ( SocketError::OPTION_SET_FAILED );
    }
  }

//  EnableMulticastLoopback(false);

  return ( SocketError::ERR );
}

SocketError UdpSocketImplLinux::ReceiveData( IpAddress &remote_addr, uint16_t &port, uint8_t *data, int16_t &data_length )
{
  SocketError ret_status = SocketError::OK;

  if( ( data == nullptr ) || ( socket_fd_ < 0 ) )
  {
    return ( SocketError::ERR );
  }

  // printf("UdpSocketImplLinux::ReceiveData: ReadData socked_fd[%d]\n", socket_fd_);

  struct sockaddr_storage sock_store = { 0 };

  unsigned int            sockaddr_len   = sizeof( struct sockaddr_storage );
  auto                    received_bytes = recvfrom( socket_fd_, (void *) &data[0], data_length, 0, (sockaddr *) &sock_store, &sockaddr_len );

  if( received_bytes > 0 )
  {
    // printf("UdpSocketImplLinux::ReceiveData: ReadData data_len[%d]\n", received_bytes);
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

SocketError UdpSocketImplLinux::SendData( IpAddress &ip_address, uint16_t port, uint8_t *data, uint16_t data_length )
{
  SocketError ret_status = SocketError::OK;

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
    memcpy( &sock_addr6.sin6_addr, ip_address.get_addr(), sizeof( sock_addr6.sin6_addr ) );

    p_sock_addr  = (const sockaddr *) &sock_addr6;
    sockaddr_len = sizeof( struct sockaddr_in6 );
  }

  int len{ 0 };
  size_t currently_no_of_bytes_sent = 0;

  do
  {
    int remaining_bytes_to_send = ( ( data_length - currently_no_of_bytes_sent ) > INT_MAX ) ? INT_MAX : (int) ( data_length - currently_no_of_bytes_sent );

    len = sendto( socket_fd_, ( (char *) data ) + currently_no_of_bytes_sent,
        remaining_bytes_to_send, 0, p_sock_addr, sockaddr_len );

    if( -1 == len )
    {
      ret_status = SocketError::SEND_FAILED;
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
  } while( ( -1 == len ) || ( currently_no_of_bytes_sent < data_length ) );

  return ( ret_status );
}

SocketError UdpSocketImplLinux::EnableMulticastLoopback( bool is_enabled )
{
  if( socket_fd_ < 0 )
  {
    return ( SocketError::ERR );
  }

  int on = ( is_enabled ) ? 1 : 0;
  int level = ( ip_addr_family_ == IpAddrFamily::IPv4 ) ?  IPPROTO_IP : IPPROTO_IPV6;
  int option = ( ip_addr_family_ == IpAddrFamily::IPv4 ) ?  IP_MULTICAST_LOOP : IPV6_MULTICAST_LOOP;

	if( setsockopt( socket_fd_, level, option, (const char *) &on, sizeof( on ) ) )
	{
	  return ( SocketError::OPTION_SET_FAILED );
	}

  return ( SocketError::OK );
}

SocketError UdpSocketImplLinux::EnableReuseAddr( bool is_enabled )
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

SocketError UdpSocketImplLinux::EnableIpv6Only( bool is_enabled )
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

SocketError UdpSocketImplLinux::EnablePacketInfo( bool is_enabled )
{
  if( socket_fd_ < 0 )
  {
    return ( SocketError::SOCKET_NOT_VALID );
  }

  int on = is_enabled ? 1 : 0;

  if( ip_addr_family_ == IpAddrFamily::IPv6 )
  {
    if( setsockopt( socket_fd_, IPPROTO_IPV6, IPV6_RECVPKTINFO, reinterpret_cast<const char *>( &on ), sizeof on ) )
    {
    	auto err = errno;
    	printf("UdpSocketImplLinux::EnablePacketInfo: ERRNO %d\n", errno);
      return ( SocketError::OPTION_SET_FAILED );
    }
  }
  else
  {
    if( setsockopt( socket_fd_, IPPROTO_IP, IP_PKTINFO, reinterpret_cast<const char *>( &on ), sizeof on ) )
    {
    	auto err = errno;
    	printf("UdpSocketImplLinux::EnablePacketInfo: ERRNO %d\n", errno);
      return ( SocketError::OPTION_SET_FAILED );
    }
  }

  return ( SocketError::OK );
}

SocketError UdpSocketImplLinux::SetBlocking( bool is_blocked )
{
  int flags = fcntl( socket_fd_, F_GETFL, 0 );

  fcntl( socket_fd_, F_SETFL, ( is_blocked ) ? ( flags &= ( ~O_NONBLOCK ) ) : ( flags |= O_NONBLOCK ) );

  return ( SocketError::OK );
}

uint16_t UdpSocketImplLinux::GetLocalPort()
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

IpAddrFamily UdpSocketImplLinux::GetAddrFamily()
{
  return ( ip_addr_family_ );
}
}
}

#endif /* _OS_LINUX_ */
