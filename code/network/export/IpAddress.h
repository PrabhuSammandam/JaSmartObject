/*
 * IpAddress.h
 *
 *  Created on: Jul 15, 2017
 *      Author: psammand
 */

#ifndef IPADDRESS_H_
#define IPADDRESS_H_

#include <cstdint>

namespace ja_iot {
namespace network {
enum class IpAddrFamily { IPV4, IPv6 };

enum Ipv6AddrScope {
  NONE = 0x00,
  INTERFACE = 0x1,
  LINK = 0x2,
  REALM = 0x3,
  ADMIN = 0x4,
  SITE = 0x5,
  ORG = 0x8,
  GLOBAL = 0xE
};

class IpAddress {
public:
  IpAddress(IpAddrFamily ip_address_family = IpAddrFamily::IPV4);

  IpAddress(const char *ascii_addr,
            IpAddrFamily ip_address_family = IpAddrFamily::IPV4);

  IpAddress(const IpAddress &other);

  IpAddress(uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4);

  IpAddress(Ipv6AddrScope scope, uint16_t address);

  ~IpAddress();

public:
  bool IsBroadcast();
  bool IsMulticast();
  bool IsUnicast();
  bool IsIpv4();
  bool IsIpv6();
  uint32_t AsU32();
  uint8_t *GetAddr() { return (&address_[0]); }
  IpAddrFamily GetAddrFamily() { return (address_family_); }
  void setAddrFamily(IpAddrFamily addr_family) {
    address_family_ = addr_family;
  }
  uint8_t GetScopeId() { return (scope_id_); }
  void setScopeId(uint8_t scope_id) { scope_id_ = scope_id; }
  void SetAddrByScope(uint8_t scope_id, uint16_t address);

public:
  static Ipv6AddrScope get_addr_scope(const char *ipv6_ascii_addr);
  static bool from_string(IpAddress &ip_address, IpAddrFamily ip_addr_family,
                          const char *addr_string);

private:
  IpAddrFamily address_family_ = IpAddrFamily::IPV4;
  uint8_t scope_id_ = 0;
  uint8_t address_[16] = {0};
};
}
}

#endif /* IPADDRESS_H_ */
