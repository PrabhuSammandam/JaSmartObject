/*
 * coap_msg_codec.h
 *
 *  Created on: Sep 27, 2017
 *      Author: psammand
 */

#pragma once

#include <common_defs.h>
#include <ErrCode.h>
#include <cstdint>
#include <coap/coap_msg.h>

namespace ja_iot {
namespace network {
using ErrCode = ja_iot::base::ErrCode;


class CoapMsgCodec
{
  public:
    static ErrCode parse_header( _in_ const uint8_t *pu8_coap_pdu, _in_ const uint16_t u16_coap_pdu_len, _out_ CoapMsgHdr *pcz_coap_msg_hdr );
    static ErrCode check_options( _in_ const uint8_t *pu8_options_data_start, _in_ uint16_t u16_options_data_len, _out_ uint8_t **pp_payload_start, _out_ uint8_t *pu8_options_count );

    static ErrCode parse_coap_msg( _in_ const uint8_t *pu8_coap_pdu, _in_ const uint16_t u16_coap_pdu_len, _out_ CoapMsg *pcz_coap_msg );
    static ErrCode parse_coap_options( _in_out_ CoapMsg *pcz_coap_msg, _in_ const uint8_t *pu8_options_start, _in_ const uint16_t u16_options_data_len, _out_ uint8_t **pp_payload_start );
};
}
}
