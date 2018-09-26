/*
 * IpAdapterConfig.h
 *
 *  Created on: Sep 11, 2017
 *      Author: psammand
 */

#pragma once
#include <bitset>
#include <array>
#include <cstdint>
#include <konstants_network.h>

namespace ja_iot {
namespace network {
constexpr uint8_t IP_ADAPTER_CONFIG_IPV4_UCAST         = 0;
constexpr uint8_t IP_ADAPTER_CONFIG_IPV4_UCAST_SECURE  = 1;
constexpr uint8_t IP_ADAPTER_CONFIG_IPV4_MCAST         = 2;
constexpr uint8_t IP_ADAPTER_CONFIG_IPV4_MCAST_SECURE  = 3;
constexpr uint8_t IP_ADAPTER_CONFIG_IPV6_UCAST         = 4;
constexpr uint8_t IP_ADAPTER_CONFIG_IPV6_UCAST_SECURE  = 5;
constexpr uint8_t IP_ADAPTER_CONFIG_IPV6_MCAST         = 6;
constexpr uint8_t IP_ADAPTER_CONFIG_IPV6_MCAST_SECURE  = 7;
constexpr uint8_t IP_ADAPTER_CONFIG_DEVICE_TYPE_CLIENT = 8;
constexpr uint8_t IP_ADAPTER_CONFIG_DEVICE_TYPE_SERVER = 9;

class IpAdapterConfig
{
  public:
    IpAdapterConfig ()
    {
      set_flag( IP_ADAPTER_CONFIG_DEVICE_TYPE_SERVER, true );
      set_port( IP_ADAPTER_CONFIG_IPV4_MCAST, COAP_PORT );
      set_port( IP_ADAPTER_CONFIG_IPV6_MCAST, COAP_PORT );
      set_port( IP_ADAPTER_CONFIG_IPV4_MCAST_SECURE, COAP_SECURE_PORT );
      set_port( IP_ADAPTER_CONFIG_IPV6_MCAST_SECURE, COAP_SECURE_PORT );
    }

    void set_flag( const uint8_t config_flag, const bool value ) { _flags.set( config_flag, value ); }
    bool is_flag_set( const uint8_t config_flag ) const { return ( _flags[config_flag] ); }

    bool is_device_client() const { return ( is_flag_set( IP_ADAPTER_CONFIG_DEVICE_TYPE_CLIENT ) ); }
    bool is_device_server() const { return ( is_flag_set( IP_ADAPTER_CONFIG_DEVICE_TYPE_SERVER ) ); }

    uint16_t get_port( const uint8_t u8_config_type ) const { return ( _ports[u8_config_type] ); }
    void     set_port( const uint8_t u8_config_type, const uint16_t u16_port ) { _ports[u8_config_type] = u16_port; }
    uint16_t get_current_port( const uint8_t u8_config_type ) const { return ( _cur_ports[u8_config_type] ); }
    void     set_current_port( const uint8_t u8_config_type, const uint16_t u16_port ) { _cur_ports[u8_config_type] = u16_port; }

    bool is_ipv4_ucast_enabled() { return ( is_flag_set( IP_ADAPTER_CONFIG_IPV4_UCAST ) ); }
    bool is_ipv4_mcast_enabled() { return ( is_flag_set( IP_ADAPTER_CONFIG_IPV4_MCAST ) ); }
    bool is_ipv4_secure_ucast_enabled() { return ( is_flag_set( IP_ADAPTER_CONFIG_IPV4_UCAST_SECURE ) ); }
    bool is_ipv4_secure_mcast_enabled() { return ( is_flag_set( IP_ADAPTER_CONFIG_IPV4_MCAST_SECURE ) ); }

    bool is_ipv6_ucast_enabled() { return ( is_flag_set( IP_ADAPTER_CONFIG_IPV6_UCAST ) ); }
    bool is_ipv6_mcast_enabled() { return ( is_flag_set( IP_ADAPTER_CONFIG_IPV6_MCAST ) ); }
    bool is_ipv6_secure_ucast_enabled() { return ( is_flag_set( IP_ADAPTER_CONFIG_IPV6_UCAST_SECURE ) ); }
    bool is_ipv6_secure_mcast_enabled() { return ( is_flag_set( IP_ADAPTER_CONFIG_IPV6_MCAST_SECURE ) ); }

    bool is_ipv4_enabled() const
    {
      return ( is_flag_set( IP_ADAPTER_CONFIG_IPV4_UCAST )
             || is_flag_set( IP_ADAPTER_CONFIG_IPV4_UCAST_SECURE )
             || is_flag_set( IP_ADAPTER_CONFIG_IPV4_MCAST_SECURE )
             || is_flag_set( IP_ADAPTER_CONFIG_IPV4_MCAST ) );
    }

    bool is_ipv6_enabled() const
    {
      return ( is_flag_set( IP_ADAPTER_CONFIG_IPV6_UCAST )
             || is_flag_set( IP_ADAPTER_CONFIG_IPV6_UCAST_SECURE )
             || is_flag_set( IP_ADAPTER_CONFIG_IPV6_MCAST_SECURE )
             || is_flag_set( IP_ADAPTER_CONFIG_IPV6_MCAST ) );
    }

  private:
    std::bitset<16> _flags{};
    std::array<uint16_t, 8> _ports{};
    std::array<uint16_t, 8> _cur_ports{};
};
}
}