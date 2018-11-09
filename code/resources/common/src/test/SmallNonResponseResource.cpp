#ifdef COMPILE_TEST_RESOURCES
/*
 * SmallNonResponseResource.cpp
 *
 *  Created on: Feb 2, 2018
 *      Author: psammand
 */
#include <MsgStack.h>
#include <string.h>
#include "common/inc/test/SmallNonResponseResource.h"
#include "QueryContainer.h"
#include "DeviceInfo.h"
#include "StackConsts.h"
#include "StackEvents.h"

namespace ja_iot::resources {
using namespace stack;
SmallNonResponseResource::SmallNonResponseResource () : BaseResource{ "/small/non" }
{
  init();
}

uint8_t SmallNonResponseResource::handle_request( Interaction *interaction )
{
  auto request = interaction->get_server_request();

  if( /*( request->get_code() == COAP_MSG_CODE_PUT )
    ||*/ ( request->get_code() == COAP_MSG_CODE_POST )
    || ( request->get_code() == COAP_MSG_CODE_DEL ) )
  {
    return ( STACK_STATUS_INVALID_METHOD );
  }

  auto response = new ServerResponse{};

  response->set_type( COAP_MSG_TYPE_NON );
  response->set_code( COAP_MSG_CODE_CONTENT_205 );
  response->get_option_set().set_content_format( COAP_CONTENT_FORMAT_PLAIN );

  char *payload     = (char *) "Small NON res";
  auto  dyn_payload = new uint8_t[strlen( payload ) + 1];
  memcpy( dyn_payload, payload, strlen( payload ) );
  dyn_payload[strlen( payload )] = '\0';

  response->set_payload( (uint8_t *) dyn_payload, (uint16_t)strlen( payload ) );
  interaction->set_server_response(response);

  return ( STACK_STATUS_OK);
}

void SmallNonResponseResource::init()
{
  set_property( OCF_RESOURCE_PROP_DISCOVERABLE );
}
}
#endif
