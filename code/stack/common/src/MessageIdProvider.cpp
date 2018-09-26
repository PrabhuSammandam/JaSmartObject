/*
 * MessageIdProvider.cpp
 *
 *  Created on: Feb 14, 2018
 *      Author: psammand
 */

#include "common/inc/MessageIdProvider.h"
#include "OsalRandom.h"

using namespace ja_iot::osal;
using namespace ja_iot::network;

namespace ja_iot {
namespace stack {
uint16_t MessageIdProvider::_u16_message_id_counter = 0;

uint16_t MessageIdProvider::get_next_id()
{
  if( _u16_message_id_counter == 0 )
  {
    _u16_message_id_counter = (uint16_t) OsalRandom::get_random();
  }

  return ( _u16_message_id_counter++ );
}

void MessageIdProvider::assign_message_id( CoapMsg &rcz_coap_msg )
{
  if( rcz_coap_msg.get_id() == 0 )
  {
    rcz_coap_msg.set_id( get_next_id() );
  }
}
}
}