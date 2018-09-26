#pragma once
#include <set>
#include <bitset>
#include "ErrCode.h"
#include "byte_array.h"
#include <coap/coap_block_option.h>
#include <coap/coap_option.h>
#include <coap/coap_option_set.h>

namespace ja_iot {
namespace network {
using ByteArray = base::ByteArray;

const uint8_t k_option_bit_IfMatch        = 0;
const uint8_t k_option_bit_UriHost        = 1;
const uint8_t k_option_bit_Etag           = 2;
const uint8_t k_option_bit_IfNoneMatch    = 3;
const uint8_t k_option_bit_UriPort        = 4;
const uint8_t k_option_bit_LocationPath   = 5;
const uint8_t k_option_bit_UriPath        = 6;
const uint8_t k_option_bit_ContentFormat  = 7;
const uint8_t k_option_bit_MaxAge         = 8;
const uint8_t k_option_bit_UriQuery       = 9;
const uint8_t k_option_bit_Accept         = 10;
const uint8_t k_option_bit_LocationQuery  = 11;
const uint8_t k_option_bit_ProxyUri       = 12;
const uint8_t k_option_bit_ProxyScheme    = 13;
const uint8_t k_option_bit_Size1          = 14;
const uint8_t k_option_bit_Size2          = 15;
const uint8_t k_option_bit_Block1         = 16;
const uint8_t k_option_bit_Block2         = 17;
const uint8_t k_option_bit_Observe        = 18;
const uint8_t k_option_bit_ContentVersion = 19;
const uint8_t k_option_bit_AcceptVersion  = 20;

struct option_multiset_comparer
{
  bool operator () ( const Option *lhs, const Option *rhs ) const
  {
    return lhs->get_no() < rhs->get_no();
  }
};


class OptionsSet
{
  std::multiset<Option *, option_multiset_comparer>   _options_set;
  std::bitset<32>                                     _bits;

  public:
    OptionsSet ();
    ~OptionsSet ();

    base::ErrCode add_option( uint16_t u16_option_no, uint32_t u32_value );
    base::ErrCode add_option( uint16_t optionNo, uint16_t val_len, uint8_t *value );
    uint32_t              get_value( uint16_t u16_option_no );
    Option*               get_option( uint16_t optionNo ) const;
    bool                  has_option( uint16_t u16_option_no ) const;

    bool     HasObserve() const { return _bits[k_option_bit_Observe]; }
    uint32_t GetObserve();
    void     SetObserve( uint32_t u32_observe_value );

    uint32_t GetSize1();
    void     SetSize1( uint32_t size1 );
    bool     HasSize1() const { return _bits[k_option_bit_Size1]; }

    uint32_t GetSize2();
    void     SetSize2( uint32_t size2 );
    bool     HasSize2() const { return _bits[k_option_bit_Size2]; }

    void GetBlock1( BlockOption &blockOpt );
    void SetBlock1( const BlockOption &block1 );
    bool HasBlock1() const { return _bits[k_option_bit_Block1]; }

    void GetBlock2( BlockOption &blockOpt );
    void SetBlock2( const BlockOption &block2 );
    bool HasBlock2() const { return _bits[k_option_bit_Block2]; }

    uint16_t GetContentFormat();
    void     SetContentFormat( uint16_t u16_content_format );
    bool     HasContentFormat() const { return _bits[k_option_bit_ContentFormat]; }

    uint16_t GetContentVersion();
    void     SetContentVersion( uint16_t u16_content_version );
    bool     HasContentVersion() const { return _bits[k_option_bit_ContentVersion]; }

    uint16_t GetAccept();
    void     SetAcceptFormat( uint16_t u16_accept_format );
    bool     HasAccept() const { return _bits[k_option_bit_Accept]; }

    uint16_t GetAcceptVersion();
    void     SetAcceptVersion( uint16_t u16_accept_version );
    bool     HasAcceptVersion() const { return _bits[k_option_bit_AcceptVersion]; }

    uint16_t GetTotalSizeInBytes() const;

    base::ErrCode FreeOptions();

    uint16_t GetNoOfOptions() const { return _options_set.size(); }

    uint16_t CheckOptions();

    void     GetUriPathString( ByteArray &cz_uri_path_byte_array ) const;
    uint16_t get_uri_path_length( uint16_t &u16_no_of_segments ) const;
    uint16_t get_uri_query_length( uint16_t &u16_no_of_segments ) const;
    void     get_uri( ByteArray &cz_uri_byte_array ) const;
    void     get_uri( std::string &rcz_uri ) const;
    void     get_query( std::string &rcz_query ) const;
    void     get_full_uri( std::string &rcz_full_uri ) const;

    void Copy( OptionsSet &src );

    void Print() const;
    void Init();
    void Release();
};
}
}
