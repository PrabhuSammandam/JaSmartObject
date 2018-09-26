/*
 * InterfaceAddress.h
 *
 *  Created on: Jul 5, 2017
 *      Author: psammand
 */

#pragma once

#include <ip_addr.h>
#include <cstdint>
#include <array>

constexpr uint16_t kInterfaceAddressMaxSize = 66;

namespace ja_iot {
namespace network {
class InterfaceAddress
{
  public:
    InterfaceAddress ()
    {
    }
    InterfaceAddress( uint32_t index, uint32_t flags, base::IpAddrFamily family, const char *address );
    InterfaceAddress( const InterfaceAddress &other );

    base::IpAddrFamily get_family() const { return ( family_ ); }
    void               set_family( const base::IpAddrFamily family ) { family_ = family; }
    uint32_t           get_flags() const { return ( flags_ ); }
    void               set_flags( const uint32_t flags ) { flags_ = flags; }
    uint32_t           get_index() const { return ( index_ ); }
    void               set_index( const uint32_t index ) { index_ = index; }
    void               set_addr( base::IpAddrFamily family, const char *addr );
    char*              get_addr() { return ( &address_[0] ); }

    bool is_ipv4() { return ( family_ == base::IpAddrFamily::IPv4 ); }
    bool is_ipv6() { return ( family_ == base::IpAddrFamily::IPv6 ); }

    InterfaceAddress & operator = ( const InterfaceAddress &other );
    bool operator               == ( const InterfaceAddress &other );

    void clear_address();

  private:

    uint32_t               index_   = 0;
    uint32_t               flags_   = 0;
    base::IpAddrFamily     family_  = base::IpAddrFamily::IPv4;
    std::array<char, 16>   address_ = { { 0 } };
};
}
}