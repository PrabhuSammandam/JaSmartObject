#pragma once

#include "ErrCode.h"
#include "byte_array.h"
#include <base_utils.h>
#include <options/coap_block_option.h>
#include <options/coap_option.h>
#include <options/coap_option_set.h>

namespace ja_iot {
namespace coap {
using ErrCode   = ja_iot::base::ErrCode;
using ByteArray = ja_iot::base::ByteArray;

const uint32_t OptionBitmaskIfMatch     = 0x00000001;
const uint32_t OptionBitmaskUriHost     = 0x00000002;
const uint32_t OptionBitmaskEtag        = 0x00000004;
const uint32_t OptionBitmaskIfNoneMatch = 0x00000008;

const uint32_t OptionBitmaskUriPort       = 0x00000010;
const uint32_t OptionBitmaskLocationPath  = 0x00000020;
const uint32_t OptionBitmaskUriPath       = 0x00000040;
const uint32_t OptionBitmaskContentFormat = 0x00000080;

const uint32_t OptionBitmaskMaxAge        = 0x00000100;
const uint32_t OptionBitmaskUriQuery      = 0x00000200;
const uint32_t OptionBitmaskAccept        = 0x00000400;
const uint32_t OptionBitmaskLocationQuery = 0x00000800;

const uint32_t OptionBitmaskProxyUri    = 0x00001000;
const uint32_t OptionBitmaskProxyScheme = 0x00002000;

const uint32_t OptionBitmaskSize1 = 0x00004000;
const uint32_t OptionBitmaskSize2 = 0x00008000;

const uint32_t OptionBitmaskBlock1  = 0x00010000;
const uint32_t OptionBitmaskBlock2  = 0x00020000;
const uint32_t OptionBitmaskObserve = 0x00040000;

class OptionsSet
{
  OptionList   _optionList;
  uint32_t     _status = 0;

  public:

    OptionsSet ();

    ~OptionsSet ();

    template<typename T>
    ErrCode AddOption( uint16_t optionNo, T val )
    {
      uint8_t tempByte[sizeof( T )] {};

      for( uint8_t i = 0; i < sizeof( T ); i++ )
      {
        tempByte[i] = ( val >> ( ( ( sizeof( T ) - 1 ) - i ) * 8 ) ) & 0xFF;
      }

      return ( AddOption( optionNo, sizeof( uint32_t ), &tempByte[0] ) );
    }

    ErrCode AddOption( uint16_t optionNo, uint16_t val_len, uint8_t *value );

    uint32_t Get4ByteOption( OptionType optionNo ) const;
    Option*  GetOption( OptionType optionNo ) const;

    bool HasObserve() const { return ( ja_iot::base::is_bit_set( _status, OptionBitmaskObserve ) ); }

    uint32_t GetSize1() const;
    void     SetSize1( uint32_t size1 );
    bool     HasSize1() const { return ( ja_iot::base::is_bit_set( _status, OptionBitmaskSize1 ) ); }

    uint32_t GetSize2() const;
    void     SetSize2( uint32_t size2 );
    bool     HasSize2() const { return ( ja_iot::base::is_bit_set( _status, OptionBitmaskSize2 ) ); }

    void GetBlock1( BlockOption &blockOpt ) const;
    void SetBlock1( const BlockOption &block1 );
    bool HasBlock1() const { return ( ja_iot::base::is_bit_set( _status, OptionBitmaskBlock1 ) ); }

    void GetBlock2( BlockOption &blockOpt ) const;
    void SetBlock2( const BlockOption &block2 );
    bool HasBlock2() const { return ( ja_iot::base::is_bit_set( _status, OptionBitmaskBlock2 ) ); }

    uint16_t GetContentFormat() const;
    bool     HasContentFormat() const { return ( ja_iot::base::is_bit_set( _status, OptionBitmaskContentFormat ) ); }

    bool HasOpt( uint16_t optionNo ) const;

    uint16_t GetTotalSizeInBytes() const;

    ErrCode FreeOptions();

    uint16_t GetNoOfOptions() const { return ( _optionList.Size() ); }

    uint16_t CheckOptions();

    OptionList* OptList() { return ( &_optionList ); }

    void GetUriPathString( ByteArray &uriPath ) const;

    void Copy( OptionsSet &src );

    void Print() const;
    void Init();
    void Release();

#if 0
    ErrCode AddOption( uint16_t optionNo, uint16_t val );
    ErrCode AddOption( uint16_t optionNo, uint32_t val );
    ErrCode AddOption( uint16_t optionNo, uint64_t val );
#endif
};
}
}