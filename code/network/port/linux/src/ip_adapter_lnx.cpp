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

ErrCode IpAdapterImplLinux::do_start_interface_monitor()
{
  _access_mutex = OsalMgr::Inst()->AllocMutex();

  if( _access_mutex == nullptr )
  {
    return ( ErrCode::OUT_OF_MEM );
  }

  return ( ErrCode::OK );
}

ErrCode IpAdapterImplLinux::do_stop_interface_monitor()
{
  if( _interface_addr_list.size() > 0 )
  {
    for( auto it = _interface_addr_list.cbegin(); it != _interface_addr_list.cend(); ++it )
    {
      if( ( *it ) != nullptr )
      {
        delete ( *it );
      }
    }

    _interface_addr_list.clear();
  }

  if( _access_mutex != nullptr )
  {
    OsalMgr::Inst()->FreeMutex( _access_mutex );
    _access_mutex = nullptr;
  }

  return ( ErrCode::OK );
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

      {
        ScopedMutex lock{ _access_mutex };

        bool is_found = false;

        for( size_t i = 0; i < _interface_addr_list.size(); ++i )
        {
          auto if_addr = _interface_addr_list[i];

          if( ( if_addr->get_index() == (uint32_t) ifindex ) && ( if_addr->get_family() == e_addr_family ) )
          {
            is_found = true;
            break;
          }
        }

        if( is_found == false )
        {
          auto new_if_addr = new InterfaceAddress{ *if_addr };
          _interface_addr_list.push_back( new_if_addr );

          if( this->_adapter_event_callback )
          {
            AdapterEvent adapter_event( ADAPTER_EVENT_TYPE_CONNECTION_CHANGED );
            adapter_event.set_enabled( true );

            this->_adapter_event_callback( &adapter_event, _adapter_event_cb_data );
          }
        }
      }
    }

    DBG_INFO2( "Iterating interfaces END" );

    freeifaddrs( pst_interface_addr_list );
  }

  DBG_INFO2( "EXIT" );
  return ( pcz_interface_ptr_vector );
}

