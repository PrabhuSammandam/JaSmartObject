#include <string>
#include <coap/coap_option.h>
#include <cstring>
#include <iostream>
#include <IMemAllocator.h>

namespace ja_iot {
namespace network {
using namespace memory;
using namespace std;
Option::Option( const uint16_t u16_no, const uint16_t u16_len, uint8_t *pu8_val ) : _is_mem_allocated{ false }, _u16_len{ 0 }
{
  _u16_no = u16_no;
  this->set_val( u16_len, pu8_val );
}
Option::Option( const uint16_t u16_no ) : _is_mem_allocated{ false }, _u16_len{ 0 }, _u16_no{ u16_no }, u{ 0 }
{
}
Option::Option( const uint16_t u16_no, const string &rcz_string_value ) : _u16_no{ u16_no }
{
  set_string_value( rcz_string_value );
}
Option::Option( const uint16_t u16_no, const uint32_t u32_value ) : _u16_no{ u16_no }
{
  set_integer_value( u32_value );
}
Option::~Option ()
{
  if( _is_mem_allocated == true )
  {
    delete[] this->u._pu8_buffer;
  }

  this->u._pu8_buffer     = nullptr;
  this->_u16_no           = 0;
  this->_is_mem_allocated = false;
  this->_u16_len          = 0;
}

void Option::set_string_value( const string &rcz_string_value )
{
  set_val( uint16_t( rcz_string_value.size() ), (uint8_t *) rcz_string_value.data() );
}

string Option::get_string_value()
{
  string ret_val{ reinterpret_cast<const char *>( get_val() ), _u16_len };

  return ( ret_val );
}

void Option::set_integer_value( const uint32_t u32_value )
{
  uint8_t u8_value_len{};
  uint8_t au8_temp_value[4] {};

  for( uint8_t i = 0; i < 4; ++i )
  {
    const uint32_t temp_max_value = ( 1 << ( i * 8 ) ) - 1;

    if( u32_value > temp_max_value )
    {
      u8_value_len++;
    }
    else
    {
      break;
    }
  }

  for( uint8_t i = 0; i < u8_value_len; ++i )
  {
    au8_temp_value[i] = u32_value >> ( u8_value_len - i - 1 ) * 8 & 0xFF;
  }

  set_val( u8_value_len, &au8_temp_value[0] );
}

uint32_t Option::get_integer_value()
{
  uint32_t   value     = 0;
  const auto pu8_value = get_val();

  for( int i = 0; i < _u16_len; i++ )
  {
    value |= static_cast<uint32_t>( pu8_value[_u16_len - i - 1] & 0xFF ) << i * 8;
  }

  return ( value );
}

uint16_t Option::get_len() const
{
  return ( this->_u16_len );
}

uint8_t * Option::get_val()
{
  if( this->_is_mem_allocated )
  {
    return ( this->u._pu8_buffer );
  }

  return ( &this->u._au8_buffer[0] );
}

void Option::set_val( const uint16_t u16_option_len, uint8_t *pu8_option_value )
{
  if( this->_is_mem_allocated == true )
  {
    delete[] this->u._pu8_buffer;
    this->u._pu8_buffer = nullptr;
  }

  if( u16_option_len == 0 )
  {
    this->_is_mem_allocated = false;
    this->_u16_len          = 0;
  }
  else
  {
    if( pu8_option_value != nullptr )
    {
      if( u16_option_len > 6 )
      {
        this->u._pu8_buffer = new uint8_t[u16_option_len];
        memcpy( this->u._pu8_buffer, pu8_option_value, u16_option_len );
        this->_is_mem_allocated = true;
      }
      else
      {
        memcpy( &this->u._au8_buffer[0], pu8_option_value, u16_option_len );
        this->_is_mem_allocated = false;
      }

      /* set length */
      this->_u16_len = u16_option_len;
    }
  }
}
Option::Option( const Option &other )
{
  *this = other;
}
Option::Option( Option &&other ) noexcept
{
  *this = move( other );
}

Option & Option::operator = ( const Option &other )
{
  if( this != &other )
  {
    this->_u16_no = other._u16_no;
    set_val( other._u16_len, const_cast<Option &>( other ).get_val() );
  }

  return ( *this );
}

Option & Option::operator = ( Option &&other ) noexcept
{
  if( this != &other )
  {
    this->_u16_no           = other._u16_no;
    this->_u16_len          = other._u16_len;
    this->_is_mem_allocated = other._is_mem_allocated;

    if( other._is_mem_allocated == true )
    {
      this->u._pu8_buffer = other.u._pu8_buffer;
    }
    else
    {
      memcpy( &this->u._au8_buffer, &other.u._au8_buffer[0], other._u16_len );
    }

    other.u._pu8_buffer     = nullptr;
    other._is_mem_allocated = false;
    other._u16_len          = 0;
    other._u16_no           = 0;
  }

  return ( *this );
}

void Option::print()
{
  cout << "[ No : " << _u16_no << " , Len : " << _u16_len << " , Val : " << get_string_value() << " ]" << endl;
}
}
}
