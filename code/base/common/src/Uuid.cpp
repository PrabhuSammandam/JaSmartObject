/*
 * Uuid.cpp
 *
 *  Created on: Nov 17, 2018
 *      Author: psammand
 */

#include "Uuid.h"
#include "string.h"

namespace ja_iot{
namespace base {
std::string hex_to_str( uint8_t *data, int len );
Uuid::Uuid () : _uuid{ 0 }
{
}
Uuid::Uuid( const Uuid &other )
{
  *this = other;
}
Uuid::Uuid( Uuid &&other )
{
  *this = std::move( other );
}

Uuid & Uuid::operator = ( const Uuid &other )
{
  if( &other != this )
  {
    ::memcpy( &_uuid[0], &other._uuid[0], 16 );
  }

  return ( *this );
}

Uuid & Uuid::operator = ( Uuid &&other )
{
  if( &other != this )
  {
    ::memcpy( &_uuid[0], &other._uuid[0], 16 );
  }

  return ( *this );
}

Uuid & Uuid::operator = ( std::string &uuid_string )
{
  from_string( uuid_string );

  return ( *this );
}

bool Uuid::operator == ( const Uuid &other )
{
  return ( ::memcmp( (const void *) &_uuid[0], (const void *) &other._uuid[0], 16 ) == 0 );
}

void Uuid::operator >> ( std::string &uuid_string )
{
  uuid_string = to_string();
}

Uuid & Uuid::operator << ( std::string &uuid_string )
{
  from_string( uuid_string );

  return ( *this );
}

void Uuid::clear()
{
	::memset(&_uuid[0], 0, 16);
}

bool Uuid::is_nil()
{
  for( auto i = 0; i < 16; i++ )
  {
    if( _uuid[i] != 0 )
    {
      return ( false );
    }
  }

  return ( true );
}

std::string Uuid::to_string()
{
  return ( hex_to_str( &_uuid[0], 17 ) );
}

void Uuid::from_string( std::string &uuid_string )
{
}

constexpr char hexmap[] = { '0', '1', '2', '3', '4', '5', '6', '7',
                            '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

std::string hex_to_str( uint8_t *data, int len )
{
  std::string s( len * 2, ' ' );

  for( int i = 0; i < len; ++i )
  {
    s[2 * i]     = hexmap[( data[i] & 0xF0 ) >> 4];
    s[2 * i + 1] = hexmap[data[i] & 0x0F];
  }

  return ( s );
}
}
}
