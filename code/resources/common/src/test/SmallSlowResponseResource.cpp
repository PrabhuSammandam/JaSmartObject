#ifdef COMPILE_TEST_RESOURCES
#include <MsgStack.h>
#include <string.h>
#include "common/inc/test/SmallSlowResponseResource.h"
#include "QueryContainer.h"
#include "DeviceInfo.h"
#include "StackConsts.h"
#include "OsalTimer.h"
#include "StackEvents.h"


namespace ja_iot::resources {
using namespace stack;
using namespace osal;
using namespace network;
struct RequestInfo
{
  RequestInfo( CoapMsgToken &token, Endpoint endpoint ) : _token{ token }, _endpoint{ endpoint }
  {
  }

  CoapMsgToken   _token;
  Endpoint       _endpoint;
};

static OsalTimer *timer = nullptr;

static void time_cb( void *user_param1, void *user_param2 )
{
  auto request_info = ( (RequestInfo *) user_param1 );
  auto resource     = (SmallSlowResponseResource *) user_param2;

  auto response = new ServerResponse{};

  response->set_type( COAP_MSG_TYPE_NON );
  response->set_code( COAP_MSG_CODE_CONTENT_205 );
  response->set_token( request_info->_token );
  response->set_endpoint( request_info->_endpoint );
  response->get_option_set().set_uri_path_string( resource->get_uri() );
  response->get_option_set().set_content_format( COAP_CONTENT_FORMAT_PLAIN );

  char *payload     = (char *) "Small Slow Res";
  auto  dyn_payload = new uint8_t[strlen( payload ) + 1];
  memcpy( dyn_payload, payload, strlen( payload ) );
  dyn_payload[strlen( payload )] = '\0';

  response->set_payload( (uint8_t *) dyn_payload, (uint16_t) strlen( payload ) );

  auto send_response_stack_event = new SendServerResponseStackEvent{ response };

  MsgStack::inst().send_stack_event( send_response_stack_event );
}
SmallSlowResponseResource::SmallSlowResponseResource () : BaseResource{ "/small/slow" }
{
  init();
}

uint8_t SmallSlowResponseResource::handle_request( Interaction *interaction )
{
  auto request = interaction->get_server_request();

  if( /* ( request->get_code() == COAP_MSG_CODE_PUT )
       ||*/( request->get_code() == COAP_MSG_CODE_POST )
    || ( request->get_code() == COAP_MSG_CODE_DEL ) )
  {
    return ( STACK_STATUS_INVALID_METHOD );
  }

  auto request_info = new RequestInfo{ request->get_token(), request->get_endpoint() };

  timer = new OsalTimer{ 5000000, time_cb, request_info, this, true };

  timer->start();

  return ( STACK_STATUS_SLOW_RESPONSE );
}

void SmallSlowResponseResource::init()
{
  set_property( OCF_RESOURCE_PROP_DISCOVERABLE );
}
}
#endif
