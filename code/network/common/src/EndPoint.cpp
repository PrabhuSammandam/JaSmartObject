/*
 * EndPoint.cpp
 *
 *  Created on: Jul 16, 2017
 *      Author: psammand
 */

#include "EndPoint.h"
#include "konstants.h"

namespace ja_iot {
namespace network {
Endpoint & Endpoint::operator = ( const Endpoint &other )
{
  this->adapter_type_  = other.adapter_type_;
  this->network_flags_ = other.network_flags_;
  this->port_          = other.port_;
  this->if_index_      = other.if_index_;
  strcpy( (char *) ( &this->addr_[0] ), (const char *) ( &other.addr_[0] ) );
  return ( *this );
}

void Endpoint::setAddr( const char *addr )
{
  if( addr != nullptr )
  {
    strcpy( (char *) &addr_, addr );
  }
}

bool Endpoint::SetIpv6AddrByScope( NetworkFlag ipv6_scope )
{
  bool ret_status{ false };
  const char *scope_addr = nullptr;

  switch( ipv6_scope )
  {
    case NetworkFlag::SCOPE_INTERFACE_LOCAL:
    {
      scope_addr = IPV6_MULTICAST_ADDRESS_INTERFACE_LOCAL;
    }
    break;
    case NetworkFlag::SCOPE_LINK_LOCAL:
    {
      scope_addr = IPV6_MULTICAST_ADDRESS_LINK_LOCAL;
    }
    break;
    case NetworkFlag::SCOPE_REALM_LOCAL:
    {
      scope_addr = IPV6_MULTICAST_ADDRESS_REALM_LOCAL;
    }
    break;
    case NetworkFlag::SCOPE_ADMIN_LOCAL:
    {
      scope_addr = IPV6_MULTICAST_ADDRESS_ADMIN_LOCAL;
    }
    break;
    case NetworkFlag::SCOPE_SITE_LOCAL:
    {
      scope_addr = IPV6_MULTICAST_ADDRESS_SITE_LOCAL;
    }
    break;
    case NetworkFlag::SCOPE_ORG_LOCAL:
    {
      scope_addr = IPV6_MULTICAST_ADDRESS_ORGANIZATION_LOCAL;
    }
    break;
    case NetworkFlag::SCOPE_GLOBAL:
    {
      scope_addr = IPV6_MULTICAST_ADDRESS_GLOBAL;
    }
    break;
    default:
    {
    }
    break;
  }

  if( scope_addr != nullptr )
  {
    setAddr( scope_addr );
  }

  return ( ret_status );
}
}
}