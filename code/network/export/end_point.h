/*
 * EndPoint.h
 *
 *  Created on: Jun 30, 2017
 *      Author: psammand
 */

#pragma once

#include "Hash.h"
#include <base_consts.h>
#include <common_defs.h>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <ip_addr.h>

namespace ja_iot {
namespace network {
using namespace base;
class Endpoint
{
  public:
    Endpoint () : _cz_ip_addr{} {}
    Endpoint( uint16_t u16_adapter_type, uint16_t u16_network_flags, uint16_t port, uint32_t u32_if_index, const base::IpAddress &rcz_ip_addr );

    uint16_t       get_adapter_type() const;
    void           set_adapter_type( _in_ const uint16_t adapter_type ) { _u16_adapter_type = adapter_type; }
    uint32_t       get_if_index() const { return ( _u32_if_index ); }
    void           set_if_index( _in_ const uint32_t if_index ) { _u32_if_index = if_index; }
    uint16_t       get_network_flags() const { return ( _u16_network_flags ); }
    void           set_network_flags( _in_ const uint16_t network_flags ) { _u16_network_flags = network_flags; }
    uint16_t       get_port() const { return ( _u16_port ); }
    void           set_port( _in_ const uint16_t port ) { _u16_port = port; }
    base::IpAddress& get_addr() { return ( _cz_ip_addr ); }
    void           set_addr( _in_ const base::IpAddress &rcz_addr );
    bool           set_ipv6_addr_by_scope( _in_ uint16_t ipv6_scope );
    uint16_t       get_transport_scheme() const;
    bool           is_valid();
    bool           is_multicast();
    bool           is_ipv4();
    bool           is_ipv6();
    bool           is_secure();
    bool           is_ip_adapter()const { return ( _u16_adapter_type == k_adapter_type_ip ); }
    bool           is_btle_adapter()const { return ( _u16_adapter_type == k_adapter_type_bt_le ); }

  private:
    uint16_t          _u16_adapter_type  = k_adapter_type_default;
    uint16_t          _u16_network_flags = k_network_flag_none;
    uint16_t          _u16_port          = 0;
    uint32_t          _u32_if_index      = 0;
    base::IpAddress   _cz_ip_addr;

  public:
    Endpoint( _in_ const Endpoint &other )               = default;
    Endpoint & operator = ( _in_ const Endpoint &other ) = default;
    Endpoint( Endpoint &&other )   noexcept              = default;
    Endpoint & operator = ( Endpoint &&other ) noexcept  = default;
};

bool operator == ( Endpoint &ep1, Endpoint &ep2 );

}   // namespace network
} // namespace ja_iot

// custom specialization of std::hash can be injected in namespace std
namespace std {
template<> struct hash<ja_iot::network::Endpoint>
{
  typedef ja_iot::network::Endpoint argument_type;
  typedef size_t result_type;
  result_type operator () ( argument_type const &s ) const noexcept
  {
    auto       &endpoint    = const_cast<argument_type &>( s );
    const auto ip_addr_hash =
      hash<ja_iot::base::IpAddress> {} ( endpoint.get_addr() );
    auto       port = endpoint.get_port();

    return ( ja_iot::base::Hash::get_hash( &port, 2, ip_addr_hash ) );
  }
};
} // namespace std
