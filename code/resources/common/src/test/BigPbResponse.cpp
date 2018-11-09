#ifdef COMPILE_TEST_RESOURCES
/*
 * BigPbResponse.cpp
 *
 *  Created on: Feb 2, 2018
 *      Author: psammand
 */
#include <string.h>
#include "common/inc/test/BigPbResponse.h"
#include "QueryContainer.h"
#include "StackConsts.h"

namespace ja_iot::resources {
using namespace stack;
BigPbResponse::BigPbResponse () : BaseResource{ "/big/pb" }
{
  init();
}

uint8_t BigPbResponse::handle_request( Interaction *interaction )
{
  auto request = interaction->get_server_request();

  if( /*(request->get_code() == COAP_MSG_CODE_PUT)
       ||*/( request->get_code() == COAP_MSG_CODE_POST )
    || ( request->get_code() == COAP_MSG_CODE_DEL ) )
  {
    return ( STACK_STATUS_INVALID_METHOD );
  }

  auto response = new ServerResponse{};

  response->set_code( COAP_MSG_CODE_CONTENT_205 );
  response->get_option_set().set_content_format( COAP_CONTENT_FORMAT_PLAIN );

  char *payload     = (char *) "A CoAP server which translates incoming CoAP requests to corresponding HTTP requests which are sent to a backend HTTP server; responses are translated to CoAP and sent over to the CoAP client. crosscoap allows CoAP clients to consume content from an existing HTTP application, without adding specific CoAP functionality to the application itself.";
  auto  dyn_payload = new uint8_t[strlen( payload ) + 1];
  memcpy( dyn_payload, payload, strlen( payload ) );
  dyn_payload[strlen( payload )] = '\0';

  response->set_payload( (uint8_t *) dyn_payload, (uint16_t)strlen( payload ) );

  interaction->set_server_response( response );

  return ( STACK_STATUS_OK );
}

void BigPbResponse::init()
{
  set_property( OCF_RESOURCE_PROP_DISCOVERABLE );
}
}
#endif
