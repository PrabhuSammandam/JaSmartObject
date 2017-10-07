/*
 * EndPoint.cpp
 *
 *  Created on: Jul 16, 2017
 *      Author: psammand
 */

#include <end_point.h>
#include <konstants_network.h>
#include <cstring>

namespace ja_iot {
namespace network {
using namespace ja_iot::base;

Endpoint & Endpoint::operator = ( _in_ const Endpoint &other )
{
  this->_adapter_type  = other._adapter_type;
  this->_network_flags = other._network_flags;
  this->_port          = other._port;
  this->_if_index      = other._if_index;
  this->_addr          = other._addr;
  return ( *this );
}

void Endpoint::set_addr( _in_ IpAddress *addr )
{
  if( addr != nullptr )
  {
    _addr = *addr;
  }
}

bool Endpoint::set_ipv6_addr_by_scope( _in_ uint16_t ipv6_scope )
{
  bool ret_status = false;

  _addr.set_addr_by_scope( ipv6_scope, 158 );
  return ( ret_status );
}
}
}