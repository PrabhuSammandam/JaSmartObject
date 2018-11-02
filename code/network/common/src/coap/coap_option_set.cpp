#include <algorithm>
#include <functional>
#include <bitset>
#include <set>
#include <cstring>
#include <coap/coap_option_set.h>
#include "coap/coap_consts.h"
#include <common_defs.h>
#include <array>

namespace ja_iot {
namespace network {
using namespace base;

static std::array<uint16_t, 13> nonRepeatableOption = { { uint16_t( COAP_OPTION_URI_HOST ),
                                                          uint16_t( COAP_OPTION_IF_NONE_MATCH ),
                                                          uint16_t( COAP_OPTION_OBSERVE ),
                                                          uint16_t( COAP_OPTION_URI_PORT ),
                                                          uint16_t( COAP_OPTION_CONTENT_FORMAT ),
                                                          uint16_t( COAP_OPTION_MAX_AGE ),
                                                          uint16_t( COAP_OPTION_ACCEPT ),
                                                          uint16_t( COAP_OPTION_PROXY_URI ),
                                                          uint16_t( COAP_OPTION_PROXU_SCHEME ),
                                                          uint16_t( COAP_OPTION_SIZE1 ),
                                                          uint16_t( COAP_OPTION_SIZE2 ),
                                                          uint16_t( COAP_OPTION_BLOCK_1 ),
                                                          uint16_t( COAP_OPTION_BLOCK_2 ) } };
OptionsSet::OptionsSet () : _options_set{}, _bits{}
{
  _bits.reset();
}
OptionsSet::~OptionsSet ()
{
  std::for_each( _options_set.cbegin(), _options_set.cend(), [] ( Option *option ) {
        delete option;
      } );
  _bits.reset();
}

ErrCode OptionsSet::add_option( uint16_t u16_option_no, uint16_t u16_option_len, uint8_t *pu8_option_value )
{
  auto pcz_option = new Option{ u16_option_no, u16_option_len, pu8_option_value };

  if( pcz_option == nullptr )
  {
    return ErrCode::OUT_OF_MEM;
  }

  _options_set.insert( pcz_option );
  return ErrCode::OK;
}

ErrCode OptionsSet::add_option( uint16_t u16_option_no, uint32_t u32_value )
{
  uint8_t u8_value_len{};
  uint8_t au8_temp_value[4] {};

  for( uint8_t i = 0; i < 4; ++i )
  {
    uint32_t temp_max_value = ( 1 << i * 8 ) - 1;

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

  return add_option( u16_option_no, u8_value_len, &au8_temp_value[0] );
}

uint32_t OptionsSet::get_value( uint16_t u16_option_no )
{
  uint32_t value = 0;
  auto     opt   = get_option( u16_option_no );

  if( opt != nullptr )
  {
    auto opt_len  = opt->get_len();
    auto byte_val = opt->get_val();

    for( int i = 0; i < opt_len; i++ )
    {
      value |= (uint32_t) ( byte_val[opt_len - i - 1] & 0xFF ) << i * 8;
    }
  }

  return value;
}

Option * OptionsSet::get_option( uint16_t e_option_type ) const
{
  for( auto &option : _options_set )
  {
    if( option->get_no() == (uint16_t) e_option_type )
    {
      return option;
    }
  }

  return nullptr;
  // auto find_result = std::find_if( _options_set.cbegin(), _options_set.cend(), [&] ( const Option *pcz_option ) {
  // return ( pcz_option->get_no() == (uint16_t) e_option_type );
  // } );
  //
  // return ( find_result != _options_set.cend() ? *find_result : nullptr );
}

uint32_t OptionsSet::GetSize1()
{
  return get_value( COAP_OPTION_SIZE1 );
}

uint32_t OptionsSet::GetSize2()
{
  return get_value( COAP_OPTION_SIZE2 );
}

void OptionsSet::GetBlock1( BlockOption &blockOpt )
{
  auto opt = get_option( COAP_OPTION_BLOCK_1 );

  blockOpt.decode( opt->get_val(), opt->get_len() );
}

void OptionsSet::GetBlock2( BlockOption &blockOpt )
{
  auto opt = get_option( COAP_OPTION_BLOCK_2 );

  blockOpt.decode( opt->get_val(), opt->get_len() );
}

void OptionsSet::SetBlock1( const BlockOption &block1 )
{
  uint8_t len = 0;
  uint8_t au8_encode_buf[8];

  block1.encode( &au8_encode_buf[0], len );
  add_option( uint16_t( COAP_OPTION_BLOCK_1 ), len, &au8_encode_buf[0] );
  _bits.set( k_option_bit_Block1 );
}

void OptionsSet::SetBlock2( const BlockOption &block2 )
{
  uint8_t len = 0;
  uint8_t au8_encode_buf[8];

  block2.encode( &au8_encode_buf[0], len );
  add_option( uint16_t( COAP_OPTION_BLOCK_2 ), len, &au8_encode_buf[0] );
  _bits.set( k_option_bit_Block2 );
}

void OptionsSet::SetSize1( uint32_t size1 )
{
  add_option( uint16_t( COAP_OPTION_SIZE1 ), size1 );
  _bits.set( k_option_bit_Size1 );
}

void OptionsSet::SetSize2( uint32_t size2 )
{
  add_option( uint16_t( COAP_OPTION_SIZE2 ), size2 );
  _bits.set( k_option_bit_Size2 );
}

void OptionsSet::SetObserve( uint32_t u32_observe_value )
{
  add_option( uint16_t( COAP_OPTION_OBSERVE ), u32_observe_value );
  _bits.set( k_option_bit_Observe );
}

uint16_t OptionsSet::GetContentFormat()
{
  return _bits[k_option_bit_ContentFormat] ? get_value( COAP_OPTION_CONTENT_FORMAT ) : -1;
}

uint32_t OptionsSet::GetObserve()
{
  return _bits[k_option_bit_Observe] ? get_value( COAP_OPTION_OBSERVE ) : -1;
}

uint16_t OptionsSet::GetContentVersion()
{
  return _bits[k_option_bit_ContentVersion] ? get_value( COAP_OPTION_CONTENT_VERSION ) : -1;
}

uint16_t OptionsSet::GetAccept()
{
  return _bits[k_option_bit_Accept] ? get_value( COAP_OPTION_ACCEPT ) : -1;
}

uint16_t OptionsSet::GetAcceptVersion()
{
  return _bits[k_option_bit_AcceptVersion] ? get_value( COAP_OPTION_ACCEPT_VERSION ) : -1;
}

ErrCode OptionsSet::FreeOptions()
{
  std::for_each( _options_set.cbegin(), _options_set.cend(), [] ( Option *pcz_option ) {
        delete pcz_option;
      } );

  _options_set.clear();

  return ErrCode::OK;
}

uint16_t OptionsSet::GetTotalSizeInBytes() const
{
  return 0;
}

bool OptionsSet::has_option( uint16_t u16_option_no ) const
{
  return get_option( (uint16_t) u16_option_no ) != nullptr;
}

#if 1
static uint8_t find_option_no_in_repeat_array( _in_ uint16_t u16_option_no )
{
  auto it_find_result = std::find( nonRepeatableOption.cbegin(), nonRepeatableOption.cend(), u16_option_no );

  return it_find_result != nonRepeatableOption.cend() ? *it_find_result : 0xFF;
}

static void add_option_no_to_repeat_array( _in_ uint16_t u16_option_no )
{
  for( uint16_t i = 0; i < nonRepeatableOption.size(); i++ )
  {
    if( 0 == nonRepeatableOption[i] )
    {
      nonRepeatableOption[i] = u16_option_no;
      break;
    }
  }
}

static uint16_t check_option_no_repeated( _in_ uint16_t u16_option_no )
{
  if( find_option_no_in_repeat_array( u16_option_no ) == 0xFF )
  {
    add_option_no_to_repeat_array( u16_option_no );
  }
  else
  {
    if( ( u16_option_no & 0x01 ) == 0x01 )
    {
      return OPTION_ERROR_CRITICAL_REPEAT_MORE;
    }

    return OPTION_ERROR_REPEAT_MORE;
  }

  return 0;
}

static uint16_t check_option_for_length( _in_ Option *pcz_option, _in_ uint16_t min, _in_ uint16_t max )
{
  uint16_t optLen = pcz_option->get_len();

  if( optLen < min || optLen > max )
  {
    if( ( pcz_option->get_no() & 0x01 ) == 0x01 )
    {
      return OPTION_ERROR_CRITICAL_LEN_RANGE_OUT;
    }

    return OPTION_ERROR_LEN_RANGE_OUT;
  }

  return 0;
}

uint16_t OptionsSet::CheckOptions()
{
  nonRepeatableOption.fill( 0 );

  uint16_t errMask = 0;

  for( auto it = _options_set.cbegin(); it != _options_set.cend(); ++it )
  {
    auto p_cur_option  = *it;
    auto cur_option_no = p_cur_option->get_no();

    switch( uint16_t( cur_option_no ) )
    {
      case COAP_OPTION_IF_MATCH:
      {
        _bits.set( k_option_bit_IfMatch );
        errMask |= check_option_for_length( p_cur_option, 0, 8 );
      }
      break;

      case COAP_OPTION_URI_HOST:
      {
        _bits.set( k_option_bit_UriHost );
        errMask |= check_option_for_length( p_cur_option, 1, 255 );
        errMask |= check_option_no_repeated( cur_option_no );
      }
      break;

      case COAP_OPTION_ETAG:
      {
        _bits.set( k_option_bit_Etag );
        errMask |= check_option_for_length( p_cur_option, 1, 8 );
      }
      break;
      case COAP_OPTION_IF_NONE_MATCH:
      {
        _bits.set( k_option_bit_IfNoneMatch );
        errMask |= check_option_for_length( p_cur_option, 0, 0 );
        errMask |= check_option_no_repeated( cur_option_no );
      }
      break;
      case COAP_OPTION_OBSERVE:
      {
        _bits.set( k_option_bit_Observe );
        errMask |= check_option_for_length( p_cur_option, 0, 3 );
        errMask |= check_option_no_repeated( cur_option_no );
      }
      break;
      case COAP_OPTION_URI_PORT:
      {
        _bits.set( k_option_bit_UriPort );
        errMask |= check_option_for_length( p_cur_option, 0, 2 );
        errMask |= check_option_no_repeated( cur_option_no );
      }
      break;
      case COAP_OPTION_LOCATION_PATH:
      {
        _bits.set( k_option_bit_LocationPath );
        errMask |= check_option_for_length( p_cur_option, 0, 255 );
      }
      break;
      case COAP_OPTION_URI_PATH:
      {
        _bits.set( k_option_bit_UriPath );
        errMask |= check_option_for_length( p_cur_option, 0, 255 );
      }
      break;
      case COAP_OPTION_CONTENT_FORMAT:
      {
        _bits.set( k_option_bit_ContentFormat );
        errMask |= check_option_for_length( p_cur_option, 0, 2 );
        errMask |= check_option_no_repeated( cur_option_no );
      }
      break;
      case COAP_OPTION_MAX_AGE:
      {
        _bits.set( k_option_bit_MaxAge );
        errMask |= check_option_for_length( p_cur_option, 0, 4 );
        errMask |= check_option_no_repeated( cur_option_no );
      }
      break;
      case COAP_OPTION_URI_QUERY:
      {
        _bits.set( k_option_bit_UriQuery );
        errMask |= check_option_for_length( p_cur_option, 0, 255 );
      }
      break;
      case COAP_OPTION_ACCEPT:
      {
        _bits.set( k_option_bit_Accept );
        errMask |= check_option_for_length( p_cur_option, 0, 2 );
        errMask |= check_option_no_repeated( cur_option_no );
      }
      break;
      case COAP_OPTION_LOCATION_QUERY:
      {
        _bits.set( k_option_bit_LocationQuery );
        errMask |= check_option_for_length( p_cur_option, 0, 255 );
      }
      break;
      case COAP_OPTION_PROXY_URI:
      {
        _bits.set( k_option_bit_ProxyUri );
        errMask |= check_option_for_length( p_cur_option, 1, 1034 );
        errMask |= check_option_no_repeated( cur_option_no );
      }
      break;
      case COAP_OPTION_PROXU_SCHEME:
      {
        _bits.set( k_option_bit_ProxyScheme );
        errMask |= check_option_for_length( p_cur_option, 1, 255 );
        errMask |= check_option_no_repeated( cur_option_no );
      }
      break;
      case COAP_OPTION_SIZE1:
      {
        _bits.set( k_option_bit_Size1 );
        errMask |= check_option_for_length( p_cur_option, 0, 4 );
        errMask |= check_option_no_repeated( cur_option_no );
      }
      break;

      case COAP_OPTION_SIZE2:
      {
        _bits.set( k_option_bit_Size2 );
        errMask |= check_option_for_length( p_cur_option, 0, 4 );
        errMask |= check_option_no_repeated( cur_option_no );
      }
      break;
      case COAP_OPTION_BLOCK_1:
      {
        _bits.set( k_option_bit_Block1 );
        errMask |= check_option_for_length( p_cur_option, 0, 3 );
        errMask |= check_option_no_repeated( cur_option_no );
      }
      break;
      case COAP_OPTION_BLOCK_2:
      {
        _bits.set( k_option_bit_Block2 );
        errMask |= check_option_for_length( p_cur_option, 0, 3 );
        errMask |= check_option_no_repeated( cur_option_no );
      }
      break;
      case COAP_OPTION_CONTENT_VERSION:
      {
        _bits.set( k_option_bit_ContentVersion );
      }
      break;
      case COAP_OPTION_ACCEPT_VERSION:
      {
        _bits.set( k_option_bit_AcceptVersion );
      }
      break;

      default:
      {
        if( ( p_cur_option->get_no() & 0x01 ) == 0x01 )
        {
          errMask |= OPTION_ERROR_CRITICAL_UNKNOWN;
        }
        else
        {
          errMask |= OPTION_ERROR_UNKNOWN;
        }
      }
      break;
    }
  }

  return errMask;
}

/***
 * TODO : need to test with unit test.
 * Returns the total length of the uri query with ';' for each segment and also returns the total no of the segments.
 * @param ru16_no_of_segments - holds the no of uri_query options.
 * @return total length of the uri query string with including the count for ';' for each segment.
 */

uint16_t OptionsSet::get_uri_query_length( uint16_t &ru16_no_of_segments ) const
{
  uint16_t u16_cur_len = 0;

  ru16_no_of_segments = 0;

  if( _options_set.empty() )
  {
    return 1;
  }

  std::for_each( _options_set.cbegin(), _options_set.cend(), [&] ( Option *option ) {
        if( option->get_no() == (uint16_t) COAP_OPTION_URI_QUERY )
        {
          ru16_no_of_segments++;
          u16_cur_len += option->get_len() + 1;                               // +1 is for the ';'
        }
      } );

  return u16_cur_len;
}

/***
 * Returns the total length of the uri path with '\' for each segment and also returns the total no of the segments.
 * If there are no uri_path options in the list then length of 1 is returned for '\'.
 * @param ru16_no_of_segments - holds the no of uri_path options.
 * @return total length of the uri path string with including the count for '\' for each segment.
 */
uint16_t OptionsSet::get_uri_path_length( uint16_t &ru16_no_of_segments ) const
{
  uint16_t u16_cur_len = 0;

  ru16_no_of_segments = 0;

  if( _options_set.empty() )
  {
    return 1;
  }

  std::for_each( _options_set.cbegin(), _options_set.cend(), [&] ( Option *option ) {
        if( option->get_no() == (uint16_t) COAP_OPTION_URI_PATH )
        {
          ru16_no_of_segments++;
          u16_cur_len += option->get_len() + 1;                                       // +1 is for the '/'
        }
      } );

  return u16_cur_len;
}

/***
 * Returns the uri path with '/' appended. If there is no uri_path option in the list then single '/' is returned.
 * the total length if the uri path is also returned.
 *
 * @param cz_uri_path_byte_array - ByteArray which holds the memory to copy the full uri.
 *                                                                      The uri path encoded is null terminated.
 */
void OptionsSet::GetUriPathString( ByteArray &cz_uri_path_byte_array ) const
{
  uint16_t u16_cur_byte_index = 0;
  auto     pu8_byte_array     = cz_uri_path_byte_array.get_array();

  if( _options_set.empty() )
  {
    pu8_byte_array[0] = '/';
    pu8_byte_array[1] = '\0';
    cz_uri_path_byte_array.set_len( 1 );
    return;
  }

  for( auto it = _options_set.cbegin(); it != _options_set.cend(); ++it )
  {
    auto opt = *it;

    if( opt->get_no() == uint16_t( COAP_OPTION_URI_PATH ) && opt->get_len() > 0 )
    {
      pu8_byte_array[u16_cur_byte_index++] = '/';
      std::memcpy( &pu8_byte_array[u16_cur_byte_index], opt->get_val(), opt->get_len() );

      u16_cur_byte_index += opt->get_len();
    }
  }

  pu8_byte_array[u16_cur_byte_index] = '\0';

  cz_uri_path_byte_array.set_len( u16_cur_byte_index );
}

void OptionsSet::get_uri( std::string &rcz_uri ) const
{
  rcz_uri.clear();

  if( _options_set.empty() )
  {
    rcz_uri.append( "/" );
    return;
  }

  uint16_t no_of_segments;
  auto     total_length = get_uri_path_length( no_of_segments );

  rcz_uri.reserve( total_length );

  std::for_each( _options_set.cbegin(), _options_set.cend(), [&] ( Option *option ) {
        if( option->get_no() == uint16_t( COAP_OPTION_URI_PATH ) && option->get_len() > 0 )
        {
          rcz_uri.append( "/" );
          rcz_uri.append( (const char *) option->get_val(), option->get_len() );
        }
      } );
}

void OptionsSet::get_query( std::string &rcz_query ) const
{
  rcz_query.clear();

  if( _options_set.empty() )
  {
    return;
  }

  bool     is_first = true;
  uint16_t no_of_segments;
  auto     total_length = get_uri_query_length( no_of_segments );

  rcz_query.reserve( total_length );

  std::for_each( _options_set.cbegin(), _options_set.cend(), [&] ( Option *option ) {
        if( option->get_no() == uint16_t( COAP_OPTION_URI_QUERY ) && option->get_len() > 0 )
        {
          if( is_first )
          {
            is_first = false;
          }
          else
          {
            rcz_query.append( ";" );
          }

          rcz_query.append( (const char *) option->get_val(), option->get_len() );
        }
      } );
}

void OptionsSet::get_full_uri( std::string &rcz_full_uri ) const
{
  rcz_full_uri.clear();

  if( _options_set.empty() )
  {
    rcz_full_uri.append( "/" );
    return;
  }

  uint16_t no_of_segments;
  auto     query_len = get_uri_query_length( no_of_segments );
  auto     path_len  = get_uri_path_length( no_of_segments );

  rcz_full_uri.reserve( path_len + query_len + ( query_len > 0 ) ? 1 : 0 );       // +1 for &

  get_uri( rcz_full_uri );

  if( query_len > 0 )
  {
    rcz_full_uri.append( "?" );

    std::string query{};
    get_query( query );
    rcz_full_uri.append( query );
  }
}


/***
 * Returns the full uri of the resource request including the uri_path and uri_query.
 * @param cz_uri_byte_array
 */
void OptionsSet::get_uri( ByteArray &cz_uri_byte_array ) const
{
  uint16_t u16_uri_string_index     = 0;
  bool     is_first_uri_path_found  = false;
  bool     is_first_uri_query_found = false;
  uint8_t *pu8_uri_string           = cz_uri_byte_array.get_array();

  for( auto it = _options_set.cbegin(); it != _options_set.cend(); ++it )
  {
    auto opt = *it;

    if( opt->get_len() > 0 && ( opt->get_no() == uint16_t( COAP_OPTION_URI_PATH ) || opt->get_no() == uint16_t( COAP_OPTION_URI_QUERY ) ) )
    {
      if( is_first_uri_path_found == false )
      {
        is_first_uri_path_found              = true;
        pu8_uri_string[u16_uri_string_index] = '/';
        u16_uri_string_index++;

        if( u16_uri_string_index + opt->get_len() < cz_uri_byte_array.get_len() )
        {
        	std::memcpy( &pu8_uri_string[u16_uri_string_index], opt->get_val(), opt->get_len() );
          u16_uri_string_index += opt->get_len();
        }
        else
        {
          goto exit_label_;
        }
      }
      else
      {
        if( opt->get_no() == uint16_t( COAP_OPTION_URI_PATH ) )
        {
          if( u16_uri_string_index < cz_uri_byte_array.get_len() )
          {
            pu8_uri_string[u16_uri_string_index] = '/';
            u16_uri_string_index++;
          }
          else
          {
            goto exit_label_;
          }
        }
        else if( opt->get_no() == uint16_t( COAP_OPTION_URI_QUERY ) )
        {
          if( is_first_uri_query_found == false )
          {
            is_first_uri_query_found = true;

            if( u16_uri_string_index < cz_uri_byte_array.get_len() )
            {
              pu8_uri_string[u16_uri_string_index] = '?';
              u16_uri_string_index++;
            }
            else
            {
              goto exit_label_;
            }
          }
          else
          {
            if( u16_uri_string_index < cz_uri_byte_array.get_len() )
            {
              pu8_uri_string[u16_uri_string_index] = ';';
              u16_uri_string_index++;
            }
            else
            {
              goto exit_label_;
            }
          }
        }

        if( u16_uri_string_index + opt->get_len() < cz_uri_byte_array.get_len() )
        {
        	std::memcpy( &pu8_uri_string[u16_uri_string_index], opt->get_val(), opt->get_len() );
          u16_uri_string_index += opt->get_len();
        }
        else
        {
          goto exit_label_;
        }
      }
    }
  }

exit_label_:

  pu8_uri_string[u16_uri_string_index] = '\0';
  cz_uri_byte_array.set_len( u16_uri_string_index );
}

void OptionsSet::Copy( OptionsSet &src )
{
}

void OptionsSet::Print() const
{
}

void OptionsSet::Init()
{
  FreeOptions();
  _bits.reset();
}

void OptionsSet::Release()
{
  Init();
}

#endif

#if 0
ErrCode OptionsSet::add_option( uint16_t optionNo, uint16_t val )
{
  uint8_t temp_byte[2] {};

  temp_byte[0] = ( val >> 8 ) & 0xFF;
  temp_byte[1] = val & 0xFF;

  return ( add_option( optionNo, sizeof( uint16_t ), &temp_byte[0] ) );
}

ErrCode OptionsSet::add_option( uint16_t optionNo, uint32_t val )
{
  uint8_t temp_byte[4] {};

  for( uint8_t i = 0; i < 4; i++ )
  {
    temp_byte[i] = ( val >> ( ( 3 - i ) * 8 ) ) & 0xFF;
  }

  return ( add_option( optionNo, sizeof( uint32_t ), &temp_byte[0] ) );
}

ErrCode OptionsSet::add_option( uint16_t optionNo, uint64_t val )
{
  uint8_t temp_byte[8] {};

  for( uint8_t i = 0; i < 8; i++ )
  {
    temp_byte[i] = ( val >> ( ( 7 - i ) * 8 ) ) & 0xFF;
  }

  return ( add_option( optionNo, sizeof( uint64_t ), &temp_byte[0] ) );
}

uint32_t OptionsSet::Get4ByteOption( uint16_t optionNo ) const
{
  uint32_t value = 0;
  auto     opt   = get_option( optionNo );

  if( opt != nullptr )
  {
    for( auto i = 0; i < opt->get_len(); i++ )
    {
      value |= ( opt->get_val()[i] & 0xFF ) << ( i * 8 );
    }
  }

  return ( value );
}

#endif
}
}
