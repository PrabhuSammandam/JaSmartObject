/*
 * coap_options_set.h
 *
 *  Created on: Nov 12, 2017
 *      Author: psammand
 */

#pragma once

#include <string>
#include <functional>
#include <bitset>
#include <set>
#include <vector>
#include <cstdint>
#include "coap/coap_option.h"
#include <coap/coap_block_option.h>

namespace ja_iot {
namespace network {
struct option_mset_comparer
{
  bool operator () ( const Option &lhs, const Option &rhs ) const
  {
    return ( lhs.get_no() < rhs.get_no() );
  }
};

using UriPathList  = std::vector<std::string>;
using UriQueryList = std::vector<std::string>;
using OptionsList  = std::multiset<Option, option_mset_comparer>;

class CoapOptionsSet
{
  public:
    CoapOptionsSet ();
    ~CoapOptionsSet ();
    CoapOptionsSet( const CoapOptionsSet &other );
    CoapOptionsSet( CoapOptionsSet &&other ) noexcept;
    CoapOptionsSet & operator = ( const CoapOptionsSet &other );
    CoapOptionsSet & operator = ( CoapOptionsSet &&other );
    void                      clear();
    void                      add_option( const Option &option );
    void                      add_option( uint16_t u16_option_no, uint16_t u16_option_len, uint8_t *pu8_option_value );
    bool                      has_option( uint16_t u16_option_no ) const;
    OptionsList               get_sorted_options_list();
    /* uri host */
    bool          has_uri_host() const { return ( !_uri_host.empty() ); }
    std::string   get_uri_host() const { return ( _uri_host ); }
    CoapOptionsSet& set_uri_host( const std::string &rcz_uri_host );
    void          remove_uri_host() { _uri_host.clear(); }

    /* uri port */
    bool          has_uri_port() const { return ( _uri_port != 0xFFFF ); }
    uint16_t      get_uri_port() const { return ( _uri_port ); }
    CoapOptionsSet& set_uri_port( const uint16_t u16_uri_port ) { _uri_port = u16_uri_port; return ( *this ); }
    void          remove_uri_port() { this->_uri_port = 0xFFFF; }

    /* uri path */
    uint16_t    get_uri_paths_count() const;
    UriPathList & get_uri_path_list() { return ( this->_uri_paths_list ); }
    std::string get_uri_path_string();
    void        set_uri_path_string( const std::string &rcz_uri_path );
    void        clear_uri_path_list() { this->_uri_paths_list.clear(); }
    void        add_uri_path( const std::string &rcz_uri_path );

    /* uri query */
    uint16_t    get_uri_querys_count() const;
    UriQueryList& get_uri_querys_list() { return ( this->_uri_querys_list ); }
    std::string get_uri_query_string();
    void        set_uri_query_string( const std::string &rcz_uri_query );
    void        clear_uri_querys_list() { this->_uri_querys_list.clear(); }
    void        add_uri_query( const std::string &rcz_uri_query );

    /* content format */
    bool          has_content_format() const { return ( _content_format != 0xFFFF ); }
    uint16_t      get_content_format() const { return ( _content_format ); }
    CoapOptionsSet& set_content_format( uint16_t u16_content_format );
    CoapOptionsSet& remove_content_format() { _content_format = 0xFFFF; return ( *this ); }

    /* content version */
    bool          has_content_version() const { return ( _content_version != 0xFFFF ); }
    uint16_t      get_content_version() const { return ( _content_version ); }
    CoapOptionsSet& set_content_version( uint16_t u16_content_version );
    CoapOptionsSet& remove_content_version() { _content_version = 0xFFFF; return ( *this ); }

    /* accept format */
    bool          has_accept_format() const { return ( _accept_format != 0xFFFF ); }
    uint16_t      get_accept_format() const { return ( _accept_format ); }
    CoapOptionsSet& set_accept_format( uint16_t u16_accept_format );
    CoapOptionsSet& remove_accept_format() { _accept_format = 0xFFFF; return ( *this ); }

