#pragma once

#include <cstdint>
#include <string.h>

namespace ja_iot {
namespace stack {
class CborEncoderBuffer
{
  private:
    uint8_t *  _buffer = nullptr;
    uint32_t   _capacity = 0;
    uint32_t   _offset = 0;
    bool       _is_owned = true;

  public:
    CborEncoderBuffer(){}
    CborEncoderBuffer( uint32_t capacity )
    {
      this->_capacity = capacity;
      this->_buffer   = new uint8_t[capacity];
    }
    CborEncoderBuffer( uint8_t *buffer, uint32_t capacity )
    {
      this->_is_owned = false;
      this->_capacity = capacity;
      this->_buffer   = buffer;
    }
    virtual ~CborEncoderBuffer ()
    {
      if( this->_is_owned == true )
      {
        delete _buffer;
      }
    }

    uint8_t* data() { return ( _buffer ); }
    uint32_t size() { return ( _offset ); }

    bool put_byte( uint8_t value )
    {
      if( _offset < _capacity )
      {
        _buffer[_offset++] = value;
        return ( true );
      }
      else
      {
        return ( false );
      }
    }

    bool put_bytes( const uint8_t *data, int size )
    {
      if( _offset + size - 1 < _capacity )
      {
        memcpy( _buffer + _offset, data, size );
        _offset += size;
        return ( true );
      }
      else
      {
        return ( false );
      }
    }
};
}
}
