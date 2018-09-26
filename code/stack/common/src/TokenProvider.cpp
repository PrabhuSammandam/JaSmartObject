/*
 * TokenProvider.cpp
 *
 *  Created on: Feb 14, 2018
 *      Author: psammand
 */
#include "common/inc/TokenProvider.h"
#include "OsalRandom.h"

using namespace ja_iot::osal;
using namespace ja_iot::network;

namespace ja_iot {
namespace stack {
constexpr uint8_t TOKEN_LENGTH = 8;

void TokenProvider::assign_next_token( CoapMsg &rcz_coap_msg )
{
  auto &rcz_msg_token = rcz_coap_msg.get_token();

  if( rcz_msg_token.get_length() == 0 )
  {
    int8_t ai8_token_buf[TOKEN_LENGTH];

    OsalRandom::get_random_bytes( (uint8_t *) &ai8_token_buf[0], TOKEN_LENGTH );
    rcz_msg_token.set_token( &ai8_token_buf[0], TOKEN_LENGTH );
  }
}
}
}