#pragma once

#ifndef CBOR_ENCODER
#define CBOR_ENCODER

#include <vector>
#include <cbor/CborEncoderBuffer.h>
#include <string>
#include <type_traits>

namespace ja_iot {
namespace stack {
enum class CborType
{
  POSITIVE_INTEGER = 0,
  NEGATIVE_INTEGER = 1,
  BYTE_STRING      = 2,
  TEXT_STRING      = 3,
  ARRAY            = 4,
  MAP              = 5,
  TAG              = 6,
  SPECIAL          = 7
};

class CborEncoder
{
  private:
    CborEncoderBuffer _dst_buffer{};

  public:
    CborEncoder( uint16_t buff_size )
    {
      auto buffer = new uint8_t[buff_size];

      new(&_dst_buffer) CborEncoderBuffer{ buffer, buff_size };
    }
    CborEncoder( CborEncoderBuffer *dst_buffer ) : _dst_buffer{ *dst_buffer } {}
    ~CborEncoder () {}

    uint8_t* get_buf() { return ( _dst_buffer.data() ); }
    uint16_t get_buf_len() { return ( _dst_buffer.size() ); }

    void write_byte_string( const uint8_t *pu8_data, uint16_t u16_size )
    {
      write_type_value( (uint8_t) CborType::BYTE_STRING, u16_size );
      _dst_buffer.put_bytes( pu8_data, u16_size );
    }
    void write_string( const char *data, uint16_t size )
    {
      write_type_value( (uint8_t) CborType::TEXT_STRING, size );
      _dst_buffer.put_bytes( (const uint8_t *) data, size );
    }
    void write_string( const std::string &str )
    {
      write_type_value( (uint8_t) CborType::TEXT_STRING, (unsigned int) str.size() );
      _dst_buffer.put_bytes( (const uint8_t *) str.c_str(), (int) str.size() );
    }
    void write_string( const char *str )
    {
      if( str != nullptr )
      {
        write_type_value( (uint8_t) CborType::TEXT_STRING, ::strlen( str ) );
        _dst_buffer.put_bytes( (const uint8_t *) str, ::strlen( str ) );
      }
    }

    void write_array( uint16_t size ) { write_type_value( (uint8_t) CborType::ARRAY, size ); }
    void write_map( uint16_t size ) { write_type_value( (uint8_t) CborType::MAP, size ); }
    void write_tag( const unsigned int tag ) { write_type_value( (uint8_t) CborType::TAG, tag ); }
    void write_special( int special ) { write_type_value( (uint8_t) CborType::SPECIAL, special ); }
    void write_null() { _dst_buffer.put_byte( 0xf6 ); }
    void write_undefined() { _dst_buffer.put_byte( 0xf7 ); }
    void start_map() { _dst_buffer.put_byte( 0xBF ); }
    void end_map() { _dst_buffer.put_byte( 0xFF ); }
    void start_array() { _dst_buffer.put_byte( 0x9F ); }
    void end_array() { _dst_buffer.put_byte( 0xFF ); }

    /**
     * This api used for int, long values to encode. Even though bool is integral type in c++,
     * in CBOR bool values are encoded as 0xF4 or 0xF5. Inside the template function it is not
     * known how to find whether it is bool or not.
     *
     * @param value
     */
    template<typename T,
    typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
    void write( T value )
    {
      bool is_neg = std::is_signed<T>::value && value < 0;

      write_type_value( ( is_neg ) ? 1 : 0, static_cast<uint64_t>( ( is_neg ) ? ( -1 - value ) : ( value ) ) );
    }

    void write( bool value )
    {
      _dst_buffer.put_byte( ( value ) ? 0xf5 : 0xf4 );
    }

    void write( float value )
    {
      union _cbor_float_helper
      {
        float    as_float;
        uint32_t as_uint;
      };

      union _cbor_float_helper helper;

      helper.as_float = value;
      write_type_value( 7, helper.as_uint );
    }

    void write( double value )
    {
      if( double(float(value) ) == value )
      {
        union
        {
          float      f;
          uint32_t   i;
        };

        f = (float) value;
        _dst_buffer.put_byte( (uint8_t) ( 224 + 26 ) );
        put_value<uint32_t>( static_cast<uint32_t>( i ) );
      }
      else
      {
        union
        {
          double     f;
          uint64_t   i;
        };

        f = value;
        _dst_buffer.put_byte( (uint8_t) ( 224 + 27 ) );
        put_value<uint64_t>( i );
      }
    }

    void write( const std::string &rstr_value )
    {
      write_string( rstr_value );
    }

    template<typename T,
    typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
    void put_value( T value )
    {
      int i = sizeof( T ) - 1;

      /* values are encoded in natural order, ie network order */
      for(; i >= 0; i-- )
      {
        _dst_buffer.put_byte( static_cast<uint8_t>( ( value >> ( i * 8 ) ) ) );
      }
    }

    template<typename T,
    typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
    bool write_map_entry( const std::string &key, T value )
    {
      write( key );
      write( value );

      return ( true );
    }

    template<typename T,
    typename std::enable_if<std::is_class<T>::value, int>::type = 0>
    bool write_map_entry( const std::string &key, const T &value )
    {
      write( key );
      write( value );

      return ( true );
    }

    bool write_map_entry( const std::string &key, const std::string &value )
    {
      if( !key.empty() )
      {
        write( key );
        write( value );
      }

      return ( true );
    }

    template<typename T>
    bool write_map_entry( const std::string &key, const std::vector<T> &value, bool write_empty_array = false )
    {
      if( value.empty() && !write_empty_array )
      {
        return ( true );
      }

      write( key );
      write_array( value );

      return ( true );
    }

    template<typename T>
    bool write_array( const std::vector<T> &array_values )
    {
      int array_length = array_values.size();

      if( array_length == 0 )
      {
        write_null();
      }
      else
      {
        write_array( array_length );

        for( auto i = 0; i < array_length; i++ )
        {
          write( array_values[i] );
        }
      }

      return ( true );
    }

  private:
    void write_type_value( uint8_t u8_major_type, uint64_t u64_value )
    {
      u8_major_type <<= 5;

      if( u64_value < 24ULL )
      {
        _dst_buffer.put_byte( (uint8_t) ( u8_major_type | u64_value ) );
      }
      else if( u64_value < 256ULL )
      {
        _dst_buffer.put_byte( (uint8_t) ( u8_major_type | 24 ) );
        put_value<uint8_t>( static_cast<uint8_t>( u64_value ) );
      }
      else if( u64_value < 65536ULL )
      {
        _dst_buffer.put_byte( (uint8_t) ( u8_major_type | 25 ) );
        put_value<uint16_t>( static_cast<uint16_t>( u64_value ) );
      }
      else if( u64_value < 4294967296ULL )
      {
        _dst_buffer.put_byte( (uint8_t) ( u8_major_type | 26 ) );
        put_value<uint32_t>( static_cast<uint32_t>( u64_value ) );
      }
      else
      {
        _dst_buffer.put_byte( (uint8_t) ( u8_major_type | 27 ) );
        put_value<uint64_t>( u64_value );
      }
    }
};
}
}

#endif
