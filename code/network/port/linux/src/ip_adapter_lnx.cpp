/*
 * IpAdapterImplLinux.cpp
 *
 *  Created on: 17-Sep-2017
 *      Author: prabhu
 */

#ifdef _OS_LINUX_

#include <unistd.h>
#include <fcntl.h>
#include <asm/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <algorithm>
#include <string.h>
#include "port/linux/inc/ip_adapter_lnx.h"
#include "base_consts.h"
#include "common/inc/logging_network.h"
#include "base_utils.h"
#include "config_mgr.h"
#include "ip_addr.h"
#include "i_nwk_platform_factory.h"
#include "OsalMgr.h"
#include "ScopedMutex.h"
#include "port/linux/inc/udp_socket_lnx.h"

#define __FILE_NAME__ "IpAdapterImplLinux"

namespace ja_iot {
namespace network {
using namespace ja_iot::base;
using namespace ja_iot::network;
using namespace ja_iot::osal;

void                              intl_add_socket_to_fd_array( UdpSocketImplLinux *pcz_udp_socket, fd_set *readFds, int *max_fd );
IpAdapterImplLinux::IpAdapterImplLinux ()
{
}
IpAdapterImplLinux::~IpAdapterImplLinux ()
{
}

std::vector<InterfaceAddress *> IpAdapterImplLinux::get_interface_address_for_index( uint8_t u8_index )
{
  char ascii_addr_buf[64] = { 0 };

  std::vector<InterfaceAddress *> pcz_interface_ptr_vector{};

  DBG_INFO2( "ENTER index %d", u8_index );

  struct ifaddrs *pst_interface_addr_list = nullptr;

  if( -1 != getifaddrs( &pst_interface_addr_list ) )
  {
    struct ifaddrs *ifa = nullptr;

    DBG_INFO2( "Iterating interfaces START" );

    for( ifa = pst_interface_addr_list; ifa; ifa = ifa->ifa_next )
    {
      if( !ifa->ifa_addr )
      {
        continue;
      }

      int ifindex = if_nametoindex( ifa->ifa_name );

      if( ( ifa->ifa_flags & IFF_LOOPBACK )
        || ( ( AF_INET != ifa->ifa_addr->sa_family ) && ( AF_INET6 != ifa->ifa_addr->sa_family ) )
        || ( ( u8_index > 0 ) && ( ifindex != u8_index ) ) )
      {
        continue;
      }

      IpAddrFamily      e_addr_family = ( AF_INET6 == ifa->ifa_addr->sa_family ) ? IpAddrFamily::IPv6 : IpAddrFamily::IPv4;

      InterfaceAddress *if_addr = nullptr;

      if( e_addr_family == IpAddrFamily::IPv6 )
      {
        struct sockaddr_in6 *in6 = (struct sockaddr_in6 *) ifa->ifa_addr;
        inet_ntop( AF_INET6, (void *) ( &in6->sin6_addr ), &ascii_addr_buf[0], sizeof( ascii_addr_buf ) );
        if_addr = new InterfaceAddress{ (uint32_t) ifindex, ifa->ifa_flags, e_addr_family, (const char *) &in6->sin6_addr };
      }
      else if( e_addr_family == IpAddrFamily::IPv4 )
      {
        struct sockaddr_in *in = (struct sockaddr_in *) ifa->ifa_addr;
        inet_ntop( AF_INET, (void *) ( &in->sin_addr ), &ascii_addr_buf[0], sizeof( ascii_addr_buf ) );
        if_addr = new InterfaceAddress{ (uint32_t) ifindex, ifa->ifa_flags, e_addr_family, (const char *) &in->sin_addr };
      }

      DBG_INFO2( "Found interface index[%d], flag[%d], family[%d], addr[%s]", ifindex, ifa->ifa_flags, (int) e_addr_family, &ascii_addr_buf[0] );
      pcz_interface_ptr_vector.push_back( if_addr );
    }// for

    DBG_INFO2( "Iterating interfaces END" );

    freeifaddrs( pst_interface_addr_list );
  }

  DBG_INFO2( "EXIT" );
  return ( pcz_interface_ptr_vector );
}

void IpAdapterImplLinux::do_init_fast_shutdown_mechanism()
{
  int ret = -1;

  ret = pipe( shutdown_fds );

  if( -1 != ret )
  {
    ret = fcntl( shutdown_fds[0], F_GETFD );

    if( -1 != ret )
    {
      ret = fcntl( shutdown_fds[0], F_SETFD, ret | FD_CLOEXEC );
    }

    if( -1 != ret )
    {
      ret = fcntl( shutdown_fds[1], F_GETFD );
    }

    if( -1 != ret )
    {
      ret = fcntl( shutdown_fds[1], F_SETFD, ret | FD_CLOEXEC );
    }

    if( -1 == ret )
    {
      close( shutdown_fds[1] );
      close( shutdown_fds[0] );
      shutdown_fds[0] = -1;
      shutdown_fds[1] = -1;
    }
  }

  update_max_fd(shutdown_fds[0]);
  update_max_fd(shutdown_fds[1]);
}

void IpAdapterImplLinux::do_init_address_change_notify_mechanism()
{
  struct sockaddr_nl sa = { AF_NETLINK, 0, 0,
                            RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR };

  netlink_fd = socket( AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE );

  if( netlink_fd != -1 )
  {
    int r = bind( netlink_fd, (struct sockaddr *) &sa, sizeof( sa ) );

    if( r )
    {
      close( netlink_fd );
      netlink_fd = -1;
    }
    else
    {
    	  update_max_fd(netlink_fd);
    }
  }
}

void IpAdapterImplLinux::do_un_init_address_change_notify_mechanism()
{
  if( netlink_fd != -1 )
  {
    close( netlink_fd );
    netlink_fd = -1;
  }
}

int check_data_available( UdpSocketImplLinux *udp_socket, fd_set *read_fds, uint16_t &u16_network_flag )
{
  if( ( udp_socket != nullptr ) && ( udp_socket->get_socket() != -1 ) )
  {
    if( FD_ISSET( udp_socket->get_socket(), read_fds ) )
    {
      u16_network_flag = udp_socket->get_flags();
      return ( udp_socket->get_socket() );
    }
  }

  return ( -1 );
}

void IpAdapterImplLinux::do_handle_send_msg( IpAdapterQMsg *ip_adapter_q_msg )
{

}

void IpAdapterImplLinux::do_handle_receive()
{
  while( !is_terminated_ )
  {
    fd_set readFds;

    FD_ZERO( &readFds );

    for( auto &socket : _sockets )
    {
    	intl_add_socket_to_fd_array( (UdpSocketImplLinux *) socket, &readFds, &max_fd );
    }

    if( shutdown_fds[0] != -1 )
    {
      FD_SET( shutdown_fds[0], &readFds );
    }

    if( netlink_fd != -1 )
    {
      FD_SET( netlink_fd, &readFds );
    }

    int ret = select( max_fd + 1, &readFds, nullptr, nullptr, nullptr );

    if( ret == 0 )
    {
      continue;
    }
    else if( 0 < ret )
    {
      while( !is_terminated_ )
      {
        int  selected_fd      = -1;
        auto u16_network_flag = k_network_flag_none;

        for( auto &socket : _sockets )
        {
          auto udp_socket = (UdpSocketImplLinux *) socket;

          if( FD_ISSET( udp_socket->get_socket(), &readFds ) )
          {
            selected_fd      = udp_socket->get_socket();
            u16_network_flag = udp_socket->get_flags();
            break;
          }
        }

        if( selected_fd != -1 )
        {
          FD_CLR( selected_fd, &readFds );

          handle_received_socket_data( selected_fd, u16_network_flag );
        }
        else if( ( netlink_fd != -1 ) && FD_ISSET( netlink_fd, &readFds ) )
        {
		  uint8_t buffer[4096];
		  recv(netlink_fd, buffer, 4096, 0);

          DBG_INFO2("Got address change notification");
          handle_address_change_event();

          FD_CLR( netlink_fd, &readFds );
        }
        else if( ( shutdown_fds[0] != -1 ) && FD_ISSET( shutdown_fds[0], &readFds ) )
        {
          FD_CLR( shutdown_fds[0], &readFds );
          char    buf[10] = { 0 };
          ssize_t len     = read( shutdown_fds[0], buf, sizeof( buf ) );

          if( -1 == len )
          {
            continue;
          }

          break;
        }
        else
        {
          break;
        }
      }
    }
    else
    {
      continue;
    }
  }
}

void IpAdapterImplLinux::handle_received_socket_data( int &selected_fd, uint16_t &u16_network_flag )
{
  int                     level           = 0;
  int                     type            = 0;
  int                     namelen         = 0;
  struct sockaddr_storage packet_src_addr = { .ss_family = 0 };

  size_t                  len            = 0;
  struct cmsghdr *        cmp            = nullptr;
  const auto              is_ipv6_packet = is_bit_set( u16_network_flag, k_network_flag_ipv6 );
  struct iovec            iov            = { .iov_base = &_pu8_receive_buffer[0], .iov_len = COAP_MAX_PDU_SIZE };

  union control
  {
    struct cmsghdr cmsg;
    unsigned char  data[CMSG_SPACE( sizeof( struct in6_pktinfo ) )];
  } cmsg;

  if( is_ipv6_packet )
  {
    namelen = sizeof( struct sockaddr_in6 );
    level   = IPPROTO_IPV6;
    type    = IPV6_PKTINFO;
    len     = sizeof( struct in6_pktinfo );
  }
  else
  {
    namelen = sizeof( struct sockaddr_in );
    level   = IPPROTO_IP;
    type    = IP_PKTINFO;
    len     = sizeof( struct in_pktinfo );
  }

  struct msghdr msg = { .msg_name       = &packet_src_addr,
                        .msg_namelen    = (socklen_t) namelen,
                        .msg_iov        = &iov,
                        .msg_iovlen     = 1,
                        .msg_control    = &cmsg,
                        .msg_controllen = CMSG_SPACE( len ) };

  ssize_t       recvLen = recvmsg( selected_fd, &msg, u16_network_flag );

  if( recvLen == -1 )
  {
    return;
  }

  unsigned char *pktinfo = nullptr;

  for( cmp = CMSG_FIRSTHDR( &msg ); cmp != nullptr; cmp = CMSG_NXTHDR( &msg, cmp ) )
  {
    if( ( cmp->cmsg_level == level ) && ( cmp->cmsg_type == type ) )
    {
      pktinfo = CMSG_DATA( cmp );
    }
  }

  if( !pktinfo )
  {
    return;
  }

  uint16_t u16_recvd_port = 0;
  IpAddress ip_addr{};

  if( packet_src_addr.ss_family == AF_INET )
  {
    auto pst_ipv4_addr = reinterpret_cast<struct sockaddr_in *>( &packet_src_addr );
    u16_recvd_port = ntohs( pst_ipv4_addr->sin_port );
    ip_addr.set_addr( reinterpret_cast<uint8_t *>( &pst_ipv4_addr->sin_addr ), IpAddrFamily::IPv4 );
  }
  else
  {
    auto pst_ipv6_addr = reinterpret_cast<struct sockaddr_in6 *>( &packet_src_addr );
    u16_recvd_port = ntohs( pst_ipv6_addr->sin6_port );
    ip_addr.set_addr( reinterpret_cast<uint8_t *>( &pst_ipv6_addr->sin6_addr ), IpAddrFamily::IPv6 );
  }

#ifdef _NETWORK_DEBUG_
  unsigned char ac_ascii_addr[70] = { 0 };
  ip_addr.to_string( &ac_ascii_addr[0], 70 );
  DBG_INFO2( "received packet from %s", &ac_ascii_addr[0] );
#endif /* _NETWORK_DEBUG_ */

  const auto u32_if_index = is_ipv6_packet
    ? reinterpret_cast<struct in6_pktinfo *>( pktinfo )->ipi6_ifindex
    : reinterpret_cast<struct in_pktinfo *>( pktinfo )->ipi_ifindex;

  if(this->_event_handler)
  {
	    Endpoint endpoint{ k_adapter_type_ip, u16_network_flag, u16_recvd_port, u32_if_index, ip_addr };
	    auto         received_data = new uint8_t[recvLen];
	    memcpy( &received_data[0], &_pu8_receive_buffer[0], recvLen );

	    AdapterEvent adapter_event( ADAPTER_EVENT_TYPE_PACKET_RECEIVED, &endpoint, &received_data[0], recvLen, k_adapter_type_ip );

	    this->_event_handler->handle_event(adapter_event);
  }
}

void intl_add_socket_to_fd_array( UdpSocketImplLinux *pcz_udp_socket, fd_set *readFds, int *max_fd )
{
  if( ( pcz_udp_socket != nullptr ) && ( pcz_udp_socket->get_socket() != -1 ) )
  {
    FD_SET( pcz_udp_socket->get_socket(), readFds );

    if( pcz_udp_socket->get_socket() > *max_fd )
    {
      *max_fd = pcz_udp_socket->get_socket();
    }
  }
}

ErrCode IpAdapterImplLinux::do_pre_stop_server()
{
  if( shutdown_fds[1] != -1 )
  {
    close( shutdown_fds[1] );
  }

  return ( ErrCode::OK );
}

void IpAdapterImplLinux::update_max_fd(int new_fd)
{
	if(new_fd > max_fd)
	{
		max_fd = new_fd;
	}
}
}
}

#endif /* _OS_LINUX_ */
