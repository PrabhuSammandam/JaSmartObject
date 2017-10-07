/*
 * IpAddress.h
 *
 *  Created on: Jul 15, 2017
 *      Author: psammand
 */

#ifndef IPADDRESS_H_
#define IPADDRESS_H_

#include <cstdint>
#include <common_defs.h>

namespace ja_iot {
namespace base {
enum class IpAddrFamily
{
  IPV4 = 0,
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

class IpAddress
{
  public:

    IpAddress( IpAddrFamily ip_address_family = IpAddrFamily::IPV4 );

    IpAddress( const char *ascii_addr, IpAddrFamily ip_address_family = IpAddrFamily::IPV4 );

    IpAddress( const IpAddress &other );

    IpAddress( uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4 );

    IpAddress( Ipv6AddrScope scope, uint16_t address );

    ~IpAddress ();

  public:
    bool         is_broadcast();
    bool         is_multicast();
    bool         is_unicast();
    bool         is_ipv4();
    bool         is_ipv6();
    uint32_t     as_u32();
    uint8_t*     get_addr() { return ( &address_[0] ); }
    IpAddrFamily get_addr_family() { return ( address_family_ ); }
    void         set_addr_family( _in_ IpAddrFamily addr_family ) { address_family_ = addr_family; }
    uint8_t      get_scope_id() { return ( scope_id_ ); }
    void         set_scope_id( _in_ uint8_t scope_id ) { scope_id_ = scope_id; }
    void         set_addr_by_scope( _in_ uint8_t scope_id, _in_ uint16_t address );
    void         set_addr( _in_ uint32_t ipv4_addr );
    void         set_addr( _in_ uint8_t *ip_addr, _in_ IpAddrFamily ip_addr_family );

    bool operator == (const IpAddress &other);
    /***
     * Converts the internal binary address into ascii address. Pass always 1 byte
     * extra for null termination. If passed buffer is less than the actual representation
     * then only partial address will be converted to ascii.
     *
     * For IPv4 address pass the buffer with length 4*3 + 3*1 = 15 + 1 = 16
     * 		"xxx.xxx.xxx.xxx\0" = 16
     * For IPv6 address pass the buffer with length (6 * 4 + 5) + 1 + (4 * 3 + 3) = 29 + 1 + 15 = 45 + 1 = 46
     * 		"0000:0000:0000:0000:0000:0000:127.127.127.127\0" = 46
     * @param buf
     * @param buf_len
     */
    void         to_string( _in_out_ uint8_t *buf, _in_ uint8_t buf_len );

  public:
    static Ipv6AddrScope get_addr_scope( _in_ const char *ipv6_ascii_addr );
    static bool          from_string( _in_ const char *addr_string, _in_ IpAddrFamily ip_addr_family, _out_ IpAddress &ip_address );

  private:
    IpAddrFamily   address_family_ = IpAddrFamily::IPV4;
    uint8_t        scope_id_       = 0;
    uint8_t        address_[16]    = { 0 };
};
}
}

#endif /* IPADDRESS_H_ */
