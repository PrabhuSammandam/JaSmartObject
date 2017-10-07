/*
 * coap_msg.h
 *
 *  Created on: Sep 29, 2017
 *      Author: psammand
 */

#pragma once

#include <coap/coap_option_set.h>
#include <common_defs.h>
#include <data_types.h>
#include <ErrCode.h>
#include <cstdint>

namespace ja_iot {
namespace network {
using ErrCode = ja_iot::base::ErrCode;

class CoapMsgHdr
{
  public:
    CoapMsgType   _type          = CoapMsgType::RST;
    CoapMsgCode   _code          = CoapMsgCode::EMPTY;
    uint16_t      _id            = -1;
    ErrCode       _errCode       = ErrCode::OK;
    uint8_t *     _payloadStart  = nullptr;
    uint16_t      _payloadLen    = 0;
    uint8_t       _options_count = 0;

    bool is_request() const { return ( _code > CoapMsgCode::EMPTY && _code <= CoapMsgCode::DEL ); }
};

class CoapMsg
{
  public:
    CoapMsgCode get_code() const { return ( _code ); }
    void        set_code( CoapMsgCode code ) { _code = code; }

    uint16_t get_id() const { return ( _u16_msg_id ); }
    void     set_id( uint16_t id ) { _u16_msg_id = id; }

    uint16_t get_payload_len() const { return ( _u16_payloadLen ); }
    void     set_payload_len( uint16_t payloadLen ) { _u16_payloadLen = payloadLen; }

    uint8_t* get_payload() const { return ( _pu8_payload ); }
    void     set_payload( uint8_t *payload ) { _pu8_payload = payload; }

    uint8_t* get_token() { return ( &_pu8_token[0] ); }
    void     set_token( uint8_t *pu8_token, uint8_t u8_token_len ) { for( int i = 0; i < u8_token_len; i++ ){ _pu8_token[i] = pu8_token[i]; } }

    uint8_t get_token_len() const { return ( _u8_token_len ); }
    void    set_token_len( uint8_t tokenLen ) { _u8_token_len = tokenLen; }

    CoapMsgType get_type() const { return ( _type ); }
    void        set_type( CoapMsgType type ) { _type = type; }

    OptionsSet* get_option_set() { return ( &_option_set ); }

    ErrCode create_coap_msg( _in_ CoapMsgHdr *pcz_coap_msg_hdr, _in_ uint8_t *pu8_coap_pdu, _in_ uint16_t u16_coap_pdu_len );

  private:
    CoapMsgType   _type           = CoapMsgType::RST;
    CoapMsgCode   _code           = CoapMsgCode::EMPTY;
    uint16_t      _u16_msg_id     = -1;
    uint8_t *     _pu8_payload    = nullptr;
    uint16_t      _u16_payloadLen = 0;
    uint8_t       _pu8_token[8];
    uint8_t       _u8_token_len        = 0;
    uint8_t *     _pu8_options_start   = nullptr;
    uint8_t       _u8_options_data_len = 0;
    OptionsSet    _option_set;
};
}
}
