#include <coap/coap_option_set.h>
#include <common_defs.h>
#include <string.h>

namespace ja_iot {
namespace network {
using namespace ja_iot::base;

static uint16_t  nonRepeatableOption[] = { uint16_t( OptionType::URI_HOST ),
                                           uint16_t( OptionType::IF_NONE_MATCH ),
                                           uint16_t( OptionType::OBSERVE ),
                                           uint16_t( OptionType::URI_PORT ),
                                           uint16_t( OptionType::CONTENT_FORMAT ),
                                           uint16_t( OptionType::MAX_AGE ),
                                           uint16_t( OptionType::ACCEPT ),
                                           uint16_t( OptionType::PROXY_URI ),
                                           uint16_t( OptionType::PROXU_SCHEME ),
                                           uint16_t( OptionType::SIZE1 ),
                                           uint16_t( OptionType::SIZE2 ),
                                           uint16_t( OptionType::BLOCK_1 ),
                                           uint16_t( OptionType::BLOCK_2 ) };

ErrCode OptionsSet::AddOption( uint16_t optionNo, uint16_t val_len, uint8_t *value )
{
  auto opt = Option::Allocate( optionNo, val_len, value );

  if( opt != nullptr )
  {
    _optionList.Insert( opt );

    return ( ErrCode::OK );
  }

  return ( ErrCode::OUT_OF_MEM );
}

uint32_t OptionsSet::Get4ByteOption( OptionType optionNo ) const
{
  uint32_t value = 0;
  auto     opt   = GetOption( optionNo );

  if( opt != nullptr )
  {
    for( auto i = 0; i < opt->GetLen(); i++ )
    {
      value |= ( opt->GetVal()[i] & 0xFF ) << ( i * 8 );
    }
  }

  return ( value );
}

Option * OptionsSet::GetOption( OptionType optionNo ) const
{
  for( auto it = _optionList.begin(); it != _optionList.end(); ++it )
  {
    auto opt = *it;

    if( opt->GetNo() == uint16_t( optionNo ) )
    {
      return ( opt );
    }
  }

  return ( nullptr );
}

uint32_t OptionsSet::GetSize1() const
{
  return ( Get4ByteOption( OptionType::SIZE1 ) );
}

uint32_t OptionsSet::GetSize2() const
{
  return ( Get4ByteOption( OptionType::SIZE2 ) );
}

void OptionsSet::GetBlock1( BlockOption &blockOpt ) const
{
  auto opt = GetOption( OptionType::BLOCK_1 );

  blockOpt.decode( opt->GetVal(), opt->GetLen() );
}

void OptionsSet::GetBlock2( BlockOption &blockOpt ) const
{
  auto opt = GetOption( OptionType::BLOCK_2 );

  blockOpt.decode( opt->GetVal(), opt->GetLen() );
}

void OptionsSet::SetBlock1( const BlockOption &block1 )
{
  uint8_t len = 0;
  uint8_t au8_encode_buf[8];

  block1.encode( &au8_encode_buf[0], len );
  AddOption( uint16_t( OptionType::BLOCK_1 ), len, &au8_encode_buf[0] );
  base::set_bit( _status, OptionBitmaskBlock1 );
}

void OptionsSet::SetBlock2( const BlockOption &block2 )
{
  uint8_t len = 0;
  uint8_t au8_encode_buf[8];

  block2.encode( &au8_encode_buf[0], len );
  AddOption( uint16_t( OptionType::BLOCK_2 ), len, &au8_encode_buf[0] );
  base::set_bit( _status, OptionBitmaskBlock2 );
}

void OptionsSet::SetSize1( uint32_t size1 )
{
  AddOption( uint16_t( OptionType::SIZE1 ), size1 );
  base::set_bit( _status, OptionBitmaskSize1 );
}

void OptionsSet::SetSize2( uint32_t size2 )
{
  AddOption( uint16_t( OptionType::SIZE2 ), size2 );
  base::set_bit( _status, OptionBitmaskSize2 );
}

uint16_t OptionsSet::GetContentFormat() const
{
  if( HasContentFormat() )
  {
    return ( static_cast<uint16_t>( Get4ByteOption( OptionType::CONTENT_FORMAT ) ) );
  }

  return ( 0xFFFF );
}

ErrCode OptionsSet::FreeOptions()
{
  _optionList.FreeList();

  return ( ErrCode::OK );
}

uint16_t OptionsSet::GetTotalSizeInBytes() const
{
  return ( 0 );
}

bool OptionsSet::HasOpt( uint16_t optionNo ) const
{
  if( _optionList.IsEmpty() )
  {
    return ( false );
  }

  auto it = _optionList.begin();

  for(; it != _optionList.end(); ++it )
  {
    if( ( *it )->GetNo() == optionNo )
    {
      return ( true );
    }
  }

  return ( false );
}

static uint8_t find_option_no_in_repeat_array( _in_ uint16_t optNo )
{
  for( uint16_t i = 0; i < sizeof( nonRepeatableOption ) / sizeof( uint16_t ); i++ )
  {
    if( optNo == nonRepeatableOption[i] )
    {
      return ( i );
    }
  }

  return ( 0xFF );
}

static void add_option_no_to_repeat_array( _in_ uint16_t optNo )
{
  for( uint16_t i = 0; i < sizeof( nonRepeatableOption ) / sizeof( uint16_t ); i++ )
  {
    if( 0 == nonRepeatableOption[i] )
    {
      nonRepeatableOption[i] = optNo;
      break;
    }
  }
}

static uint16_t check_option_no_repeated( _in_ uint16_t optNo )
{
  if( find_option_no_in_repeat_array( optNo ) == 0xFF )
  {
    add_option_no_to_repeat_array( optNo );
  }
  else
  {
    if( ( optNo & 0x01 ) == 0x01 )
    {
      return ( OPTION_ERROR_CRITICAL_REPEAT_MORE );
    }

    return ( OPTION_ERROR_REPEAT_MORE );
  }

  return ( 0 );
}

static uint16_t check_option_for_length( _in_ Option *option, _in_ uint16_t min, _in_ uint16_t max )
{
  uint16_t optLen = option->GetLen();

  if( ( optLen < min ) || ( optLen > max ) )
  {
    if( ( option->GetNo() & 0x01 ) == 0x01 )
    {
      return ( OPTION_ERROR_CRITICAL_LEN_RANGE_OUT );
    }

    return ( OPTION_ERROR_LEN_RANGE_OUT );
  }

  return ( 0 );
}

uint16_t OptionsSet::CheckOptions()
{
  for( uint16_t i = 0; i < ( sizeof( nonRepeatableOption ) / sizeof( uint16_t ) ); i++ )
  {
    nonRepeatableOption[i] = 0;
  }

  uint16_t errMask = 0;

  for( auto it = _optionList.begin(); ( it != _optionList.end() ); ++it )
  {
    auto p_cur_option  = *it;
    auto cur_option_no = p_cur_option->GetNo();

    switch( OptionType( cur_option_no ) )
    {
      case OptionType::IF_MATCH:
      {
        base::set_bit( _status, OptionBitmaskIfMatch );
        errMask |= check_option_for_length( p_cur_option, 0, 8 );
      }
      break;

      case OptionType::URI_HOST:
      {
        base::set_bit( _status, OptionBitmaskUriHost );
        errMask |= check_option_for_length( p_cur_option, 1, 255 );
        errMask |= check_option_no_repeated( cur_option_no );
      }
      break;

      case OptionType::ETAG:
      {
        base::set_bit( _status, OptionBitmaskEtag );
        errMask |= check_option_for_length( p_cur_option, 1, 8 );
      }
      break;
      case OptionType::IF_NONE_MATCH:
      {
        base::set_bit( _status, OptionBitmaskIfNoneMatch );
        errMask |= check_option_for_length( p_cur_option, 0, 0 );
        errMask |= check_option_no_repeated( cur_option_no );
      }
      break;
      case OptionType::OBSERVE:
      {
        base::set_bit( _status, OptionBitmaskObserve );
        errMask |= check_option_for_length( p_cur_option, 0, 3 );
        errMask |= check_option_no_repeated( cur_option_no );
      }
      break;
      case OptionType::URI_PORT:
      {
        base::set_bit( _status, OptionBitmaskUriPort );
        errMask |= check_option_for_length( p_cur_option, 0, 2 );
        errMask |= check_option_no_repeated( cur_option_no );
      }
      break;
      case OptionType::LOCATION_PATH:
      {
        base::set_bit( _status, OptionBitmaskLocationPath );
        errMask |= check_option_for_length( p_cur_option, 0, 255 );
      }
      break;
      case OptionType::URI_PATH:
      {
        base::set_bit( _status, OptionBitmaskUriPath );
        errMask |= check_option_for_length( p_cur_option, 0, 255 );
      }
      break;
      case OptionType::CONTENT_FORMAT:
      {
        base::set_bit( _status, OptionBitmaskContentFormat );
        errMask |= check_option_for_length( p_cur_option, 0, 2 );
        errMask |= check_option_no_repeated( cur_option_no );
      }
      break;
      case OptionType::MAX_AGE:
      {
        base::set_bit( _status, OptionBitmaskMaxAge );
        errMask |= check_option_for_length( p_cur_option, 0, 4 );
        errMask |= check_option_no_repeated( cur_option_no );
      }
      break;
      case OptionType::URI_QUERY:
      {
        base::set_bit( _status, OptionBitmaskUriQuery );
        errMask |= check_option_for_length( p_cur_option, 0, 255 );
      }
      break;
      case OptionType::ACCEPT:
      {
        base::set_bit( _status, OptionBitmaskAccept );
        errMask |= check_option_for_length( p_cur_option, 0, 2 );
        errMask |= check_option_no_repeated( cur_option_no );
      }
      break;
      case OptionType::LOCATION_QUERY:
      {
        base::set_bit( _status, OptionBitmaskLocationQuery );
        errMask |= check_option_for_length( p_cur_option, 0, 255 );
      }
      break;
      case OptionType::PROXY_URI:
      {
        base::set_bit( _status, OptionBitmaskProxyUri );
        errMask |= check_option_for_length( p_cur_option, 1, 1034 );
        errMask |= check_option_no_repeated( cur_option_no );
      }
      break;
      case OptionType::PROXU_SCHEME:
      {
        base::set_bit( _status, OptionBitmaskProxyScheme );
        errMask |= check_option_for_length( p_cur_option, 1, 255 );
        errMask |= check_option_no_repeated( cur_option_no );
      }
      break;
      case OptionType::SIZE1:
      {
        base::set_bit( _status, OptionBitmaskSize1 );
        errMask |= check_option_for_length( p_cur_option, 0, 4 );
        errMask |= check_option_no_repeated( cur_option_no );
      }
      break;

      case OptionType::SIZE2:
      {
        base::set_bit( _status, OptionBitmaskSize2 );
        errMask |= check_option_for_length( p_cur_option, 0, 4 );
        errMask |= check_option_no_repeated( cur_option_no );
      }
      break;
      case OptionType::BLOCK_1:
      {
        base::set_bit( _status, OptionBitmaskBlock1 );
        errMask |= check_option_for_length( p_cur_option, 0, 3 );
        errMask |= check_option_no_repeated( cur_option_no );
      }
      break;
      case OptionType::BLOCK_2:
      {
        base::set_bit( _status, OptionBitmaskBlock2 );
        errMask |= check_option_for_length( p_cur_option, 0, 3 );
        errMask |= check_option_no_repeated( cur_option_no );
      }
      break;

      default:
      {
        if( ( p_cur_option->GetNo() & 0x01 ) == 0x01 )
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

  return ( errMask );
}

/***
 * TODO : need to test with unit test.
 * Returns the total length of the uri query with ';' for each segment and also returns the total no of the segments.
 * @param u16_no_of_segments - holds the no of uri_query options.
 * @return total length of the uri query string with including the count for ';' for each segment.
 */

uint16_t OptionsSet::get_uri_query_length( uint16_t &u16_no_of_segments )
{
  uint16_t u16_cur_len = 0;

  u16_no_of_segments = 0;

  if( _optionList.Size() == 0 )
  {
    return ( 1 );
  }

  for( auto it = _optionList.begin(); it != _optionList.end(); ++it )
  {
    auto opt = *it;

    if( opt->GetNo() == uint16_t( OptionType::URI_QUERY ) )
    {
      u16_no_of_segments++;
      u16_cur_len += ( opt->GetLen() + 1 );                 // +1 is for the ';'
    }
  }

  return ( u16_cur_len );
}

/***
 * Returns the total length of the uri path with '\' for each segment and also returns the total no of the segments.
 * If there are no uri_path options in the list then length of 1 is returned for '\'.
 * @param u16_no_of_segments - holds the no of uri_path options.
 * @return total length of the uri path string with including the count for '\' for each segment.
 */
uint16_t OptionsSet::get_uri_path_length( uint16_t &u16_no_of_segments )
{
  uint16_t u16_cur_len = 0;

  u16_no_of_segments = 0;

  if( _optionList.Size() == 0 )
  {
    return ( 1 );
  }

  for( auto it = _optionList.begin(); it != _optionList.end(); ++it )
  {
    auto opt = *it;

    if( opt->GetNo() == uint16_t( OptionType::URI_PATH ) )
    {
      u16_no_of_segments++;
      u16_cur_len += ( opt->GetLen() + 1 );         // +1 is for the '/'
    }
  }

  return ( u16_cur_len );
}

/***
 * Returns the uri path with '/' appended. If there is no uri_path option in the list then single '\' is returned.
 * the total length if the uri path is also returned.
 *
 * @param cz_uri_path_byte_array - ByteArray which holds the memory to copy the full uri.
 *                                                                      The uri path encoded is null terminated.
 */
void OptionsSet::GetUriPathString( ByteArray &cz_uri_path_byte_array ) const
{
  uint16_t u16_cur_byte_index = 0;
  auto     pu8_byte_array     = cz_uri_path_byte_array.get_array();

  if( _optionList.Size() == 0 )
  {
    pu8_byte_array[0] = '/';
    pu8_byte_array[1] = '\0';
    cz_uri_path_byte_array.set_len( 1 );
    return;
  }

  for( auto it = _optionList.begin(); it != _optionList.end(); ++it )
  {
    auto opt = *it;

    if( ( opt->GetNo() == uint16_t( OptionType::URI_PATH ) ) && ( opt->GetLen() > 0 ) )
    {
      pu8_byte_array[u16_cur_byte_index++] = '/';
      memcpy( &pu8_byte_array[u16_cur_byte_index], opt->GetVal(), opt->GetLen() );

      u16_cur_byte_index += opt->GetLen();
    }
  }

  pu8_byte_array[u16_cur_byte_index] = '\0';

  cz_uri_path_byte_array.set_len( u16_cur_byte_index );
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

  for( auto it = _optionList.begin(); it != _optionList.end(); ++it )
  {
    auto opt = *it;

    if( ( opt->GetLen() > 0 ) && ( ( opt->GetNo() == uint16_t( OptionType::URI_PATH ) ) || ( opt->GetNo() == uint16_t( OptionType::URI_QUERY ) ) ) )
    {
      if( is_first_uri_path_found == false )
      {
        is_first_uri_path_found              = true;
        pu8_uri_string[u16_uri_string_index] = '/';
        u16_uri_string_index++;

        if( ( u16_uri_string_index + opt->GetLen() ) < cz_uri_byte_array.get_len() )
        {
          memcpy( &pu8_uri_string[u16_uri_string_index], opt->GetVal(), opt->GetLen() );
          u16_uri_string_index += opt->GetLen();
        }
        else
        {
          goto exit_label_;
        }
      }
      else
      {
        if( opt->GetNo() == uint16_t( OptionType::URI_PATH ) )
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
        else if( opt->GetNo() == uint16_t( OptionType::URI_QUERY ) )
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

        if( ( u16_uri_string_index + opt->GetLen() ) < cz_uri_byte_array.get_len() )
        {
          memcpy( &pu8_uri_string[u16_uri_string_index], opt->GetVal(), opt->GetLen() );
          u16_uri_string_index += opt->GetLen();
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
  if( src.GetNoOfOptions() == 0 )
  {
    return;
  }

  for( auto it = _optionList.begin(); it != _optionList.end(); ++it )
  {
    auto opt = *it;
    AddOption( opt->GetNo(), opt->GetLen(), opt->GetVal() );
  }

  this->_status = src._status;
}

void OptionsSet::Print() const
{
  _optionList.Print();
}

void OptionsSet::Init()
{
  _status = 0;
}

void OptionsSet::Release()
{
  _optionList.FreeList();
  Init();
}

OptionsSet::OptionsSet () :
  _optionList()
{
}

OptionsSet::~OptionsSet ()
{
  Release();
}

#if 0
ErrCode OptionsSet::AddOption( uint16_t optionNo, uint16_t val )
{
  uint8_t temp_byte[2] {};

  temp_byte[0] = ( val >> 8 ) & 0xFF;
  temp_byte[1] = val & 0xFF;

  return ( AddOption( optionNo, sizeof( uint16_t ), &temp_byte[0] ) );
}

ErrCode OptionsSet::AddOption( uint16_t optionNo, uint32_t val )
{
  uint8_t temp_byte[4] {};

  for( uint8_t i = 0; i < 4; i++ )
  {
    temp_byte[i] = ( val >> ( ( 3 - i ) * 8 ) ) & 0xFF;
  }

  return ( AddOption( optionNo, sizeof( uint32_t ), &temp_byte[0] ) );
}

ErrCode OptionsSet::AddOption( uint16_t optionNo, uint64_t val )
{
  uint8_t temp_byte[8] {};

  for( uint8_t i = 0; i < 8; i++ )
  {
    temp_byte[i] = ( val >> ( ( 7 - i ) * 8 ) ) & 0xFF;
  }

  return ( AddOption( optionNo, sizeof( uint64_t ), &temp_byte[0] ) );
}
#endif
}
}
