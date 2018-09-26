#include <ip_socket_addr.h>
#include <Hash.h>
#include <stdio.h>

namespace ja_iot
{
  namespace base
  {
    IpSocketAddr::IpSocketAddr() : _ip_addr{}
    {
    }

    IpSocketAddr::IpSocketAddr(const IpAddress& ipAddr, const uint16_t port) : _ip_addr{ipAddr}, _port{port}
    {
    }

    IpSocketAddr::IpSocketAddr(const uint8_t* ipAddr, const uint16_t port) : _ip_addr{
      reinterpret_cast<const char *>(ipAddr)
    }, _port{port}
    {
    }

    IpSocketAddr::~IpSocketAddr()
    {
      _ip_addr.~IpAddress();
      _port = 0;
    }

    bool IpSocketAddr::operator ==(const IpSocketAddr& other)
    {
      return _port == other._port && _ip_addr == other._ip_addr;
    }

    void IpSocketAddr::print() const
    {
      printf("IP:");
      //    _ipAddr.Print();
      printf("Port:%u\n", _port);
    }

    uint32_t IpSocketAddr::get_hash_value()
    {
      auto hashVal = Hash::get_hash(_ip_addr.get_addr(), sizeof( IpAddress));

      hashVal = Hash::get_hash(&_port, sizeof( uint16_t), hashVal);

      return hashVal;
    }
  }
}
