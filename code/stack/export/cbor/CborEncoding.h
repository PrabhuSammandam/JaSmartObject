#pragma once
#include <cstdint>

template<typename T>
size_t encode_pos( T value, unsigned char *buffer, size_t buffer_size )
{
  switch( sizeof( T ) )
  {
    case 1: return ( _cbor_encode_uint8( value, buffer, buffer_size, 0x00 ) );
    case 2: return ( _cbor_encode_uint16( value, buffer, buffer_size, 0x00 ) );
    case 4: return ( _cbor_encode_uint32( value, buffer, buffer_size, 0x00 ) );
    case 8: return ( _cbor_encode_uint64( value, buffer, buffer_size, 0x00 ) );
  }
}

template<typename T>
size_t encode_neg(T value, unsigned char *buffer, size_t buffer_size)
{
	switch (sizeof(T))
	{
	case 1: return (_cbor_encode_uint8(value, buffer, buffer_size, 0x20));
	case 2: return (_cbor_encode_uint16(value, buffer, buffer_size, 0x20));
	case 4: return (_cbor_encode_uint32(value, buffer, buffer_size, 0x20));
	case 8: return (_cbor_encode_uint64(value, buffer, buffer_size, 0x20));
	}
}

size_t cbor_encode_uint8( uint8_t, unsigned char *, size_t );
size_t cbor_encode_uint16( uint16_t, unsigned char *, size_t );
size_t cbor_encode_uint32( uint32_t, unsigned char *, size_t );
size_t cbor_encode_uint64( uint64_t, unsigned char *, size_t );
size_t cbor_encode_uint( uint64_t, unsigned char *, size_t );
size_t cbor_encode_negint8( uint8_t, unsigned char *, size_t );
size_t cbor_encode_negint16( uint16_t, unsigned char *, size_t );
size_t cbor_encode_negint32( uint32_t, unsigned char *, size_t );
size_t cbor_encode_negint64( uint64_t, unsigned char *, size_t );
size_t cbor_encode_negint( uint64_t, unsigned char *, size_t );
size_t cbor_encode_bytestring_start( size_t, unsigned char *, size_t );
size_t cbor_encode_indef_bytestring_start( unsigned char *, size_t );
size_t cbor_encode_string_start( size_t, unsigned char *, size_t );
size_t cbor_encode_indef_string_start( unsigned char *, size_t );
size_t cbor_encode_array_start( size_t, unsigned char *, size_t );
size_t cbor_encode_indef_array_start( unsigned char *, size_t );
size_t cbor_encode_map_start( size_t, unsigned char *, size_t );
size_t cbor_encode_indef_map_start( unsigned char *, size_t );
size_t cbor_encode_tag( uint64_t, unsigned char *, size_t );
size_t cbor_encode_bool( bool, unsigned char *, size_t );
size_t cbor_encode_null( unsigned char *, size_t );
size_t cbor_encode_undef( unsigned char *, size_t );

/** Encodes a half-precision float
 *
 * Since there is no native representation or semantics for half floats
 * in the language, we use single-precision floats, as every value that
 * can be expressed as a half-float can also be expressed as a float.
 *
 * This however means that not all floats passed to this function can be
 * unambiguously encoded. The behavior is as follows:
 *  - Infinity, NaN are preserved
 *  - Zero is preserved
 *  - Denormalized numbers keep their sign bit and 10 most significant bit of the significand
 *  - All other numbers
 *   - If the logical value of the exponent is < -24, the output is zero
 *   - If the logical value of the exponent is between -23 and -14, the output
 *     is cut off to represent the 'magnitude' of the input, by which we
 *     mean (-1)^{signbit} x 1.0e{exponent}. The value in the significand is lost.
 *   - In all other cases, the sign bit, the exponent, and 10 most significant bits
 *     of the significand are kept
 *
 * @param value
 * @param buffer Target buffer
 * @param buffer_size Available space in the buffer
 * @return number of bytes written
 */
size_t cbor_encode_half( float, unsigned char *, size_t );
size_t cbor_encode_single( float, unsigned char *, size_t );
size_t cbor_encode_double( double, unsigned char *, size_t );
size_t cbor_encode_break( unsigned char *, size_t );
size_t cbor_encode_ctrl( uint8_t, unsigned char *, size_t );
