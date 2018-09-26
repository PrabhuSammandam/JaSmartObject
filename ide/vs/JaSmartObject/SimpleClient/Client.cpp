#include "coap/coap_msg.h"
#include "coap/coap_msg_codec.h"
#include "Client.h"
#include "base_consts.h"
#include "adapter_mgr.h"
#include "StackEvents.h"
#include "MsgStack.h"

using namespace ja_iot::network;
using namespace ja_iot::base;
BaseRequestInfo::BaseRequestInfo( ja_iot::network::Endpoint &endpoint, std::string uri_path, client_response_cb response_cb ) :
  _endpoint{ endpoint }, _uri_path{ uri_path }, _response_cb{ response_cb }
{
}
GetRequestInfo::GetRequestInfo( ja_iot::network::Endpoint &endpoint, std::string uri_path, client_response_cb response_cb ) :
  BaseRequestInfo{ endpoint, uri_path, response_cb }
{
}
PutRequestInfo::PutRequestInfo( ja_iot::network::Endpoint &endpoint, std::string uri_path, client_response_cb response_cb ) :
  BaseRequestInfo{ endpoint, uri_path, response_cb }
{
}

void PutRequestInfo::set_payload(std::string & payload)
{
	if (payload.size() == 0)
	{
		return;
	}

	auto  dyn_payload = new char[payload.size() + 1];
	memcpy(&dyn_payload[0], payload.c_str(), payload.size());
	dyn_payload[payload.size()] = '\0';

	_payload_buf = (int8_t *)dyn_payload;
	_payload_buf_len = payload.size();
}

uint8_t Client::get( GetRequestInfo &get_request_info )
{
  ClientRequest *request = new ClientRequest{};

  request->set_code( COAP_MSG_CODE_GET );
  request->set_type( get_request_info._msg_type );
  request->set_endpoint( get_request_info._endpoint );

  auto &option_set = request->get_option_set();
  option_set.set_uri_path_string( get_request_info._uri_path );
  option_set.set_uri_query_string( get_request_info._uri_query );
  option_set.set_accept_format( get_request_info._accept_format );

  if( get_request_info._block2_size != 0 )
  {
    option_set.set_block2( BlockOption::size_to_szx( get_request_info._block2_size ), false, 0 );
  }

	auto client_send_request_stack_event = new ClientSendRequestStackEvent{request, get_request_info._response_cb};

	MsgStack::inst().send_stack_event(client_send_request_stack_event);

  return ( uint8_t() );
}

uint8_t Client::put(PutRequestInfo & put_request_info)
{
	if (put_request_info._endpoint.is_multicast())
	{
		printf("PUT request will not be send to multicast address\n");
		return 1;
	}

	ClientRequest *request = new ClientRequest{};

	request->set_code(COAP_MSG_CODE_PUT);
	request->set_type(put_request_info._msg_type);
	request->set_endpoint(put_request_info._endpoint);
	request->set_payload((uint8_t*)put_request_info._payload_buf, put_request_info._payload_buf_len);

	auto &option_set = request->get_option_set();
	option_set.set_uri_path_string(put_request_info._uri_path);
	option_set.set_uri_query_string(put_request_info._uri_query);
	option_set.set_accept_format(put_request_info._accept_format);
	option_set.set_content_format(put_request_info._content_format);

	if (put_request_info._block2_size != 0)
	{
		option_set.set_block2(BlockOption::size_to_szx(put_request_info._block2_size), false, 0);
	}

	auto client_send_request_stack_event = new ClientSendRequestStackEvent{ request, put_request_info._response_cb };

	MsgStack::inst().send_stack_event(client_send_request_stack_event);

	return uint8_t();
}

uint8_t Client::post(PostRequestInfo & put_request_info)
{
	if (put_request_info._endpoint.is_multicast())
	{
		printf("POST request will not be send to multicast address\n");
		return 1;
	}

	ClientRequest *request = new ClientRequest{};

	request->set_code(COAP_MSG_CODE_POST);
	request->set_type(put_request_info._msg_type);
	request->set_endpoint(put_request_info._endpoint);
	request->set_payload((uint8_t*)put_request_info._payload_buf, put_request_info._payload_buf_len);

	auto &option_set = request->get_option_set();
	option_set.set_uri_path_string(put_request_info._uri_path);
	option_set.set_uri_query_string(put_request_info._uri_query);
	option_set.set_accept_format(put_request_info._accept_format);
	option_set.set_content_format(put_request_info._content_format);

	if (put_request_info._block2_size != 0)
	{
		option_set.set_block2(BlockOption::size_to_szx(put_request_info._block2_size), false, 0);
	}

	auto client_send_request_stack_event = new ClientSendRequestStackEvent{ request, put_request_info._response_cb };

	MsgStack::inst().send_stack_event(client_send_request_stack_event);

	return uint8_t();
}
