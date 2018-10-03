/*
 * coap_msg_test.cpp
 *
 *  Created on: Nov 14, 2017
 *      Author: psammand
 */

#include "coap_msg_test.h"
#include "coap/coap_msg.h"
#include "coap/coap_consts.h"
#include "coap/coap_msg_codec.h"

using namespace ja_iot::network;

void test_coap_msg_constructor()
{
  uint8_t token[] = { 0x10, 0x20, 0x30, 0x40 };
  CoapMsg coap_msg{};

  coap_msg.set_type( COAP_MSG_TYPE_CON );
  coap_msg.set_code( COAP_MSG_CODE_EMPTY );
  coap_msg.set_id( 0x0001 );
  coap_msg.set_token( &token[0], 4 );
}

bool options_callback(uint16_t option_no)
{
	return true;
}

void test_coap_msg_encode()
{
  uint8_t token[] = { 0x10, 0x20, 0x30, 0x40 };
  CoapMsg coap_msg{};

  coap_msg.set_type( COAP_MSG_TYPE_CON );
  coap_msg.set_code( COAP_MSG_CODE_EMPTY );
  coap_msg.set_id( 0x0001 );
  coap_msg.set_token( &token[0], 4 );

  coap_msg.get_option_set().set_uri_path_string("a/b/c");
  coap_msg.get_option_set().set_content_format(COAP_CONTENT_FORMAT_CBOR);
  coap_msg.get_option_set().set_accept_format(COAP_CONTENT_FORMAT_CBOR);
  coap_msg.get_option_set().set_observe(1);

  uint8_t* pu8_coap_pdu = new uint8_t[256];
  uint16_t u16_coap_pdu_len{};

  CoapMsgCodec::encode_coap_msg(coap_msg, pu8_coap_pdu, u16_coap_pdu_len);

  CoapMsg coap_msg_decoded{};
  OptionsCallback option_callback = [](uint16_t opt_no)->bool{return true;};
  CoapMsgCodec::decode_coap_msg(pu8_coap_pdu, u16_coap_pdu_len, coap_msg_decoded, option_callback);
}

cute::suite make_suite_coap_msg_test()
{
  cute::suite s{};
  s.push_back( CUTE( test_coap_msg_constructor ) );
  s.push_back( CUTE( test_coap_msg_encode ) );

  return ( s );
}
