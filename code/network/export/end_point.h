/*
 * EndPoint.h
 *
 *  Created on: Jun 30, 2017
 *      Author: psammand
 */

#pragma once

#include <common_defs.h>
#include <base_consts.h>
#include <ip_addr.h>
#include <cstdint>

namespace ja_iot {
namespace network {

class Endpoint
{
  public:

    Endpoint () {}

    Endpoint( _in_ const Endpoint &other ) { *this = other; }

    Endpoint & operator = ( _in_ const Endpoint &other );

    uint16_t get_adapter_type() const { return ( _adapter_type ); }
    void     set_adapter_type( _in_ uint16_t adapter_type ) { _adapter_type = adapter_type; }
    uint32_t get_if_index() const { return ( _if_index ); }
    void     set_if_index( _in_ uint32_t if_index ) { _if_index = if_index; }
    uint16_t get_network_flags() const { return ( _network_flags ); }
    void     set_network_flags( _in_ uint16_t network_flags ) { _network_flags = network_flags; }
    uint16_t get_port() const { return ( _port ); }
    void     set_port( _in_ uint16_t port ) { _port = port; }

    ja_iot::base::IpAddress* get_addr() { return ( &_addr ); }
    void       set_addr( _in_ ja_iot::base::IpAddress *addr );

    bool set_ipv6_addr_by_scope( _in_ uint16_t ipv6_scope );

  private:
    uint16_t    _adapter_type  = ja_iot::base::kAdapterType_default;
    uint16_t    _network_flags = ja_iot::base::kNetworkFlag_none;
    uint16_t    _port          = 0;
    uint32_t    _if_index      = 0;
    ja_iot::base::IpAddress   _addr;
};
}
}
