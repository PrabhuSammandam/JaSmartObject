#include "common/inc/cbor/CborTypedefs.h"
#include "common/inc/cbor/CborEncoding.h"

size_t _cbor_encode_uint8( uint8_t value, unsigned char *buffer, size_t buffer_size, uint8_t offset )
{
  if( value <= 23 )
  {
    if( buffer_size >= 1 )
    {
      buffer[0] = value + offset;
      return ( 1 );
    }
  }
  else
  {
    if( buffer_size >= 2 )
    {
      buffer[0] = 0x18 + offset;
      buffer[1] = value;
      return ( 2 );
    }
  }

  return ( 0 );
}

size_t _cbor_encode_uint16( uint16_t value, unsigned char *buffer, size_t buffer_size, uint8_t offset )
{
  if( buffer_size >= 3 )
  {
    buffer[0] = 0x19 + offset;
    buffer[1] = static_cast<uint8_t>( value >> 8 );
    buffer[2] = static_cast<uint8_t>( value );

    return ( 3 );
  }
  else
  {
    return ( 0 );
  }
}

size_t _cbor_encode_uint32( uint32_t value, unsigned char *buffer, size_t buffer_size, uint8_t offset )
{
  if( buffer_size >= 5 )
  {
    buffer[0] = 0x1A + offset;
    buffer[1] = value >> 24;
    buffer[2] = value >> 16;
    buffer[3] = value >> 8;
    buffer[4] = value;

    return ( 5 );
  }
  else
  {
    return ( 0 );
  }
}

size_t _cbor_encode_uint64( uint64_t value, unsigned char *buffer, size_t buffer_size, uint8_t offset )
{
  if( buffer_size >= 9 )
  {
    buffer[0] = 0x1B + offset;
    buffer[1] = static_cast<uint8_t>( value >> 56 );
    buffer[2] = static_cast<uint8_t>( value >> 48 );
    buffer[3] = static_cast<uint8_t>( value >> 40 );
    buffer[4] = static_cast<uint8_t>( value >> 32 );
    buffer[5] = static_cast<uint8_t>( value >> 24 );
    buffer[6] = static_cast<uint8_t>( value >> 16 );
    buffer[7] = static_cast<uint8_t>( value >> 8 );
    buffer[8] = static_cast<uint8_t>( value );

    return ( 9 );
  }
  else
  {
    return ( 0 );
  }
}

size_t _cbor_encode_uint( uint64_t value, unsigned char *buffer, size_t buffer_size, uint8_t offset )
{
  if( value <= UINT16_MAX )
  {
    if( value <= UINT8_MAX )
    {
      return ( _cbor_encode_uint8( static_cast<uint8_t>( value ), buffer, buffer_size, offset ) );
    }
    else
    {
      return ( _cbor_encode_uint16( static_cast<uint16_t>( value ), buffer, buffer_size, offset ) );
    }
  }
  else if( value <= UINT32_MAX )
  {
    return ( _cbor_encode_uint32( static_cast<uint32_t>( value ), buffer, buffer_size, offset ) );
  }
  else
  {
    return ( _cbor_encode_uint64( value, buffer, buffer_size, offset ) );
  }
}

size_t cbor_encode_uint8( uint8_t value, unsigned char *buffer, size_t buffer_size )
{
  return ( _cbor_encode_uint8( value, buffer, buffer_size, 0x00 ) );
}

size_t cbor_encode_uint16( uint16_t value, unsigned char *buffer, size_t buffer_size )
{
  return ( _cbor_encode_uint16( value, buffer, buffer_size, 0x00 ) );
}

size_t cbor_encode_uint32( uint32_t value, unsigned char *buffer, size_t buffer_size )
{
  return ( _cbor_encode_uint32( value, buffer, buffer_size, 0x00 ) );
}

size_t cbor_encode_uint64( uint64_t value, unsigned char *buffer, size_t buffer_size )
{
  return ( _cbor_encode_uint64( value, buffer, buffer_size, 0x00 ) );
}

size_t cbor_encode_uint( uint64_t value, unsigned char *buffer, size_t buffer_size )
{
  return ( _cbor_encode_uint( value, buffer, buffer_size, 0x00 ) );
}


size_t cbor_encode_negint8( uint8_t value, unsigned char *buffer, size_t buffer_size )
{
  return ( _cbor_encode_uint8( value, buffer, buffer_size, 0x20 ) );
}

size_t cbor_encode_negint16( uint16_t value, unsigned char *buffer, size_t buffer_size )
{
  return ( _cbor_encode_uint16( value, buffer, buffer_size, 0x20 ) );
}

size_t cbor_encode_negint32( uint32_t value, unsigned char *buffer, size_t buffer_size )
{
  return ( _cbor_encode_uint32( value, buffer, buffer_size, 0x20 ) );
}

size_t cbor_encode_negint64( uint64_t value, unsigned char *buffer, size_t buffer_size )
{
  return ( _cbor_encode_uint64( value, buffer, buffer_size, 0x20 ) );
}

size_t cbor_encode_negint( uint64_t value, unsigned char *buffer, size_t buffer_size )
{
  return ( _cbor_encode_uint( value, buffer, buffer_size, 0x20 ) );
}

size_t cbor_encode_bytestring_start( size_t length, unsigned char *buffer, size_t buffer_size )
{
  return ( _cbor_encode_uint( (size_t) length, buffer, buffer_size, 0x40 ) );
}

size_t _cbor_encode_byte( uint8_t value, unsigned char *buffer, size_t buffer_size )
{
  if( buffer_size >= 1 )
  {
    buffer[0] = value;
    return ( 1 );
  }
  else
  {
    return ( 0 );
  }
}

