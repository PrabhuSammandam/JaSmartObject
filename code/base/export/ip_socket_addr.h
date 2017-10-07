#pragma once

#include <ip_addr.h>

namespace ja_iot {
namespace base {
class IpSocketAddr
{
  IpAddress   _ipAddr;
  uint16_t    _port = 0;

  public:

    IpSocketAddr ();

    IpSocketAddr( const IpAddress &ipAddr, uint16_t port );

    IpSocketAddr( const uint8_t *ipAddr, uint16_t port );

    ~IpSocketAddr ();

    uint16_t Port() const { return ( _port ); }
    void     Port( uint16_t port ) { _port = port; }

    IpAddress* IpAddr() { return ( &_ipAddr ); }
    void       IpAddr( const IpAddress &ipAddr ) { _ipAddr = ipAddr; }

    bool operator == ( const IpSocketAddr &other );

    void Print();

    uint32_t GetHashValue();
};
}
}
