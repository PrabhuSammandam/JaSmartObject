#pragma once

#ifndef CBOR_ENCODER
#define CBOR_ENCODER

#include "CborEncoderBuffer.h"
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
    CborEncoderBuffer * _dst_buffer;

  public:
    CborEncoder( CborEncoderBuffer *dst_buffer) : _dst_buffer{ dst_buffer } {}
    ~CborEncoder () {}

		void write_byte_string(const uint8_t *data, uint16_t size)
		{
			write_type_value(2, size);
			_dst_buffer->put_bytes(data, size);
		}
		void write_string(const char *data, uint16_t size)
		{
			write_type_value(3, size);
			_dst_buffer->put_bytes((const uint8_t *)data, size);
		}
		void write_string(std::string &str)
		{
			write_type_value(3, (unsigned int)str.size());
			_dst_buffer->put_bytes((const uint8_t *)str.c_str(), (int)str.size());
		}

    void write_array( uint16_t size ) { write_type_value( 4, size ); }
    void write_map( uint16_t size ) { write_type_value( 5, size ); }
    void write_tag( const unsigned int tag ) { write_type_value( 6, tag ); }
    void write_special( int special ) { write_type_value( 7, special ); }
    void write_null() { _dst_buffer->put_byte( 0xf6 ); }
    void write_undefined() { _dst_buffer->put_byte( 0xf7 ); }

    template<typename T,
    typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
    void write( T value )
    {
      bool is_neg = std::is_signed<T>::value && value < 0;

      write_type_value( ( is_neg ) ? 1 : 0, static_cast<uint64_t>( ( is_neg ) ? ( -1 - value ) : ( value ) ) );
    }

    void write( bool value )
    {
      _dst_buffer->put_byte( ( value ) ? 0xf5 : 0xf4 );
    }

		void write(float value)
		{
			union _cbor_float_helper {
				float    as_float;
				uint32_t as_uint;
			};
			union _cbor_float_helper helper;
			helper.as_float = value;
			write_type_value(7, helper.as_uint);
		}

		void write(double value)
		{
			if (double(float(value)) == value)
			{
				union
				{
					float      f;
					uint32_t   i;
				};

				f = (float)value;
				_dst_buffer->put_byte((uint8_t)(224+ 26));
				put_value<uint32_t>(static_cast<uint32_t>(i));
			}
			else
			{
				union
				{
					double     f;
					uint64_t   i;
				};

				f = value;
				_dst_buffer->put_byte((uint8_t)(224 + 27));
				put_value<uint64_t>(i);
			}
		}

		void write(std::string& value)
		{
			write_string(value);
		}

    template<typename T,
    typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
    void put_value( T value )
    {
      int i = sizeof( T ) - 1;

      for(; i >= 0; i-- )
      {
        _dst_buffer->put_byte( static_cast<uint8_t>( ( value >> ( i * 8 ) ) ) );
      }
    }

  private:
		void write_type_value(uint8_t major_type, uint64_t value)
		{

			major_type <<= 5;

			if (value < 24ULL)
			{
				_dst_buffer->put_byte((uint8_t)(major_type | value));
			}
			else if (value < 256ULL)
			{
				_dst_buffer->put_byte((uint8_t)(major_type | 24));
				put_value<uint8_t>(static_cast<uint8_t>(value));
			}
			else if (value < 65536ULL)
			{
				_dst_buffer->put_byte((uint8_t)(major_type | 25));
				put_value<uint16_t>(static_cast<uint16_t>(value));
			}
			else if (value < 4294967296ULL)
			{
				_dst_buffer->put_byte((uint8_t)(major_type | 26));
				put_value<uint32_t>(static_cast<uint32_t>(value));
			}
			else
			{
				_dst_buffer->put_byte((uint8_t)(major_type | 27));
				put_value<uint64_t>(value);
			}
		}
};
}
}

#endif