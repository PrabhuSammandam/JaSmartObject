/*
 * InterfaceAddress.cpp
 *
 *  Created on: Jul 25, 2017
 *      Author: psammand
 */

#include <cstring>
#include <interface_addr.h>
#include <ip_addr.h>


namespace ja_iot {
namespace network {
using namespace base;
InterfaceAddress::InterfaceAddress( const uint32_t index, const uint32_t flags, const base::IpAddrFamily family, const char *address ) :
  index_{ index }, flags_{ flags }, family_{ family }
{
  set_addr( family, address );
}
InterfaceAddress::InterfaceAddress( const InterfaceAddress &other )
{
  *this = other;
}

bool InterfaceAddress::operator == ( const InterfaceAddress &other )
{
  return ( index_ == other.index_ && family_ == other.family_ &&
         strcmp( &address_[0], &other.address_[0] ) == 0 );
}

void InterfaceAddress::set_addr( const IpAddrFamily family, const char *addr )
{
  if( addr != nullptr )
  {
    if( family == IpAddrFamily::IPv4 )
    {
      memcpy( &this->address_[0], addr, 4 );
    }
    else if( family == IpAddrFamily::IPv6 )
    {
      memcpy( &this->address_[0], addr, 16 );
    }
  }
}

void InterfaceAddress::clear_address()
{
  for( int i = 0; i < 16; ++i )
  {
    address_[i] = 0;
  }
}

InterfaceAddress & InterfaceAddress::operator = ( const InterfaceAddress &other )
{
  this->index_  = other.index_;
  this->flags_  = other.flags_;
  this->family_ = other.family_;

  for( auto i = 0; i < 16; ++i )
  {
    this->address_[i] = other.address_[i];
  }

  return ( *this );
}
}
}