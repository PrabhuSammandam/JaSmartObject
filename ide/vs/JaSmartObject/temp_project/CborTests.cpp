#if 0
#include <stdlib.h>
#include <memory.h>
#include "cbor/CborCodec.h"
#include "cbor/CborEncoderBuffer.h"
#include "cbor/CborEncoder.h"
#include "cbor/CborDecoder.h"
#include "cbor/CborDecoderBuffer.h"
#include "ResPropValue.h"
#include "cbor/CborItem.h"
#include "cbor/CborPrettyPrinter.h"
#include "cbor/CborItemBuilder.h"
#include "ErrCode.h"

using namespace ja_iot::stack;
using namespace ja_iot::base;

char*          bin2hex( unsigned char *p, int len );
unsigned char* hex2bin( const char *str, uint16_t &hex_buffer_len );
void           get_data_from_file( const char *file_name, uint8_t * &file_data, int32_t &data_length );

void test_cbor_decode();
void test1();
void test2();

void run_cbor_test_suite()
{
  ResRepresentation core_rep{};

  core_rep.add<std::string>( "if", "oic.d.light" );
  core_rep.add( "temp", (long) -100 );

  ResRepresentation a{};
  a.add( "a", (long) 1 );
  ResRepresentation b{};
  b.add( "b", (long) 2 );
  ResRepresentation c{};
  c.add( "b", (long) 3 );

  std::vector<ResRepresentation> obj_array{};
  obj_array.push_back( std::move( a ) );
  obj_array.push_back( std::move( b ) );
  obj_array.push_back( std::move( c ) );

  std::vector<bool> bool_values{ true, false, true };
  std::vector<long> long_values{ 0, 1, 2, 3 };
  std::vector<double> double_values{ 0.0, 1.0, 2.0, 3.0 };

  ResRepresentation light_res{};

  light_res.add( "on", true );
  light_res.add( "lum", (long) 100 );
  light_res.add( "name", std::string{ "bedroom" } );
  light_res.add( "temp", -100.00 );
  light_res.add( "bool_values", std::move( bool_values ) );
  light_res.add( "long_values", std::move( long_values ) );
  light_res.add( "double_values", std::move( double_values ) );
  light_res.add<ResRepresentation>( "core", std::move( core_rep ) );
  light_res.add( "objs", std::move( obj_array ) );

	light_res.print();

  //uint8_t *         buffer;
  //uint16_t          buffer_length;
  //CborCodec::encode( light_res, buffer, buffer_length );

  //ResRepresentation d;
  //CborCodec::decode( buffer, buffer_length, d );
}

void test_cbor_decode()
{
  auto     file_name = "D:\\Sources\\cbor\\libcbor\\examples\\data\\map.cbor";
  uint8_t *buffer;
  int32_t  buffer_length;

  get_data_from_file( file_name, buffer, buffer_length );

  // CborDecoderListenerDebug decoder_listener_debug{};
  CborDecoderBuffer decoder_buffer{ buffer, (int) buffer_length };
  // CborDecoder decoder{ &decoder_buffer, &decoder_listener_debug };

  // decoder.decode();
}

void test2()
{
  auto encoder_buffer = new CborEncoderBuffer{ 1024 };
  CborEncoder encoder{ encoder_buffer };

  // encoder.write<uint8_t>(0);
  // encoder.write<uint8_t>(1);
  // encoder.write<uint8_t>(10);
  // encoder.write<uint8_t>(23);
  // encoder.write<uint8_t>(24);
  // encoder.write<uint8_t>(25);
  // encoder.write<uint8_t>(100);
  // encoder.write(1000);// 1903e8
  // encoder.write(1000000);// 1a000f4240
  // encoder.write(1000000000000);// 1b000000e8d4a51000
  // encoder.write(18446744073709551615);// 1bffffffffffffffff
  // encoder.write(-1);//20
  // encoder.write(-10);//29
  // encoder.write(-100);//3863
  // encoder.write(-1000);//3903e7
  // encoder.write(true);
  // encoder.write(false);
  // encoder.write_special(16);
  // encoder.write_special(24);
  // encoder.write_special(255);
  // encoder.write_byte_string((const uint8_t*)"", 0);
  // encoder.write(std::string{"IETF"});
  // encoder.write_array(0);

  // encoder.write_array(3);
  // encoder.write(1);
  // encoder.write(2);
  // encoder.write(3);


  // encoder.write_array(3);
  // encoder.write(1);
  // encoder.write_array(2);
  // encoder.write(2);
  // encoder.write(3);
  // encoder.write_array(2);
  // encoder.write(4);
  // encoder.write(5);

  // encoder.write_map(0);

  // encoder.write_map(2);
  // encoder.write(1);
  // encoder.write(2);
  // encoder.write(3);
  // encoder.write(4);

  // encoder.write_map(2);
  // encoder.write(std::string{"a"});
  // encoder.write(1);
  // encoder.write(std::string{ "b" });
  // encoder.write_array(2);
  // encoder.write(2);
  // encoder.write(3);

  // encoder.write_array(2);
  // encoder.write(std::string{ "a" });
  // encoder.write_map(1);
  // encoder.write(std::string{ "b" });
  // encoder.write(std::string{ "c" });

  encoder.write( 3.4028234663852886e+38 );

  auto out_array = bin2hex( encoder_buffer->data(), encoder_buffer->size() );
  printf( "%s", out_array );
}

