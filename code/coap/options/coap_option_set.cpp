#include <options/coap_option_set.h>
#include <common_defs.h>
#include <string.h>

namespace ja_iot {
namespace coap {
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

static ByteArray _8byteEncodeArray( 8 );
static ByteArray _8byteDecodeArray{};


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

  block1.encode( _8byteEncodeArray.get_array(), len );
  _8byteEncodeArray.set_len( len );
  AddOption( uint16_t( OptionType::BLOCK_1 ), _8byteEncodeArray.get_len(), _8byteEncodeArray.get_array() );
  base::set_bit( _status, OptionBitmaskBlock1 );
}

void OptionsSet::SetBlock2( const BlockOption &block2 )
{
  uint8_t len = 0;

  block2.encode( _8byteEncodeArray.get_array(), len );
  _8byteEncodeArray.set_len( len );
  AddOption( uint16_t( OptionType::BLOCK_2 ), _8byteEncodeArray.get_len(), _8byteEncodeArray.get_array() );
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
  for( auto i = 0; i < ( sizeof( nonRepeatableOption ) / sizeof( uint16_t ) ); i++ )
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

void OptionsSet::GetUriPathString( ByteArray &fullUri ) const
{
  uint16_t currBytesIdx = 0;
  auto     byteArray    = fullUri.get_array();

  if( _optionList.Size() == 0 )
  {
    byteArray[0] = '/';
    fullUri.set_len( 1 );
    return;
  }

  for( auto it = _optionList.begin(); it != _optionList.end(); ++it )
  {
    auto opt = *it;

    if( ( opt->GetNo() == uint16_t( OptionType::URI_PATH ) ) && ( opt->GetLen() > 0 ) )
    {
      byteArray[currBytesIdx++] = '/';
      memcpy( &byteArray[currBytesIdx], opt->GetVal(), opt->GetLen() );

      currBytesIdx += opt->GetLen();
    }
  }

  byteArray[currBytesIdx] = '\0';

  fullUri.set_len( currBytesIdx );
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