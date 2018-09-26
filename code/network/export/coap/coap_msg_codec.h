/*
 * coap_msg_codec.h
 *
 *  Created on: Sep 27, 2017
 *      Author: psammand
 */

#pragma once

#include <cstdint>
#include <functional>
#include "common_defs.h"
#include "ErrCode.h"
#include "coap/coap_msg.h"

namespace ja_iot {
namespace network {
typedef std::function<bool ( uint16_t option_no )> OptionsCallback;

class CoapMsgCodec
{
  public:
    static base::ErrCode parse_coap_header( _in_ const uint8_t *pu8_coap_pdu, _in_ const uint16_t u16_coap_pdu_len, _out_ CoapMsgHdr &rcz_coap_msg_hdr );

    static base::ErrCode decode_coap_msg( _in_ const uint8_t *pu8_coap_pdu, _in_ const uint16_t u16_coap_pdu_len, _out_ CoapMsg &pcz_coap_msg, OptionsCallback &options_callback );

    static base::ErrCode encode_coap_msg( _in_ CoapMsg &rcz_coap_msg, uint8_t *pu8_coap_pdu, uint16_t &ru16_coap_pdu_len );

    static base::ErrCode encode_empty_coap_msg(uint8_t msg_type, uint16_t msg_id, uint8_t *pu8_coap_pdu, uint16_t &ru16_coap_pdu_len);
};
}
}
