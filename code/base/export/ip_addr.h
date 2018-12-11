/*
 * IpAddress.h
 *
 *  Created on: Jul 15, 2017
 *      Author: psammand
 */

#pragma once
#include <cstdint>
#include <cstddef>
#include <functional>
#include <common_defs.h>
#include "Hash.h"

namespace ja_iot {
namespace base {
enum class IpAddrFamily
{
  IPv4 = 0,
  IPv6 = 1
};

enum Ipv6AddrScope
{
  NONE      = 0x00,
  INTERFACE = 0x1,
  LINK      = 0x2,
  REALM     = 0x3,
  ADMIN     = 0x4,
  SITE      = 0x5,
  ORG       = 0x8,
  GLOBAL    = 0xE
};

#define IS_IPV4( __x__ ) (__x__ == IpAddrFamily::IPv4 )

class IpAddress
{
  public:
    IpAddress( IpAddrFamily ip_address_family = IpAddrFamily::IPv4 );
    IpAddress( const char *ascii_addr, IpAddrFamily ip_address_family = IpAddrFamily::IPv4 );
    IpAddress( uint8_t *pu8_addr, IpAddrFamily ip_address_family = IpAddrFamily::IPv4 );
    IpAddress( uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4 );
    IpAddress( Ipv6AddrScope scope, uint16_t address );
    ~IpAddress ();
    IpAddress( const IpAddress &other )               = default;// use the default compiler generated copy construtor.
    IpAddress( IpAddress &&other )                    = default;
    IpAddress & operator = ( const IpAddress &other ) = default;
    IpAddress & operator = ( IpAddress &&other )      = default;

    bool         is_broadcast();
    bool         is_multicast();
    bool         is_unicast();
    bool         is_ipv4();
    bool         is_ipv6();
    uint32_t     as_u32();
    uint8_t*     get_addr() { return ( &address_[0] ); }
    uint8_t			get_addr_size(){return is_ipv4() ? 4 : 16;}
    IpAddrFamily get_addr_family() const { return ( address_family_ ); }
    void         set_addr_family( _in_ const IpAddrFamily addr_family ) { address_family_ = addr_family; }
    uint8_t      get_scope_id() const { return ( scope_id_ ); }
    void         set_scope_id( _in_ const uint8_t scope_id ) { scope_id_ = scope_id; }
    void         set_addr_by_scope( _in_ uint8_t scope_id, _in_ uint16_t address );
    void         set_addr( _in_ uint32_t ipv4_addr );
    void         set_addr( _in_ uint8_t *ip_addr, _in_ IpAddrFamily ip_addr_family );
    bool         is_valid();

    bool operator == ( const IpAddress &other );
    /***
     * Converts the internal binary address into ascii address. Pass always 1 byte
     * extra for null termination. If passed buffer is less than the actual representation
     * then only partial address will be converted to ascii.
     *
     * For IPv4 address pass the buffer with length 4*3 + 3*1 = 15 + 1 = 16
     *          "xxx.xxx.xxx.xxx\0" = 16
     * For IPv6 address pass the buffer with length (6 * 4 + 5) + 1 + (4 * 3 + 3) = 29 + 1 + 15 = 45 + 1 = 46
     *          "0000:0000:0000:0000:0000:0000:127.127.127.127\0" = 46
     * @param buf
     * @param buf_len
     */
    void to_string( _in_out_ uint8_t *buf, _in_ uint8_t buf_len );

    static Ipv6AddrScope get_addr_scope( _in_ const char *ipv6_ascii_addr );
    static bool          from_string( _in_ const char *addr_string, _in_ IpAddrFamily ip_addr_family, _out_ IpAddress &ip_address );

  private:
    IpAddrFamily   address_family_ = IpAddrFamily::IPv4;
    uint8_t        scope_id_       = 0;
    uint8_t        address_[16]    = { 0 };
};
}
}

// custom specialization of std::hash can be injected in namespace std
namespace std {
template<> struct hash<ja_iot::base::IpAddress>
{
  typedef ja_iot::base::IpAddress argument_type;
  typedef size_t result_type;
  result_type operator () ( argument_type const &s ) const noexcept
  {
    auto &ip_address = const_cast<argument_type &>( s );

    return ( ja_iot::base::Hash::get_hash( ip_address.get_addr(), ip_address.get_addr_family() == ja_iot::base::IpAddrFamily::IPv4 ? 4 : 16 ) );
  }
};
}
