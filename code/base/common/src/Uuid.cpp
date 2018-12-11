/*
 * Uuid.cpp
 *
 *  Created on: Nov 17, 2018
 *      Author: psammand
 */

#include "Uuid.h"
#include "string.h"

namespace ja_iot {
namespace base {
std::string hex_to_str( uint8_t *data, int len );
Uuid::Uuid () : _uuid{ 0 }
{
}
Uuid::Uuid(uint8_t * pu8_buff, uint8_t u8_buf_len)
{
	set_value(pu8_buff, u8_buf_len);
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
  ::memset( &_uuid[0], 0, 16 );
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

void Uuid::set_value(uint8_t * pu8_buff, uint8_t u8_buf_len)
{
	if (pu8_buff == nullptr || u8_buf_len != 16)
	{
		return;
	}
	::memcpy(&_uuid[0], &pu8_buff[0], 16);
}

std::string Uuid::to_string()
{
	uint8_t buf[37];

  sprintf( (char*)&buf[0], "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
    _uuid[0], _uuid[1], _uuid[2], _uuid[3],
    _uuid[4], _uuid[5], _uuid[6], _uuid[7],
    _uuid[8], _uuid[9], _uuid[10], _uuid[11],
    _uuid[12], _uuid[13], _uuid[14], _uuid[15] );

	std::string str{ (const char*)buf };
  return ( str );
}

void Uuid::from_string( std::string &uuid_string )
{
	auto str_len = uuid_string.length();

	if (str_len == 36)
	{
		size_t j = 0;
		size_t i = 0;

		for (; i < 16; i++, j += 2)
		{
			if (uuid_string[j] == '-')
			{
				j++;
			}
			sscanf(uuid_string.data() + j, "%2hhx", &_uuid[i]);
		}
	}
}

bool Uuid::is_valid_uuid_string(std::string & uuid_string)
{
	if (uuid_string.length() == 36)
	{
		if (uuid_string[8] == '-'
			&& uuid_string[13] == '-'
			&&uuid_string[18] == '-'
			&&uuid_string[23] == '-')
		{
			return true;
		}
	}
	return false;
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