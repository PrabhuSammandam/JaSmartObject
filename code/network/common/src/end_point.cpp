/*
 * EndPoint.cpp
 *
 *  Created on: Jul 16, 2017
 *      Author: psammand
 */

#include "base_utils.h"
#include <end_point.h>

namespace ja_iot {
  namespace network {
    using namespace base;
    Endpoint::Endpoint( const uint16_t u16_adapter_type, const uint16_t u16_network_flags, const uint16_t u16_port, const uint32_t u32_if_index, const IpAddress &rcz_ip_addr )
      : _u16_adapter_type{ u16_adapter_type },
      _u16_network_flags{ u16_network_flags }, _u16_port{ u16_port },
      _u32_if_index{ u32_if_index }, _cz_ip_addr{ rcz_ip_addr }
    {
    }

    void Endpoint::set_addr( _in_ const IpAddress &rcz_addr )
    {
      this->_cz_ip_addr = rcz_addr;
    }

    bool Endpoint::set_ipv6_addr_by_scope( _in_ const uint16_t u16_ipv6_scope )
    {
      const auto ret_status = false;

      _cz_ip_addr.set_addr_by_scope( u16_ipv6_scope, uint8_t( 158 ) );
      return ( ret_status );
    }

    uint16_t Endpoint::get_transport_scheme() const
    {
      if( is_bit_set( _u16_adapter_type, k_adapter_type_ip ) &&
        ( is_bit_set( _u16_network_flags, k_network_flag_ipv4 ) ||
        is_bit_set( _u16_network_flags, k_network_flag_ipv6 ) ) )
      {
        if( is_bit_set( _u16_network_flags, k_network_flag_secure ) )
        {
          return ( k_transport_scheme_coaps );
        }

        return ( k_transport_scheme_coap );
      }

      if( is_bit_set( _u16_adapter_type, k_adapter_type_tcp ) &&
        ( is_bit_set( _u16_network_flags, k_network_flag_ipv4 ) ||
        is_bit_set( _u16_network_flags, k_network_flag_ipv6 ) ) )
      {
        if( is_bit_set( _u16_network_flags, k_network_flag_secure ) )
        {
          return ( k_transport_scheme_coaps_tcp );
        }

        return ( k_transport_scheme_coap_tcp );
      }

      return ( k_transport_scheme_none );
    }

    bool Endpoint::is_valid()
    {
      return ( _cz_ip_addr.is_valid() );
    }

    bool Endpoint::is_multicast()
    {
      return ( ( _u16_network_flags & k_network_flag_multicast ) != 0 );
    }

    bool operator == ( Endpoint & rcz_endpoint1, Endpoint & rcz_endpoint2 ) {
      return ( ( rcz_endpoint1.get_addr() == rcz_endpoint2.get_addr() ) &&
             ( rcz_endpoint1.get_port() == rcz_endpoint2.get_port() ) );
    }
  } // namespace network
} // namespace ja_iot
