/*
 * IpAdapterConfig.h
 *
 *  Created on: Sep 11, 2017
 *      Author: psammand
 */

#ifndef NETWORK_EXPORT_IPADAPTERCONFIG_H_
#define NETWORK_EXPORT_IPADAPTERCONFIG_H_

#include <cstdint>
#include <konstants.h>
#include <network_config.h>

namespace ja_iot {
namespace network {
#define SET_BIT( VAL, MASK ) VAL   |= MASK
#define CLEAR_BIT( VAL, MASK ) VAL &= ( ~MASK )
#define IS_BIT_SET( VAL, MASK ) ( ( VAL & MASK ) == MASK )

enum DeviceType
{
  CLIENT = 0x01,
  SERVER = 0x02
};

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
      if( value ){ SET_BIT( data_, config_flag ); }
      else{ CLEAR_BIT( data_, config_flag ); }
    }

    bool is_config_flag_set( IpAdapterConfigFlag config_flag ) { return ( IS_BIT_SET( data_, config_flag ) ); }

    bool is_device_client() { return ( is_config_flag_set( DEVICE_TYPE_CLIENT ) ); }
    bool is_device_server() { return ( is_config_flag_set( DEVICE_TYPE_SERVER ) ); }

    uint16_t getClientNetworkFlag() const { return ( client_network_flag_ ); }
    void     setClientNetworkFlag( uint16_t clientNetworkFlag = 0 ) { client_network_flag_ = clientNetworkFlag; }

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

    uint16_t getServerNetworkFlag() const { return ( server_network_flag_ ); }
    void     setServerNetworkFlag( uint16_t serverNetworkFlag ) { server_network_flag_ = serverNetworkFlag; }

    bool is_ipv4_enabled() { return ( (client_network_flag_ &NetworkFlag::IPV4) || ( server_network_flag_ & NetworkFlag::IPV4 ) ); }
    bool is_ipv6_enabled() { return ( (client_network_flag_ &NetworkFlag::IPV6) || ( server_network_flag_ & NetworkFlag::IPV6 ) ); }

  private:
    uint16_t   data_                       = 0;
    uint16_t   client_network_flag_        = 0;
    uint16_t   server_network_flag_        = 0;
    uint16_t   ipv4_unicast_port_          = 0;
    uint16_t   ipv4_unicast_secure_port_   = 0;
    uint16_t   ipv6_unicast_port_          = 0;
    uint16_t   ipv6_unicast_secure_port_   = 0;
    uint16_t   ipv4_multicast_port_        = COAP_PORT;
    uint16_t   ipv4_multicast_secure_port_ = COAP_SECURE_PORT;
    uint16_t   ipv6_multicast_port_        = COAP_PORT;
    uint16_t   ipv6_multicast_secure_port_ = COAP_SECURE_PORT;
};
}
}



#endif /* NETWORK_EXPORT_IPADAPTERCONFIG_H_ */
