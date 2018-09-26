/*
 * MulticastClientInteraction.cpp
 *
 *  Created on: Feb 27, 2018
 *      Author: psammand
 */
#include "common/inc/MulticastClientInteraction.h"
#include "common/inc/MessageIdProvider.h"
#include "common/inc/TokenProvider.h"
#include "MessageSender.h"

using namespace ja_iot::network;

namespace ja_iot {
namespace stack {
MulticastClientInteraction::MulticastClientInteraction( ClientRequest *client_request ) : _client_request{ client_request }
{
}

void MulticastClientInteraction::send_request()
{
  if( ( _client_request == nullptr ) || ( _client_response_cb == nullptr ) )
  {
    return;
  }

  _client_request->set_type( COAP_MSG_TYPE_NON );

  MessageIdProvider::assign_message_id( *_client_request );
  TokenProvider::assign_next_token( *_client_request );
  MessageSender::send( _client_request );
}
}
}