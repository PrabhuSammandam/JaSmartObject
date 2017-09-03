/*
 * TransportType.h
 *
 *  Created on: Jun 30, 2017
 *      Author: psammand
 */

#ifndef NETWORK_EXPORT_TRANSPORTTYPE_H_
#define NETWORK_EXPORT_TRANSPORTTYPE_H_

#include <cstdint>

namespace ja_iot {
namespace network {
constexpr uint16_t MAX_NO_OF_ADAPTER_TYPES = 6;

enum class AdapterType
{
  DEFAULT       = 0x00,
  IP            = 0x01,
  TCP           = 0x02,
  NFC           = 0x04,
  REMOTE_ACCESS = 0x08,
  BT_LE         = 0x10,
  BT_EDR        = 0x20,
  ALL           = 0xFF,
};

enum class NetworkFlag
{
  DEFAULT               = 0,
  SCOPE_INTERFACE_LOCAL = 1,
  SCOPE_LINK_LOCAL      = 2,
  SCOPE_REALM_LOCAL     = 3,
  SCOPE_ADMIN_LOCAL     = 4,
  SCOPE_SITE_LOCAL      = 5,
  SCOPE_ORG_LOCAL       = 8,
  SCOPE_GLOBAL          = 0xE,
  SECURE                = ( 1 << 4 ),
  IPV4                  = ( 1 << 5 ),
  IPV6                  = ( 1 << 6 ),
  MULTICAST             = ( 1 << 7 )
};

enum class InterfaceStatusFlag
{
  UP,
  DOWN
};
}
}



#endif /* NETWORK_EXPORT_TRANSPORTTYPE_H_ */