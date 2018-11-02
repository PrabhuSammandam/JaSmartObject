/*
 * SocketHelper.h
 *
 *  Created on: Oct 31, 2018
 *      Author: psammand
 */

#pragma once

#include "i_udp_socket.h"
#include "base_consts.h"

namespace ja_iot {
namespace network {
using namespace ja_iot::base;

class SocketHelper
{
  public:
    SocketHelper( IUdpSocket *pcz_socket ) : _pcz_socket{ pcz_socket }
    {
    }

    bool is_multicast() { return ( check_flag( k_network_flag_multicast ) ); }
    bool is_secure() { return ( check_flag( k_network_flag_secure ) ); }
    bool is_ipv4() { return ( check_flag( k_network_flag_ipv4 ) ); }
    bool is_ipv4_secure() { return ( check_flag( k_network_flag_ipv4 | k_network_flag_secure ) ); }
    bool is_ipv4_mcast() { return ( check_flag( k_network_flag_ipv4 | k_network_flag_multicast ) ); }
    bool is_ipv4_mcast_secure() { return ( check_flag( k_network_flag_ipv4 | k_network_flag_multicast | k_network_flag_secure ) ); }
    bool is_ipv6() { return ( check_flag( k_network_flag_ipv6 ) ); }
    bool is_ipv6_secure() { return ( check_flag( k_network_flag_ipv6 | k_network_flag_secure ) ); }
    bool is_ipv6_mcast() { return ( check_flag( k_network_flag_ipv6 | k_network_flag_multicast ) ); }
    bool is_ipv6_mcast_secure() { return ( check_flag( k_network_flag_ipv6 | k_network_flag_multicast | k_network_flag_secure ) ); }

  private:
    bool check_flag( uint16_t flag )
    {
      if( _pcz_socket != nullptr )
      {
        return ( ( _pcz_socket->get_flags() & flag ) == flag );
      }

      return ( false );
    }

  private:
    IUdpSocket * _pcz_socket;
};
}
}