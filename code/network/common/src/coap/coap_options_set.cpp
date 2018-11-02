/*
 * coap_options_set.cpp
 *
 *  Created on: Nov 12, 2017
 *      Author: psammand
 */
#include <algorithm>
#include <string>
#include <set>
#include <cstdio>
#include "coap/coap_options_set.h"
#include "coap/coap_consts.h"

//extern int sprintf(char *out, const char *format, ...);

namespace ja_iot {
namespace network {
using namespace std;

void tokenize( string in_str, const char delimter, vector<string> &rcz_token_list );
CoapOptionsSet::CoapOptionsSet ()
{
}
CoapOptionsSet::~CoapOptionsSet ()
{
  clear();
}
CoapOptionsSet::CoapOptionsSet( const CoapOptionsSet &other )
{
  *this = other;
}
CoapOptionsSet::CoapOptionsSet( CoapOptionsSet &&other ) noexcept
{
  *this = move( other );
}

CoapOptionsSet & CoapOptionsSet::operator = ( const CoapOptionsSet &other )
{
  if( &other != this )
  {
    _uri_host        = other._uri_host;
    _uri_port        = other._uri_port;
    _uri_paths_list  = other._uri_paths_list;
    _uri_querys_list = other._uri_querys_list;
    _content_format  = other._content_format;
    _content_version = other._content_version;
    _accept_format   = other._accept_format;
    _accept_version  = other._accept_version;
    _cz_block_1      = other._cz_block_1;
    _cz_block_2      = other._cz_block_2;
    _u32_size_1      = other._u32_size_1;
    _u32_size_2      = other._u32_size_2;
    _options_bit     = other._options_bit;
    _u32_observe     = other._u32_observe;
  }

  return ( *this );
}

CoapOptionsSet & CoapOptionsSet::operator = ( CoapOptionsSet &&other )
{
  if( &other != this )
  {
    _uri_host        = std::move( other._uri_host );
    _uri_port        = other._uri_port;
    _uri_paths_list  = std::move( other._uri_paths_list );
    _uri_querys_list = std::move( other._uri_querys_list );
    _content_format  = other._content_format;
    _content_version = other._content_version;
    _accept_format   = other._accept_format;
    _accept_version  = other._accept_version;
    _cz_block_1      = std::move( other._cz_block_1 );
    _cz_block_2      = std::move( other._cz_block_2 );
    _u32_size_1      = other._u32_size_1;
    _u32_size_2      = other._u32_size_2;
    _options_bit     = std::move( other._options_bit );
    _u32_observe     = other._u32_observe;

    other._uri_port        = 0xFFFF;
    other._content_format  = 0xFFFF;
    other._content_version = 0xFFFF;
    other._accept_format   = 0xFFFF;
    other._accept_version  = 0xFFFF;
    other._u32_size_1      = 0;
    other._u32_size_2      = 0;
    other._u32_observe     = 0xFFFFFFFF;
  }

  return ( *this );
}

void CoapOptionsSet::clear()
{
  this->_uri_host.clear();
  this->_uri_port = 0xFFFF;
  this->_uri_paths_list.clear();
  this->_uri_querys_list.clear();
  this->_content_format  = 0xFFFF;
  this->_content_version = 0xFFFF;
  this->_accept_format   = 0xFFFF;
  this->_accept_version  = 0xFFFF;
  this->_u32_observe     = 0xFFFFFFFF;
  this->_cz_block_1.clear();
  this->_cz_block_2.clear();
  this->_options_bit.reset( 0 );
  this->_options_bit.reset( 1 );
  this->_u32_size_1 = 0;
  this->_u32_size_2 = 0;
}

string CoapOptionsSet::get_string_value( const uint16_t u16_option_len, uint8_t *pu8_option_value )
{
  string ret_val{ reinterpret_cast<const char *>( pu8_option_value ), u16_option_len };

  return ( ret_val );
}

uint32_t CoapOptionsSet::get_integer_value( const uint16_t u16_option_len, uint8_t *pu8_option_value )
{
  uint32_t value = 0;

  for( int i = 0; i < u16_option_len; i++ )
  {
    value |= static_cast<uint32_t>( pu8_option_value[u16_option_len - i - 1] & 0xFF ) << i * 8;
  }

  return ( value );
}

OptionsList CoapOptionsSet::get_sorted_options_list()
{
  OptionsList options_list{};

  if( has_uri_host() )
  {
    options_list.insert( Option{ COAP_OPTION_URI_HOST, _uri_host } );
  }

  if( has_uri_port() )
  {
    options_list.insert( Option{ COAP_OPTION_URI_PORT, _uri_port } );
  }

  if( !_uri_paths_list.empty() )
  {
    for( auto &uri_path : _uri_paths_list )
    {
      options_list.insert( Option{ COAP_OPTION_URI_PATH, uri_path } );
    }
  }

  if( !_uri_querys_list.empty() )
  {
    for( auto &uri_query : _uri_querys_list )
    {
      options_list.insert( Option{ COAP_OPTION_URI_QUERY, uri_query } );
    }
  }

  if( has_content_format() )
  {
    options_list.insert( Option{ COAP_OPTION_CONTENT_FORMAT, _content_format } );
  }

  if( has_content_version() )
  {
    options_list.insert( Option{ COAP_OPTION_CONTENT_VERSION, _content_version } );
  }

  if( has_accept_format() )
  {
    options_list.insert( Option{ COAP_OPTION_ACCEPT, _accept_format } );
  }

  if( has_accept_version() )
  {
    options_list.insert( Option{ COAP_OPTION_ACCEPT_VERSION, _accept_version } );
  }

  if( has_observe() )
  {
    options_list.insert( Option{ COAP_OPTION_OBSERVE, _u32_observe } );
  }

  if( has_block1() )
  {
    uint8_t buff[3]={0};
    uint8_t buff_len = 0;

    _cz_block_1.encode( &buff[0], buff_len );
    options_list.insert( Option{ COAP_OPTION_BLOCK_1, buff_len, &buff[0] } );
  }

  if( has_block2() )
  {
    uint8_t buff[3]  = {0};
    uint8_t buff_len = 0;

    _cz_block_2.encode( &buff[0], buff_len );
    options_list.insert( Option{ COAP_OPTION_BLOCK_2, buff_len, &buff[0] } );
  }

  if( has_size1() )
  {
    options_list.insert( Option{ COAP_OPTION_SIZE1, _u32_size_1 } );
  }

  if( has_size2() )
  {
    options_list.insert( Option{ COAP_OPTION_SIZE2, _u32_size_2 } );
  }

  return ( options_list ); // because RVO (return value optimisation) it is not copy by return
}

bool CoapOptionsSet::has_option( const uint16_t u16_option_no ) const
{
  switch( u16_option_no )
  {
    case COAP_OPTION_URI_HOST:
    {
      return ( has_uri_host() );
    }
    case COAP_OPTION_URI_PORT:
    {
      return ( has_uri_port() );
    }
    case COAP_OPTION_URI_PATH:
    {
      return ( !_uri_paths_list.empty() );
    }
    case COAP_OPTION_URI_QUERY:
    {
      return ( !_uri_querys_list.empty() );
    }
    case COAP_OPTION_CONTENT_FORMAT:
    {
      return ( has_content_format() );
    }
    case COAP_OPTION_ACCEPT:
    {
      return ( has_accept_format() );
    }
    case COAP_OPTION_CONTENT_VERSION:
    {
      return ( has_content_version() );
    }
    case COAP_OPTION_ACCEPT_VERSION:
    {
      return ( has_accept_version() );
    }
    case COAP_OPTION_OBSERVE:
    {
      return ( has_observe() );
    }
    case COAP_OPTION_BLOCK_1:
    {
      return ( has_block1() );
    }
    case COAP_OPTION_BLOCK_2:
    {
      return ( has_block2() );
    }
    case COAP_OPTION_SIZE1:
    {
      return ( has_size1() );
    }
    case COAP_OPTION_SIZE2:
    {
      return ( has_size2() );
    }
    default:
    {
    }
    break;
  }

  return ( false );
}

void CoapOptionsSet::add_option( const uint16_t u16_option_no, const uint16_t u16_option_len, uint8_t *pu8_option_value )
{
  switch( u16_option_no )
  {
    case COAP_OPTION_URI_HOST:
    {
      set_uri_host( get_string_value( u16_option_len, pu8_option_value ) );
    }
    break;

    case COAP_OPTION_URI_PORT:
    {
      set_uri_port( get_integer_value( u16_option_len, pu8_option_value ) );
    }
    break;

    case COAP_OPTION_URI_PATH:
    {
      add_uri_path( get_string_value( u16_option_len, pu8_option_value ) );
    }
    break;

    case COAP_OPTION_URI_QUERY:
    {
      add_uri_query( get_string_value( u16_option_len, pu8_option_value ) );
    }
    break;

    case COAP_OPTION_CONTENT_FORMAT:
    {
      set_content_format( get_integer_value( u16_option_len, pu8_option_value ) );
    }
    break;

    case COAP_OPTION_ACCEPT:
    {
      set_accept_format( get_integer_value( u16_option_len, pu8_option_value ) );
    }
    break;

    case COAP_OPTION_CONTENT_VERSION:
    {
      set_content_version( get_integer_value( u16_option_len, pu8_option_value ) );
    }
    break;

    case COAP_OPTION_ACCEPT_VERSION:
    {
      set_accept_version( get_integer_value( u16_option_len, pu8_option_value ) );
    }
    break;

    case COAP_OPTION_OBSERVE:
    {
      set_observe( get_integer_value( u16_option_len, pu8_option_value ) );
    }
    break;

    case COAP_OPTION_SIZE1:
    {
      set_size1( get_integer_value( u16_option_len, pu8_option_value ) );
    }
    break;

    case COAP_OPTION_SIZE2:
    {
      set_size2( get_integer_value( u16_option_len, pu8_option_value ) );
    }
    break;

    case COAP_OPTION_BLOCK_1:
    {
      BlockOption block_option{};
      block_option.decode( pu8_option_value, (uint8_t)u16_option_len );
      set_block1( block_option );
    }
    break;

    case COAP_OPTION_BLOCK_2:
    {
      BlockOption block_option{};
      block_option.decode( pu8_option_value, (uint8_t)u16_option_len );
      set_block2( block_option );
    }
    break;

    default:
    {
    }
    break;
  }
}

void CoapOptionsSet::add_option( const Option &option )
{
  add_option( option.get_no(), option.get_len(), const_cast<Option &>( option ).get_val() );
}


CoapOptionsSet & CoapOptionsSet::set_uri_host( const string &rcz_uri_host )
{
  this->_uri_host = rcz_uri_host;
  return ( *this );
}

uint16_t CoapOptionsSet::get_uri_paths_count() const
{
  return ( uint16_t( _uri_paths_list.size() ) );
}

string CoapOptionsSet::get_uri_path_string()
{
  string cz_uri_path_string{"/"};

  for( auto &uri_path : _uri_paths_list )
  {
    cz_uri_path_string.append( uri_path ).append( "/" );
  }

  if( cz_uri_path_string.empty() )
  {
    return ( string() );
  }

  cz_uri_path_string.pop_back();
  return ( cz_uri_path_string );
}

void CoapOptionsSet::set_uri_path_string( const string &rcz_uri_path )
{
  clear_uri_path_list();
  string::size_type u8_slash_count = 0;
  uint8_t           u8_index       = 0;

  while( rcz_uri_path[u8_index++] == '/' )
  {
    ++u8_slash_count;
  }

  if( u8_slash_count > 0 )
  {
    auto uri_path = rcz_uri_path.substr( u8_slash_count, rcz_uri_path.length() );
    tokenize( uri_path, '/', _uri_paths_list );
  }
  else
  {
    tokenize( rcz_uri_path, '/', _uri_paths_list );
  }
}

void CoapOptionsSet::add_uri_path( const string &rcz_uri_path )
{
  if( !rcz_uri_path.empty() )
  {
    _uri_paths_list.push_back( rcz_uri_path );
  }
}

uint16_t CoapOptionsSet::get_uri_querys_count() const
{
  return ( uint16_t( _uri_querys_list.size() ) );
}

string CoapOptionsSet::get_uri_query_string()
{
  string cz_uri_query_string{};

  for( auto &uri_path : _uri_querys_list )
  {
    cz_uri_query_string.append( uri_path ).append( "&" );
  }

  if( cz_uri_query_string.empty() )
  {
    return ( string() );
  }

  cz_uri_query_string.pop_back();
  return ( cz_uri_query_string );
}

void CoapOptionsSet::set_uri_query_string( const string &rcz_uri_query )
{
  clear_uri_querys_list();
  string::size_type u8_question_count = 0;
  uint8_t           u8_index          = 0;

  while( rcz_uri_query[u8_index++] == '?' )
  {
    ++u8_question_count;
  }

  if( u8_question_count > 0 )
  {
    auto uri_path = rcz_uri_query.substr( u8_question_count, rcz_uri_query.length() );
    tokenize( uri_path, '&', _uri_querys_list );
  }
  else
  {
    tokenize(rcz_uri_query, '&', _uri_querys_list );
  }
}

void CoapOptionsSet::add_uri_query( const string &rcz_uri_query )
{
  if( !rcz_uri_query.empty() )
  {
    _uri_querys_list.push_back( rcz_uri_query );
  }
}

CoapOptionsSet & CoapOptionsSet::set_content_format( const uint16_t u16_content_format )
{
  this->_content_format = u16_content_format;
  return ( *this );
}

CoapOptionsSet & CoapOptionsSet::set_content_version( const uint16_t u16_content_version )
{
  this->_content_version = u16_content_version;
  return ( *this );
}

CoapOptionsSet & CoapOptionsSet::set_accept_format( const uint16_t u16_accept_format )
{
  this->_accept_format = u16_accept_format;
  return ( *this );
}

CoapOptionsSet & CoapOptionsSet::set_accept_version( const uint16_t u16_accept_version )
{
  this->_accept_version = u16_accept_version;
  return ( *this );
}

CoapOptionsSet & CoapOptionsSet::set_block1( uint8_t szx, bool m, uint32_t num )
{
  _cz_block_1.set_szx( szx );
  _cz_block_1.set_more( m );
  _cz_block_1.set_num( num );

  _options_bit.set( 0 );

  return ( *this );
}

CoapOptionsSet & CoapOptionsSet::set_block1( const BlockOption &rcz_block1_option )
{
  _cz_block_1 = rcz_block1_option;

  _options_bit.set( 0 );

  return ( *this );
}

CoapOptionsSet & CoapOptionsSet::set_block1( uint8_t *buf, uint8_t bufLen )
{
  _cz_block_1.decode( buf, bufLen );

  _options_bit.set( 0 );

  return ( *this );
}

CoapOptionsSet & CoapOptionsSet::set_block2( uint8_t szx, bool m, uint32_t num )
{
  _cz_block_2.set_szx( szx );
  _cz_block_2.set_more( m );
  _cz_block_2.set_num( num );

  _options_bit.set( 1 );

  return ( *this );
}

CoapOptionsSet & CoapOptionsSet::set_block2( const BlockOption &rcz_block2_option )
{
  _cz_block_2 = rcz_block2_option;

  _options_bit.set( 1 );

  return ( *this );
}

CoapOptionsSet & CoapOptionsSet::set_block2( uint8_t *buf, uint8_t bufLen )
{
  _cz_block_2.decode( buf, bufLen );

  _options_bit.set( 1 );

  return ( *this );
}

const char * CoapOptionsSet::get_option_no_string( uint16_t option_no )
{
  switch( option_no )
  {
    case COAP_OPTION_URI_PATH:
    {
      return ( "U_PATH" );
    }
		case COAP_OPTION_URI_QUERY:
		{
			return ("U_QRY");
		}
		case COAP_OPTION_SIZE1:
    {
      return ( "SZ1" );
    }
    case COAP_OPTION_SIZE2:
    {
      return ( "SZ2" );
    }
    case COAP_OPTION_BLOCK_1:
    {
      return ( "BLK1" );
    }
    case COAP_OPTION_BLOCK_2:
    {
      return ( "BLK2" );
    }
		case COAP_OPTION_CONTENT_FORMAT:
		{
			return ("CF");
		}
		case COAP_OPTION_ACCEPT:
		{
			return ("AF");
		}
		default:
    {
    }
    break;
  }

  return ( "" );
}

void CoapOptionsSet::get_option_value_as_string(Option& rcz_option, char *pc_buffer, uint16_t u16_buf_len )
{
  switch( rcz_option.get_no() )
  {
    case COAP_OPTION_URI_PATH:
    {
      sprintf( pc_buffer, "%s", rcz_option.get_string_value().c_str() );
    }
    break;
    case COAP_OPTION_BLOCK_1:
    {
      sprintf( pc_buffer, "[%d,%d,%d]", _cz_block_1.get_num(), _cz_block_1.has_more(), _cz_block_1.get_size() );
    }
    break;
    case COAP_OPTION_BLOCK_2:
    {
      sprintf( pc_buffer, "[%d,%d,%d]", _cz_block_2.get_num(), _cz_block_2.has_more(), _cz_block_2.get_size() );
    }
    break;
		case COAP_OPTION_SIZE1:
		case COAP_OPTION_SIZE2:
		case COAP_OPTION_CONTENT_FORMAT:
		case COAP_OPTION_ACCEPT:
		{
			sprintf(pc_buffer, "%d", rcz_option.get_integer_value());
		}
		break;
		default:
    {
      sprintf( pc_buffer, "NA" );
    }
    break;
  }
}

void tokenize( string in_str, const char delimter, vector<string> &rcz_token_list )
{
  auto start = in_str.find_first_not_of( delimter ), end = start;

  while( start != string::npos )
  {
    // Find next occurrence of delimiter
    end = in_str.find( delimter, start );

    // Push back the token found into vector
    rcz_token_list.push_back( in_str.substr( start, end - start ) );

    // Skip all occurrences of the delimiter to find new start
    start = in_str.find_first_not_of( delimter, end );
  }
}
}
}
