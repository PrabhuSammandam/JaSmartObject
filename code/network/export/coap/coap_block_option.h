#pragma once

#include <cstdint>

namespace ja_iot {
namespace network {
class BlockOption
{
  uint32_t   _len : 8;
  uint32_t   _num : 20;
  uint32_t   _m   : 1;
  uint32_t   _szx : 3;

  public:
    BlockOption () : _len{}, _num{}, _m{}, _szx{}
    {
    }
    BlockOption( const uint8_t szx, const bool m, const uint32_t num ) : _len{}, _num{ num }, _m{ m }, _szx{ szx }
    {
    }

    uint8_t get_szx() const { return ( _szx ); }
    void    set_szx( const uint8_t szx ) { this->_szx = szx; }

    uint16_t get_size() const { return ( 1 << ( 4 + get_szx() ) ); }
    void     set_size( const uint16_t size ) { set_szx( size_to_szx( size ) ); }

    bool has_more() const { return ( this->_m ); }
    void set_more( const bool set_value ) { this->_m = set_value; }

    uint32_t get_num() const { return ( this->_num ); }
    void     set_num( const uint32_t num ) { this->_num = num; }

    uint8_t get_len() const { return ( this->_len ); }
    void    set_len( const uint8_t len ) { this->_len = len; }

    void clear()
    {
      _num = 0;
      _m   = false;
      _szx = 0;
      _len = 0;
    }

    // bool is_valid() { return ( !( _num == 0 && _szx == 0 ) && _m == 0 && _len == 0 ); }

    void decode( uint8_t *buf, uint8_t buf_len );
    void encode( uint8_t *buf, uint8_t &buf_len ) const;

    static uint8_t size_to_szx( const uint16_t size )
    {
      if( size <= 16 ){ return ( 0 ); }

      if( ( size >= 32 ) && ( size < 64 ) ){ return ( 1 ); }

      if( ( size >= 64 ) && ( size < 128 ) ){ return ( 2 ); }

      if( ( size >= 128 ) && ( size < 256 ) ){ return ( 3 ); }

      if( ( size >= 256 ) && ( size < 512 ) ){ return ( 4 ); }

      if( ( size >= 512 ) && ( size < 1024 ) ){ return ( 5 ); }

      return ( 6 );
    }

    static uint16_t szx_to_size( const uint8_t szx )
    {
      if( szx <= 0 ){ return ( 16 ); }

      if( szx >= 6 ){ return ( 1024 ); }

      return ( 1 << ( szx + 4 ) );
    }
};
}
}