    /* accept version */
    bool          has_accept_version() const { return ( _accept_version != 0xFFFF ); }
    uint16_t      get_accept_version() const { return ( _accept_version ); }
    CoapOptionsSet& set_accept_version( uint16_t u16_accept_version );
    CoapOptionsSet& remove_accept_version() { _accept_version = 0xFFFF; return ( *this ); }

    /* Size 1 */
    bool          has_size1() const { return ( _u32_size_1 != 0 ); }
    uint32_t      get_size1() const { return ( _u32_size_1 ); }
    CoapOptionsSet& set_size1( const uint32_t u32_size1 ) { _u32_size_1 = u32_size1; return ( *this ); }
    CoapOptionsSet& remove_size1() { _u32_size_1 = 0; return ( *this ); }

    /* Size 2 */
    bool          has_size2() const { return ( _u32_size_2 != 0 ); }
    uint32_t      get_size2() const { return ( _u32_size_2 ); }
    CoapOptionsSet& set_size2( const uint32_t u32_size2 ) { _u32_size_2 = u32_size2; return ( *this ); }
    CoapOptionsSet& remove_size2() { _u32_size_2 = 0; return ( *this ); }

    /* Observe */
    bool          has_observe() const { return ( _u32_observe != 0xFFFFFFFF ); }
    uint32_t      get_observe() const { return ( _u32_observe ); }
    CoapOptionsSet& set_observe( const uint32_t u32_observe ) { _u32_observe = u32_observe; return ( *this ); }
    CoapOptionsSet& remove_observe() { _u32_observe = 0; return ( *this ); }

    /* Block1 */
    bool          has_block1() const { return ( _options_bit[0] ); }
    BlockOption   & get_block1() { return ( _cz_block_1 ); }
    CoapOptionsSet& set_block1( uint8_t szx, bool m, uint32_t num );
    CoapOptionsSet& set_block1( const BlockOption &rcz_block1_option );
    CoapOptionsSet& set_block1( uint8_t *buf, uint8_t bufLen );
    CoapOptionsSet& remove_block1() { _cz_block_1.clear(); _options_bit.reset( 0 ); return ( *this ); }

    /* Block2 */
    bool          has_block2() const { return ( _options_bit[1] ); }
    BlockOption   & get_block2() { return ( _cz_block_2 ); }
    CoapOptionsSet& set_block2( uint8_t szx, bool m, uint32_t num );
    CoapOptionsSet& set_block2( const BlockOption &rcz_block1_option );
    CoapOptionsSet& set_block2( uint8_t *buf, uint8_t bufLen );
    CoapOptionsSet& remove_block2() { _cz_block_2.clear(); _options_bit.reset( 1 ); return ( *this ); }

    static const char* get_option_no_string( uint16_t option_no );
    void               get_option_value_as_string(Option& option, char *buffer, uint16_t buff_len );

  private:
    std::string get_string_value( uint16_t u16_option_len, uint8_t *pu8_option_value );
    uint32_t    get_integer_value( uint16_t u16_option_len, uint8_t *pu8_option_value );

    std::string _uri_host{};
    uint16_t _uri_port{ 0xFFFF };
    UriPathList _uri_paths_list{};
    UriQueryList _uri_querys_list{};
    uint16_t _content_format{ 0xFFFF };
    uint16_t _content_version{ 0xFFFF };
    uint16_t _accept_format{ 0xFFFF };
    uint16_t _accept_version{ 0xFFFF };
    BlockOption _cz_block_1{};
    BlockOption _cz_block_2{};
    uint32_t _u32_size_1{};
    uint32_t _u32_size_2{};
    // OptionsList _other_options{};
    std::bitset<2> _options_bit{};
    uint32_t _u32_observe{ 0xFFFFFFFF };
};
}
}