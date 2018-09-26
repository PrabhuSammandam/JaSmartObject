#pragma once

#include <ip_addr.h>

namespace ja_iot
{
  namespace base
  {
    class IpSocketAddr
    {
      IpAddress _ip_addr;
      uint16_t _port = 0;

    public:

      IpSocketAddr();

      IpSocketAddr(const IpAddress& ip_addr, uint16_t port);

      IpSocketAddr(const uint8_t* ip_addr, uint16_t port);

      ~IpSocketAddr();

      uint16_t Port() const { return _port; }
      void Port(const uint16_t port) { _port = port; }

      IpAddress* IpAddr() { return &_ip_addr; }
      void IpAddr(const IpAddress& ipAddr) { _ip_addr = ipAddr; }

      bool operator ==(const IpSocketAddr& other);

      void print() const;

      uint32_t get_hash_value();
    };
  }
}
