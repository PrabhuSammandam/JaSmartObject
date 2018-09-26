/*
 * MessageIdProvider.h
 *
 *  Created on: Feb 14, 2018
 *      Author: psammand
 */

#pragma once

#include <coap/coap_msg.h>
#include <cstdint>

namespace ja_iot {
namespace stack {
class MessageIdProvider
{
  public:
    static uint16_t get_next_id();
    static void     assign_message_id( ja_iot::network::CoapMsg &rcz_coap_msg );

  private:
    static uint16_t   _u16_message_id_counter;
};
}
}