void test1()
{
  // auto input = "00"; // 0
  // auto input = "01"; // 1
  // auto input = "0a"; // 10
  // auto input = "17"; // 23
  // auto input = "1818"; // 24
  // auto input = "1819"; //25
  // auto input = "1864"; // 100
  // auto input = "1903e8";// 1000
  // auto input = "1a000f4240";// 1000000
  // auto input = "1b000000e8d4a51000";// 1000000000000
  // auto input = "1bffffffffffffffff"; // 18446744073709551615
  // auto input = "c249010000000000000000"; // 18446744073709551616
  // auto input = "20";// -1
  // auto input = "29";// -10
  // auto input = "3863";// -100
  // auto input = "3903e7";// -1000
  // auto input = "f90000";// 0.0
  // auto input = "f98000";// -0.0
  // auto input = "f93c00";// 1.0
  // auto input = "fb3ff199999999999a";// 1.1
  // auto input = "f93e00";// 1.5
  // auto input = "f97bff";// 65504.0
  // auto input = "fa47c35000";// 100000.0
  // auto input = "fa7f7fffff";// 3.4028234663852886e+38
  // auto input = "fb7e37e43c8800759c";// 1.0e+300
  // auto input = "f90001";// 5.960464477539063e-08
  // auto input = "f90400";// 6.103515625e-05
  // auto input = "f9c400";// -4.0
  // auto input = "fbc010666666666666";// -4.1
  auto input = "f97c00";      // Infinity
  // auto input = "f4";// false
  // auto input = "f5";// true
  // auto input = "f6";// null
  // auto input = "f7";// undefined
  // auto input = "f0";// 16
  // auto input = "f818";// 24
  // auto input = "f8ff";// 255
  // auto input = "40";
  // auto input = "4401020304";
  // auto input = "60";
  // auto input = "6161"; // 'a'
  // auto input = "6449455446"; // 'IETF'
  // auto input = "62225c";
  // auto input = "62c3bc";
  // auto input = "80";
  // auto input = "83010203";
  // auto input = "8301820203820405";
  // auto input = "98190102030405060708090a0b0c0d0e0f101112131415161718181819";
  // auto input = "a0";
  // auto input = "a201020304";
  // auto input = "a26161016162820203";
  // auto input = "826161a161626163";
  // auto input = "a56161614161626142616361436164614461656145";
  // auto input = "5f42010243030405ff";

  uint16_t buffer_length;
  uint8_t *buffer;

  buffer = hex2bin( input, buffer_length );
  // CborDecoderListenerDebug decoder_listener_debug{};
  // CborDecoderBuffer decoder_buffer{ buffer, (int) buffer_length };
  // CborDecoder decoder{ &decoder_buffer, &decoder_listener_debug };
  // decoder.decode();
}

void get_data_from_file( const char *file_name, uint8_t * &file_data, int32_t &data_length )
{
  file_data   = nullptr;
  data_length = 0;
  FILE *f = fopen( file_name, "rb" );

  if( f == NULL )
  {
    return;
  }

  fseek( f, 0, SEEK_END );
  data_length = (size_t) ftell( f );
  fseek( f, 0, SEEK_SET );
  file_data = new uint8_t[data_length];
  fread( file_data, data_length, 1, f );
}

char* bin2hex( unsigned char *p, int len )
{
  char *hex = new char[( ( ( 2 * len ) + 1 ) )];
  char *r   = hex;

  while( len && p )
  {
    ( *r ) = ( ( *p ) & 0xF0 ) >> 4;
    ( *r ) = ( ( *r ) <= 9 ? '0' + ( *r ) : 'A' - 10 + ( *r ) );
    r++;
    ( *r ) = ( ( *p ) & 0x0F );
    ( *r ) = ( ( *r ) <= 9 ? '0' + ( *r ) : 'A' - 10 + ( *r ) );
    r++;
    p++;
    len--;
  }

  *r = '\0';

  return ( hex );
}

unsigned char* hex2bin( const char *str, uint16_t &hex_buffer_len )
{
  int      len, h;
  uint8_t *result, *p, c;

  if( !str )
  {
    return ( nullptr );
  }

  if( !*str )
  {
    return ( nullptr );
  }

  len = 0;
  p   = (uint8_t *) str;

  while( *p++ )
  {
    len++;
  }

  hex_buffer_len = ( ( len / 2 ) );
  result         = new uint8_t[hex_buffer_len + 1];
  h              = !( len % 2 ) * 4;
  p              = result;
  *p             = 0;

  c = *str;

  while( c )
  {
    if( ( '0' <= c ) && ( c <= '9' ) )
    {
      *p += ( c - '0' ) << h;
    }
    else if( ( 'A' <= c ) && ( c <= 'F' ) )
    {
      *p += ( c - 'A' + 10 ) << h;
    }
    else if( ( 'a' <= c ) && ( c <= 'f' ) )
    {
      *p += ( c - 'a' + 10 ) << h;
    }
    else
    {
      return ( nullptr );
    }

    str++;
    c = *str;

    if( h )
    {
      h = 0;
    }
    else
    {
      h = 4;
      p++;
      *p = 0;
    }
  }

  return ( result );
}
#endif