size_t cbor_encode_indef_bytestring_start( unsigned char *buffer, size_t buffer_size )
{
  return ( _cbor_encode_byte( 0x5F, buffer, buffer_size ) );
}

size_t cbor_encode_string_start( size_t length, unsigned char *buffer, size_t buffer_size )
{
  return ( _cbor_encode_uint( (size_t) length, buffer, buffer_size, 0x60 ) );
}

size_t cbor_encode_indef_string_start( unsigned char *buffer, size_t buffer_size )
{
  return ( _cbor_encode_byte( 0x7F, buffer, buffer_size ) );
}

size_t cbor_encode_array_start( size_t length, unsigned char *buffer, size_t buffer_size )
{
  return ( _cbor_encode_uint( (size_t) length, buffer, buffer_size, 0x80 ) );
}

size_t cbor_encode_indef_array_start( unsigned char *buffer, size_t buffer_size )
{
  return ( _cbor_encode_byte( 0x9F, buffer, buffer_size ) );
}

size_t cbor_encode_map_start( size_t length, unsigned char *buffer, size_t buffer_size )
{
  return ( _cbor_encode_uint( (size_t) length, buffer, buffer_size, 0xA0 ) );
}

size_t cbor_encode_indef_map_start( unsigned char *buffer, size_t buffer_size )
{
  return ( _cbor_encode_byte( 0xBF, buffer, buffer_size ) );
}

size_t cbor_encode_tag( uint64_t value, unsigned char *buffer, size_t buffer_size )
{
  return ( _cbor_encode_uint( value, buffer, buffer_size, 0xC0 ) );
}

size_t cbor_encode_bool( bool value, unsigned char *buffer, size_t buffer_size )
{
  return ( value ? _cbor_encode_byte( 0xF5, buffer, buffer_size ) : _cbor_encode_byte( 0xF4, buffer, buffer_size ) );
}

size_t cbor_encode_null( unsigned char *buffer, size_t buffer_size )
{
  return ( _cbor_encode_byte( 0xF6, buffer, buffer_size ) );
}

size_t cbor_encode_undef( unsigned char *buffer, size_t buffer_size )
{
  return ( _cbor_encode_byte( 0xF7, buffer, buffer_size ) );
}

size_t cbor_encode_half( float value, unsigned char *buffer, size_t buffer_size )
{
  /* Assuming value is normalized */
  union _cbor_float_helper float_helper;

  float_helper.as_float = value;
  uint32_t                 val = float_helper.as_uint;
  uint16_t                 res;
  uint8_t                  exp  = (uint8_t) ( ( val & 0x7F800000 ) >> 23 ); /* 0b0111_1111_1000_0000_0000_0000_0000_0000 */
  uint32_t                 mant = val & 0x7FFFFF; /* 0b0000_0000_0111_1111_1111_1111_1111_1111 */

  if( exp == 0xFF )        /* Infinity or NaNs */
  {
    if( value != value )
    {
      res = (uint16_t) 0x00e700;                   /* Not IEEE semantics - required by CBOR [s. 3.9] */
    }
    else
    {
      res = (uint16_t) ( ( val & 0x80000000 ) >> 16 | 0x7C00 | ( mant ? 1 : 0 ) << 15 );
    }
  }
  else if( exp == 0x00 )          /* Zeroes or subnorms */
  {
    res = (uint16_t) ( ( val & 0x80000000 ) >> 16 | mant >> 13 );
  }
  else           /* Normal numbers */
  {
    int8_t logical_exp = (int8_t) ( exp - 127 );
    // assert( logical_exp == exp - 127 );

    // Now we know that 2^exp <= 0 logically
    if( logical_exp < -24 )
    {
      /* No unambiguous representation exists, this float is not a half float and is too small to
       * be represented using a half, round off to zero. Consistent with the reference implementation. */
      res = 0;
    }
    else if( logical_exp < -14 )
    {
      /* Offset the remaining decimal places by shifting the significand, the value is lost.
       * This is an implementation decision that works around the absence of standard half-float
       * in the language. */
      res = (uint16_t) ( val & 0x80000000 ) >> 16 | (uint16_t) ( 1 << ( 24 + logical_exp ) );
    }
    else
    {
      res = (uint16_t) ( ( val & 0x80000000 ) >> 16 | ( ( ( (uint8_t) logical_exp ) + 15 ) << 10 ) | (uint16_t) ( mant >> 13 ) );
    }
  }

  return ( _cbor_encode_uint16( res, buffer, buffer_size, 0xE0 ) );
}

size_t cbor_encode_single( float value, unsigned char *buffer, size_t buffer_size )
{
  union _cbor_float_helper float_helper;

  float_helper.as_float = value;

  return ( _cbor_encode_uint32( float_helper.as_uint, buffer, buffer_size, 0xE0 ) );
}

size_t cbor_encode_double( double value, unsigned char *buffer, size_t buffer_size )
{
  union _cbor_double_helper double_helper;

  double_helper.as_double = value;

  return ( _cbor_encode_uint64( double_helper.as_uint, buffer, buffer_size, 0xE0 ) );
}

size_t cbor_encode_break( unsigned char *buffer, size_t buffer_size )
{
  return ( _cbor_encode_byte( 0xFF, buffer, buffer_size ) );
}

size_t cbor_encode_ctrl( uint8_t value, unsigned char *buffer, size_t buffer_size )
{
  return ( _cbor_encode_uint8( value, buffer, buffer_size, 0xE0 ) );
}