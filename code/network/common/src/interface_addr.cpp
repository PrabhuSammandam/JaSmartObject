/*
 * InterfaceAddress.cpp
 *
 *  Created on: Jul 25, 2017
 *      Author: psammand
 */

#include <interface_addr.h>

namespace ja_iot {
namespace network {
InterfaceAddress::InterfaceAddress( uint32_t index, uint32_t flags, IpAddrFamily family, const char *address ) :
  index_{ index }, flags_{ flags }, family_{ family }
{
  set_addr( address );
}

InterfaceAddress::InterfaceAddress( const InterfaceAddress &other )
{
  *this = other;
}

bool InterfaceAddress::operator == ( const InterfaceAddress &other )
{
  return ( index_ == other.index_ && family_ == other.family_ &&
         ( strcmp( &address_[0], &other.address_[0] ) == 0 ) );
}

void InterfaceAddress::set_addr( const char *addr )
{
  if( addr != nullptr )
  {
    for( int i = 0; i < kInterfaceAddressMaxSize; ++i )
    {
      if( addr[i] != '\0' )
      {
        this->address_[i] = addr[i];
      }
      else
      {
        this->address_[i] = 0;
      }
    }
  }
}

void InterfaceAddress::clear_address()
{
  for( int i = 0; i < kInterfaceAddressMaxSize; ++i )
  {
    address_[i] = 0;
  }
}

InterfaceAddress & InterfaceAddress::operator = ( const InterfaceAddress &other )
{
  this->index_  = other.index_;
  this->flags_  = other.flags_;
  this->family_ = other.family_;

  for( int i = 0; i < kInterfaceAddressMaxSize; ++i )
  {
    this->address_[i] = other.address_[i];
  }

  return ( *this );
}
}
}
