/*
 * IpAdapterConfig.h
 *
 *  Created on: Sep 11, 2017
 *      Author: psammand
 */

#ifndef NETWORK_EXPORT_IP_ADAPTER_CONFIG_H_
#define NETWORK_EXPORT_IP_ADAPTER_CONFIG_H_

#include <config_network.h>
#include <konstants_network.h>
#include <cstdint>

namespace ja_iot {
namespace network {
#define SET_BIT( VAL, MASK ) VAL   |= MASK
#define CLEAR_BIT( VAL, MASK ) VAL &= ( ~MASK )
#define IS_BIT_SET( VAL, MASK ) ( ( VAL & MASK ) == MASK )

enum IpAdapterConfigFlag
{
  IPV4_UCAST_ENABLED        = 0x01,
  IPV4_UCAST_SECURE_ENABLED = 0x02,
  IPV4_MCAST_ENABLED        = 0x04,
  IPV4_MCAST_SECURE_ENABLED = 0x08,
  IPV6_UCAST_ENABLED        = 0x10,
  IPV6_UCAST_SECURE_ENABLED = 0x20,
  IPV6_MCAST_ENABLED        = 0x40,
  IPV6_MCAST_SECURE_ENABLED = 0x80,
  DEVICE_TYPE_CLIENT        = 0x100,
  DEVICE_TYPE_SERVER        = 0x200,
};

class IpAdapterConfig
{
  public:

    IpAdapterConfig () { set_config_flag( DEVICE_TYPE_SERVER, true ); }

    void set_config_flag( IpAdapterConfigFlag config_flag, bool value )
    {
      if( value ){ SET_BIT( _flags_bitmask, config_flag ); }
      else{ CLEAR_BIT( _flags_bitmask, config_flag ); }
    }

    bool is_config_flag_set( IpAdapterConfigFlag config_flag ) { return ( IS_BIT_SET( _flags_bitmask, config_flag ) ); }

    bool is_device_client() { return ( is_config_flag_set( DEVICE_TYPE_CLIENT ) ); }
    bool is_device_server() { return ( is_config_flag_set( DEVICE_TYPE_SERVER ) ); }

    uint16_t getIpv4MulticastPort() const { return ( ipv4_multicast_port_ ); }
    void     setIpv4MulticastPort( uint16_t ipv4MulticastPort ) { ipv4_multicast_port_ = ipv4MulticastPort; }

    uint16_t getIpv4MulticastSecurePort() const { return ( ipv4_multicast_secure_port_ ); }
    void     setIpv4MulticastSecurePort( uint16_t ipv4MulticastSecurePort ) { ipv4_multicast_secure_port_ = ipv4MulticastSecurePort; }

    uint16_t getIpv4UnicastPort() const { return ( ipv4_unicast_port_ ); }
    void     setIpv4UnicastPort( uint16_t ipv4UnicastPort ) { ipv4_unicast_port_ = ipv4UnicastPort; }

    uint16_t getIpv4UnicastSecurePort() const { return ( ipv4_unicast_secure_port_ ); }
    void     setIpv4UnicastSecurePort( uint16_t ipv4UnicastSecurePort ) { ipv4_unicast_secure_port_ = ipv4UnicastSecurePort; }

    uint16_t getIpv6MulticastPort() const { return ( ipv6_multicast_port_ ); }
    void     setIpv6MulticastPort( uint16_t ipv6MulticastPort ) { ipv6_multicast_port_ = ipv6MulticastPort; }

    uint16_t getIpv6MulticastSecurePort() const { return ( ipv6_multicast_secure_port_ ); }
    void     setIpv6MulticastSecurePort( uint16_t ipv6MulticastSecurePort ) { ipv6_multicast_secure_port_ = ipv6MulticastSecurePort; }

    uint16_t getIpv6UnicastPort() const { return ( ipv6_unicast_port_ ); }
    void     setIpv6UnicastPort( uint16_t ipv6UnicastPort ) { ipv6_unicast_port_ = ipv6UnicastPort; }

    uint16_t getIpv6UnicastSecurePort() const { return ( ipv6_unicast_secure_port_ ); }
    void     setIpv6UnicastSecurePort( uint16_t ipv6UnicastSecurePort ) { ipv6_unicast_secure_port_ = ipv6UnicastSecurePort; }

    uint16_t get_cur_ipv4_unicast_port() const { return ( ipv4_cur_unicast_port_ ); }
    void     set_cur_ipv4_unicast_port( uint16_t ipv4UnicastPort ) { ipv4_cur_unicast_port_ = ipv4UnicastPort; }

    uint16_t get_cur_ipv4_unicast_secure_port() const { return ( ipv4_cur_unicast_secure_port_ ); }
    void     set_cur_ipv4_unicast_secure_port( uint16_t ipv4UnicastSecurePort ) { ipv4_cur_unicast_secure_port_ = ipv4UnicastSecurePort; }

    uint16_t get_cur_ipv6_unicast_port() const { return ( ipv6_cur_unicast_port_ ); }
    void     set_cur_ipv6_unicast_port( uint16_t ipv6UnicastPort ) { ipv6_cur_unicast_port_ = ipv6UnicastPort; }

    uint16_t get_cur_ipv6_unicast_secure_port() const { return ( ipv6_cur_unicast_secure_port_ ); }
    void     set_cur_ipv6_unicast_secure_port( uint16_t ipv6UnicastSecurePort ) { ipv6_cur_unicast_secure_port_ = ipv6UnicastSecurePort; }

    bool is_ipv4_enabled()
    {
      return (
        is_config_flag_set( IpAdapterConfigFlag::IPV4_UCAST_ENABLED )
             || is_config_flag_set( IpAdapterConfigFlag::IPV4_UCAST_SECURE_ENABLED )
             || is_config_flag_set( IpAdapterConfigFlag::IPV4_MCAST_SECURE_ENABLED )
             || is_config_flag_set( IpAdapterConfigFlag::IPV4_MCAST_ENABLED )
             );
    }
    bool is_ipv6_enabled()
    {
      return (
        is_config_flag_set( IpAdapterConfigFlag::IPV6_UCAST_ENABLED )
             || is_config_flag_set( IpAdapterConfigFlag::IPV6_UCAST_SECURE_ENABLED )
             || is_config_flag_set( IpAdapterConfigFlag::IPV6_MCAST_SECURE_ENABLED )
             || is_config_flag_set( IpAdapterConfigFlag::IPV6_MCAST_ENABLED )
             );
    }

  private:
    uint16_t   _flags_bitmask = 0;

    uint16_t   ipv4_unicast_port_        = 0;
    uint16_t   ipv4_unicast_secure_port_ = 0;
    uint16_t   ipv6_unicast_port_        = 0;
    uint16_t   ipv6_unicast_secure_port_ = 0;

    uint16_t   ipv4_cur_unicast_port_        = 0;
    uint16_t   ipv4_cur_unicast_secure_port_ = 0;
    uint16_t   ipv6_cur_unicast_port_        = 0;
    uint16_t   ipv6_cur_unicast_secure_port_ = 0;

    uint16_t   ipv4_multicast_port_        = COAP_PORT;
    uint16_t   ipv4_multicast_secure_port_ = COAP_SECURE_PORT;
    uint16_t   ipv6_multicast_port_        = COAP_PORT;
    uint16_t   ipv6_multicast_secure_port_ = COAP_SECURE_PORT;
};
}
}



#endif /* NETWORK_EXPORT_IP_ADAPTER_CONFIG_H_ */
