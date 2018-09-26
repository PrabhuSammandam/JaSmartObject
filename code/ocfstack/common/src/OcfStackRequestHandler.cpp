#include <ErrCode.h>
#include "OcfStackRequestHandler.h"
#include "OcfStackImpl.h"
#include <memory>
#include "OcfRequest.h"

namespace ja_iot {
namespace ocfstack {
using namespace base;
using namespace network;

OcfRequest*                     create_res_request( RequestPacket *pcz_request_packet );
std::unique_ptr<ResponsePacket> create_reponse_packet( RequestPacket *pcz_request_packet, const uint16_t response_code, const uint16_t msg_type );

ErrCode OcfStackRequestHandler::process_request( RequestPacket *pcz_request_packet )
{
  if( pcz_request_packet == nullptr )
  {
    return ( ErrCode::INVALID_PARAMS );
  }

  if( !_p_stack_impl->_platform_cfg.is_server_enabled() )
  {
    delete pcz_request_packet;
    return ( ErrCode::OK );
  }

  auto &coap_msg = pcz_request_packet->get_msg();
  auto uri_path  = coap_msg.get_option_set().get_uri_path_string();

  if( uri_path.empty() || ( uri_path.length() > 64 ) )
  {
    delete pcz_request_packet;
    return ( ErrCode::INVALID_URI );
  }

  auto uri_query = coap_msg.get_option_set().get_uri_query_string();

  if( !uri_query.empty() && ( uri_query.length() > 64 ) )
  {
    delete pcz_request_packet;
    return ( ErrCode::INVALID_URI );
  }

  if( ( coap_msg.get_code() != COAP_MSG_CODE_GET )
    && ( coap_msg.get_code() != COAP_MSG_CODE_PUT )
    && ( coap_msg.get_code() != COAP_MSG_CODE_POST )
    && ( coap_msg.get_code() != COAP_MSG_CODE_DEL ) )
  {
    const auto response_packet = create_reponse_packet( pcz_request_packet, COAP_MSG_CODE_BAD_REQUEST_400, coap_msg.get_type() == COAP_MSG_TYPE_CON ? COAP_MSG_TYPE_ACK : COAP_MSG_TYPE_NON );
    _p_stack_impl->send_direct_response( response_packet.get() );
    return ( ErrCode::INVALID_REQUEST );
  }

  auto res_request = create_res_request( pcz_request_packet );

  return ( ErrCode::OK );
}

OcfRequest* create_res_request( RequestPacket *pcz_request_packet )
{
  auto res_request = new OcfRequest{};
  auto &coap_msg   = pcz_request_packet->get_msg();

  res_request->set_type( OcfMsgType( coap_msg.get_type() ) );
  res_request->set_code( OcfMsgCode( coap_msg.get_code() ) );
  res_request->set_id( coap_msg.get_id() );
	OcfMsgToken token{ (const uint8_t)coap_msg.get_token().get_length(), (uint8_t*)coap_msg.get_token().get_token() };
  res_request->set_token( token  );

  auto &option_set = coap_msg.get_option_set();
  res_request->set_uri( option_set.get_uri_path_string() );
  res_request->get_query_container().parse( option_set.get_uri_querys_list() );

  if( option_set.has_observe() )
  {
    res_request->get_observe_info().set_action( ObserveAction( option_set.get_observe() ) );
  }

  if( coap_msg.get_payload_len() > 0 )
  {
    /* convert cbor payload to representation */
  }

  return ( res_request );
}

std::unique_ptr<ResponsePacket> create_reponse_packet( RequestPacket *pcz_request_packet, const uint16_t response_code, const uint16_t msg_type )
{
  auto response_packet = std::make_unique<ResponsePacket>();

  response_packet->set_endpoint( pcz_request_packet->get_end_point() );
  response_packet->get_msg().get_option_set() = pcz_request_packet->get_msg().get_option_set();
  response_packet->get_msg().set_code( response_code );
  response_packet->get_msg().set_type( msg_type );
  response_packet->get_msg().set_id( pcz_request_packet->get_msg().get_id() );
  response_packet->get_msg().set_token( pcz_request_packet->get_msg().get_token().get_token(), pcz_request_packet->get_msg().get_token().get_length() );

  return ( response_packet );
}
}
}