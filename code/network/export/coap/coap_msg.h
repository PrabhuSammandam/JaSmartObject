/*
 * coap_msg.h
 *
 *  Created on: Sep 29, 2017
 *      Author: psammand
 */

#pragma once

#include <cstdint>
#include <stdio.h>
#include <coap/coap_consts.h>
#include <coap/coap_options_set.h>
#include "end_point.h"

namespace ja_iot {
namespace network {
class CoapMsgHdr
{
  public:
    CoapMsgHdr () : _version{ 1 }, _type{ COAP_MSG_TYPE_NONE }, _token_len{ 0 }, _code{ 0 }, _msg_id{ 0 }
    {}

  public:
    uint32_t   _version   : 1;
    uint32_t   _type      : 3;
    uint32_t   _token_len : 4;
    uint32_t   _code      : 8;
    uint32_t   _msg_id    : 16;
    uint8_t *  _payloadStart  = nullptr;
    uint16_t   _payloadLen    = 0;
    uint8_t    _options_count = 0;

    bool is_request() const { return ( _code > COAP_MSG_CODE_EMPTY && _code <= COAP_MSG_CODE_DEL ); }
    bool is_empty_msg() const { return ( _code == COAP_MSG_CODE_EMPTY ); }
    bool is_confirmable() const { return ( _type == COAP_MSG_TYPE_CON ); }
    bool has_mid() const { return ( _msg_id != 0xFFFF ); }
};

class CoapMsgToken
{
  public:
    CoapMsgToken () {}
    CoapMsgToken( int8_t *p8_token, uint8_t token_len )
    {
      set_token( p8_token, token_len );
    }
    CoapMsgToken( const CoapMsgToken &other )
    {
      *this = other;
    }

    CoapMsgToken & operator = ( const CoapMsgToken &other )
    {
      if( &other != this )
      {
        set_token( (int8_t *) other._p8_token, other._token_len );
      }

      return ( *this );
    }

    void set_token( int8_t *p8_token, uint8_t token_len )
    {
      _token_len = token_len;

      if( p8_token != nullptr )
      {
        for( auto i = 0; i < token_len; i++ )
        {
          _p8_token[i] = p8_token[i];
        }
      }
    }

    int8_t* get_token() const { return ( (int8_t *) &_p8_token[0] ); }
    uint8_t get_length() const { return ( _token_len ); }

    void print()
    {
      for( auto i = 0; i < get_length(); i++ )
      {
        uint8_t byte = get_token()[i];
        printf( "%02X", byte );
      }
    }

  private:
    int8_t    _p8_token[8] {};
    uint8_t   _token_len = 0;
};

bool operator == ( const CoapMsgToken &tkn1, const CoapMsgToken &tkn2 );

class CoapMsg
{
  public:
    CoapMsg () : _version{ 1 }, _type{ COAP_MSG_TYPE_NONE }, _code{ 0 }, _msg_id{ 0 }, _reserved{ 0 }
    {
    }
    CoapMsg( uint8_t msg_type ) : _version{ 1 }, _type{ msg_type }, _code{ 0 }, _msg_id{ 0 }, _reserved{ 0 }
    {
    }
    CoapMsg( const CoapMsg &other ) { *this = other; }
    CoapMsg( CoapMsg &&other ) noexcept{ *this = std::move( other ); }
    ~CoapMsg ()
    {
      if( _pu8_payload != nullptr )
      {
        delete[] _pu8_payload;
        _pu8_payload = nullptr;
      }

      _u16_payload_len = 0;
    }

    CoapMsg & operator = ( const CoapMsg &other );
    CoapMsg & operator = ( CoapMsg &&other ) noexcept;

    uint8_t  get_type() const { return ( _type ); }
    void     set_type( const uint8_t type ) { _type = type; }
    uint8_t  get_code() const { return ( _code ); }
    void     set_code( const uint8_t code ) { _code = code; }
    uint16_t get_id() const { return ( _msg_id ); }
    void     set_id( const uint16_t id ) { _msg_id = id; }

    bool     has_payload() const { return ( _u16_payload_len > 0 && _pu8_payload != nullptr ); }
    uint16_t get_payload_len() const { return ( _u16_payload_len ); }
    uint8_t* get_payload() const { return ( _pu8_payload ); }

    void set_payload( uint8_t *pu8_payload, const uint16_t u16_payloadLen )
    {
      _pu8_payload     = pu8_payload;
      _u16_payload_len = u16_payloadLen;
    }

    CoapMsgToken& get_token() { return ( _token ); }
    void        set_token( CoapMsgToken &token ) { _token = token; }
    void        set_token( int8_t *pu8_token, const uint8_t u8_token_len ) { _token.set_token( pu8_token, u8_token_len ); }

    CoapOptionsSet& get_option_set() { return ( _coap_options_set ); }
    Endpoint      & get_endpoint() { return ( _endpoint ); }
    void          set_endpoint( const Endpoint &endpoint ) { _endpoint = endpoint; }

    bool is_request() const { return ( _code > COAP_MSG_CODE_EMPTY && _code <= COAP_MSG_CODE_DEL ); }
    bool is_empty_msg() const { return ( _code == COAP_MSG_CODE_EMPTY ); }
    bool is_confirmable() const { return ( _type == COAP_MSG_TYPE_CON ); }
    bool is_non_confirmable() const { return ( _type == COAP_MSG_TYPE_NON ); }
    bool is_ack() const { return ( _type == COAP_MSG_TYPE_ACK ); }
    bool is_rst() const { return ( _type == COAP_MSG_TYPE_RST ); }
    bool has_mid() const { return ( _msg_id != 0xFFFF ); }
    bool has_block_option();
    bool is_multicast() { return ( _endpoint.is_multicast() ); }
    bool is_valid_request_code() { return ( !( _code != COAP_MSG_CODE_GET && _code != COAP_MSG_CODE_PUT && _code != COAP_MSG_CODE_POST && _code != COAP_MSG_CODE_DEL ) ); }

    CoapMsg* create_msg();
    CoapMsg* create_msg( uint8_t code );
    CoapMsg* create_msg( uint8_t type, uint8_t code );
    CoapMsg* create_msg_with_options();
    CoapMsg* create_msg_with_options( uint8_t code );
    CoapMsg* create_msg_with_options( uint8_t type, uint8_t code );
    void     copy_with_options( CoapMsg *src_msg );

  private:
    uint32_t   _version  : 2;
    uint32_t   _type     : 3;
    uint32_t   _code     : 8;
    uint32_t   _msg_id   : 16;
    uint32_t   _reserved : 3;

    uint8_t *_pu8_payload{};
    uint16_t _u16_payload_len{};
    CoapOptionsSet _coap_options_set{};
    Endpoint _endpoint{};
    CoapMsgToken _token{};
};
}
}
