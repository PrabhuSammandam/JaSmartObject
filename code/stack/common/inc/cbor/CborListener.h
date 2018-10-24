#pragma once
#include <cstdint>
#include <string>

class CborListener
{
  public:
    virtual ~CborListener () {}

    virtual void on_uint8( uint8_t u8_value )    = 0;
    virtual void on_uint16( uint16_t u16_value ) = 0;
    virtual void on_uint32( uint32_t u32_value ) = 0;
    virtual void on_uint64( uint64_t u64_value ) = 0;

    virtual void on_int8( int8_t i8_value )    = 0;
    virtual void on_int16( int16_t i16_value ) = 0;
    virtual void on_int32( int32_t i32_value ) = 0;
    virtual void on_int64( int64_t i64_value ) = 0;

    virtual void on_float16( float i16_value )  = 0;
    virtual void on_float32( float i32_value )  = 0;
    virtual void on_float64( double i64_value ) = 0;

    virtual void on_indefinite_byte_string()                           = 0;
    virtual void on_definite_byte_string( uint8_t *data, size_t size ) = 0;
    virtual void on_indefinite_text_string()                           = 0;
    virtual void on_definite_text_string( uint8_t *data, size_t size ) = 0;
    virtual void on_indefinite_array()                                 = 0;
    virtual void on_definite_array( uint32_t size )                    = 0;
    virtual void on_indefinite_map()                                   = 0;
    virtual void on_definite_map( uint32_t size )                      = 0;
    virtual void on_tag( uint64_t tag )                                = 0;
    virtual void on_bool( bool )                                       = 0;
    virtual void on_null()                                             = 0;
    virtual void on_undefined()                                        = 0;
    virtual void on_indefinite_break()                                 = 0;
    virtual void on_error( const char *error )                         = 0;
};