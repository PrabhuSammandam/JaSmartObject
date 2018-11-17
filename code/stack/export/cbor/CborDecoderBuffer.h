#pragma once
#include <string.h>
#include <cstdint>

namespace ja_iot {
namespace stack {
class CborDecoderBuffer
{
  private:
    unsigned char * _data;
    int             _size;
    int             _offset;

  public:
    CborDecoderBuffer( void *data, int size )
    {
      _data   = (unsigned char *) data;
      _size   = size;
      _offset = 0;
    }
    ~CborDecoderBuffer ()
    {
    }

    bool    has_bytes( int count ) { return ( _size - _offset >= count ); }
    uint8_t peek_byte() { return ( _data[_offset] ); }

    uint8_t get_major_type()
    {
      return ( _data[_offset] >> 5 );
    }

    template<typename T>
    T get()
    {
      uint8_t length = sizeof( T );
      T       value  = 0;
      int     j      = 0;

      for( int i = length - 1; i >= 0; i--, j++ )
      {
        T temp_value = static_cast<T>( _data[_offset + j] ) << ( i * 8 );
        value |= temp_value;
      }

      _offset += length;
      return ( value );
    }

    uint32_t get_value( uint8_t length )
    {
      uint32_t value = 0;
      int      j     = 0;

      for( int i = length - 1; i >= 0; i--, j++ )
      {
        uint32_t temp_value = static_cast<uint32_t>( _data[_offset + j] ) << ( i * 8 );
        value |= temp_value;
      }

      _offset += length;
      return ( value );
    }

    void get_bytes( void *to, int count )
    {
      memcpy( to, _data + _offset, count );
      _offset += count;
    }
};
}
}