std::vector<InterfaceAddress *> IpAdapterImplLinux::get_newly_found_interface_address()
{
  std::vector<InterfaceAddress *> if_array;

  char                            buf[4096] = { 0 };
  struct nlmsghdr *               nh        = nullptr;
  struct sockaddr_nl              sa        = { .nl_family = 0 };

  struct iovec                    iov = { .iov_base = buf,
                                          .iov_len  = sizeof( buf ) };

  struct msghdr                   msg = { .msg_name    = (void *) &sa,
                                          .msg_namelen = sizeof( sa ),
                                          .msg_iov     = &iov,
                                          .msg_iovlen  = 1 };

  ScopedMutex lock{ _access_mutex };

  ssize_t len = recvmsg( netlink_fd, &msg, 0 );

  for( nh = (struct nlmsghdr *) buf; NLMSG_OK( nh, len ); nh = NLMSG_NEXT( nh, len ) )
  {
    if( ( nh != nullptr ) && ( ( nh->nlmsg_type != RTM_DELADDR ) && ( nh->nlmsg_type != RTM_NEWADDR ) ) )
    {
      continue;
    }

    if( RTM_DELADDR == nh->nlmsg_type )
    {
      struct ifaddrmsg *ifa = (struct ifaddrmsg *) NLMSG_DATA( nh );

      if( ifa )
      {
        auto   ifiIndex    = ifa->ifa_index;
        auto   addr_family = ( ifa->ifa_family == AF_INET ) ? IpAddrFamily::IPv4 : IpAddrFamily::IPv6;

        bool   is_found = false;
        size_t i        = 0;

        for( i = 0; i < _interface_addr_list.size(); ++i )
        {
          auto if_addr = _interface_addr_list[i];

          if( ( if_addr->get_index() == ifiIndex ) && ( if_addr->get_family() == addr_family ) )
          {
            is_found = true;
            break;
          }
        }

        if( is_found )
        {
          auto if_addr = _interface_addr_list[i];
          _interface_addr_list.erase( _interface_addr_list.cbegin() + i );

          delete if_addr;

          if( this->_adapter_event_callback )
          {
            AdapterEvent adapter_event( ADAPTER_EVENT_TYPE_CONNECTION_CHANGED );
            adapter_event.set_enabled( false );

            this->_adapter_event_callback( &adapter_event, _adapter_event_cb_data );
          }
        }
      }

      continue;
    }

    // Netlink message type is RTM_NEWADDR.
    struct ifaddrmsg *ifa = (struct ifaddrmsg *) NLMSG_DATA( nh );

    if( ifa )
    {
      int ifiIndex = ifa->ifa_index;
      if_array = get_interface_address_for_index( ifiIndex );
    }
  }

  return ( if_array );
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
          FD_CLR( netlink_fd, &readFds );

          std::vector<InterfaceAddress *> if_addr_array     = get_newly_found_interface_address();
          const auto                      ip_adapter_config = ConfigManager::Inst().get_ip_adapter_config();

          for( auto &if_addr : if_addr_array )
          {
            if( ( if_addr->get_family() == IpAddrFamily::IPv4 ) && ip_adapter_config->is_flag_set( IP_ADAPTER_CONFIG_IPV4_MCAST ) )
            {
              start_ipv4_mcast_at_interface( if_addr->get_index() );
            }

            if( ( if_addr->get_family() == IpAddrFamily::IPv6 ) && ip_adapter_config->is_flag_set( IP_ADAPTER_CONFIG_IPV6_MCAST ) )
            {
              start_ipv6_mcast_at_interface( if_addr->get_index() );
            }
          }
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

/*
 * There are two ways to send message
 * 1. unicast message
 * 2. multicast message
 *
 * For unicast message it is straight forward and need destination address and
 * destination port.
 *
 * For multicast message it needs to be sent in all the interfaces, but the socket
 * used to send the message should be unicast socket and also before sending need to
 * select the interface to send the multicast message.
 * */

void IpAdapterImplLinux::do_handle_send_msg( IpAdapterQMsg *ip_adapter_q_msg )
{
  if( ip_adapter_q_msg == nullptr )
  {
    return;
  }

  auto       &endpoint        = ip_adapter_q_msg->end_point_;
  const auto pu8_data         = ip_adapter_q_msg->_data;
  const auto u16_data_len     = ip_adapter_q_msg->_dataLength;
  const auto network_flag     = ip_adapter_q_msg->end_point_.get_network_flags();
  const auto is_secure        = is_bit_set( network_flag, k_network_flag_secure );
  const auto is_ipv4_transfer = is_bit_set( network_flag, k_network_flag_ipv4 );
  const auto is_ipv6_transfer = is_bit_set( network_flag, k_network_flag_ipv6 );

  DBG_INFO2( "ENTER msg_data[%p], data_len[%d], mcast[%d], port[%d], adapter[%x], if_index[%d], nw_flag[%x]"
           , ip_adapter_q_msg->_data,
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

    auto if_addr_array = get_interface_address_for_index( 0 );

    for( auto &if_addr : if_addr_array )
    {
      if( ipv4_socket != nullptr && if_addr->is_ipv4())
      {
        ipv4_socket->SelectMulticastInterface( endpoint.get_addr(), if_addr->get_index() );
        ipv4_socket->SendData( endpoint.get_addr(), endpoint.get_port(), pu8_data, u16_data_len );
      }

      if( ipv6_socket != nullptr && if_addr->is_ipv6())
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

    /*
     * 1. uipv4
     * 2. usipv4
     * 3. mipv4      X
     * 4. msipv4     X
     * 5. uipv6
     * 6. usipv6
     * 7. mipv6      X
     * 8. msipv6     X
     */

    clear_bit( (uint16_t) network_flag, k_network_flag_multicast );
    auto socket = get_socket_by_mask( network_flag );

    if( socket != nullptr )
    {
      send_data( (UdpSocketImplLinux *) socket, endpoint, pu8_data, u16_data_len );
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

  if( this->_adapter_event_callback )
  {
    Endpoint endpoint{ k_adapter_type_ip, u16_network_flag, u16_recvd_port, u32_if_index, ip_addr };
    auto         received_data = new uint8_t[recvLen];
    memcpy( &received_data[0], &_pu8_receive_buffer[0], recvLen );

    AdapterEvent adapter_event( ADAPTER_EVENT_TYPE_PACKET_RECEIVED, &endpoint, &received_data[0], recvLen, k_adapter_type_ip );

    this->_adapter_event_callback( &adapter_event, _adapter_event_cb_data );
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

void IpAdapterImplLinux::send_data( UdpSocketImplLinux *pcz_udp_socket, Endpoint &rcz_endpoint, const uint8_t *pu8_data, const uint16_t u16_data_length ) const
{
  if( pcz_udp_socket == nullptr )
  {
    return;
  }

  const auto send_status = pcz_udp_socket->SendData( rcz_endpoint.get_addr(), rcz_endpoint.get_port(), const_cast<uint8_t *>( pu8_data ), u16_data_length );

  if( send_status != SocketError::OK )
  {
    AdapterEvent cz_adapter_event{ ADAPTER_EVENT_TYPE_ERROR, &rcz_endpoint, (uint8_t *) pu8_data, u16_data_length, k_adapter_type_ip };
    cz_adapter_event.set_error_code( ErrCode::SEND_DATA_FAILED );

    if( _adapter_event_callback != nullptr )
    {
      _adapter_event_callback( &cz_adapter_event, _adapter_event_cb_data );
    }
  }
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
