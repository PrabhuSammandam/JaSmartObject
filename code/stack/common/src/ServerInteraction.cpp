/*
 * ServerInteraction.cpp
 *
 *  Created on: Feb 14, 2018
 *      Author: psammand
 */
#include <string.h>
#include <algorithm>
#include "base_utils.h"
#include "common/inc/BaseInteraction.h"
#include "common/inc/ServerInteraction.h"
#include "common/inc/InteractionStore.h"
#include "common/inc/BlockTransferStatus.h"
#include "Exchange.h"
#include "MessageSender.h"
#include "StackConfig.h"
#include "StackConsts.h"
#include "ResourceMgr.h"
#include "common/inc/MessageIdProvider.h"

using namespace ja_iot::base;
using namespace ja_iot::network;

namespace ja_iot {
namespace stack {
ServerResponse* create_server_response_with_code( ServerRequest *pcz_server_request, uint8_t code );

constexpr uint8_t BLOCK1_ERROR_BIG_BLOCK_SIZE    = 0;
constexpr uint8_t BLOCK1_ERROR_MAX_RES_BODY_SIZE = 1;
constexpr uint8_t BLOCK1_ERROR_INVALID_BLOCK_NO  = 2;
ServerInteraction::ServerInteraction( CoapMsgToken &rcz_token, Endpoint &rcz_endpoint ) :
  _token{ rcz_token }, _endpoint{ rcz_endpoint }
{
}
ServerInteraction::~ServerInteraction ()
{
  delete_and_clear<ServerRequest>( _server_request );
  delete_and_clear<ServerResponse>( _server_response );
}

void ServerInteraction::add_exchange( Exchange *pcz_new_exchange )
{
  if( pcz_new_exchange != nullptr )
  {
    pcz_new_exchange->set_interaction( this );
    InteractionStore::inst().add_server_exchange( pcz_new_exchange );
    _no_of_exchanges++;
  }
}

void ServerInteraction::delete_exchange( Exchange *pcz_exchange )
{
  BaseInteraction::delete_exchange( pcz_exchange );

  if( InteractionStore::inst().delete_server_exchange( pcz_exchange ) )
  {
    _no_of_exchanges--;
  }
}

CoapMsg * ServerInteraction::get_next_res_block( CoapMsg *pcz_server_response, bool &rb_has_more )
{
  rb_has_more = false;
  auto pcz_res_blk_status     = get_response_block_transfer_status();
  auto pcz_new_response_block = new CoapMsg{};

  pcz_new_response_block->set_code( pcz_server_response->get_code() );
  pcz_new_response_block->get_option_set() = pcz_server_response->get_option_set();

  auto u32_cur_block_no = pcz_res_blk_status->get_current_block_no();
  auto u16_payload_size = pcz_server_response->get_payload_len();
  auto u32_from         = u32_cur_block_no * pcz_res_blk_status->get_block_size();

  if( ( 0 < u16_payload_size ) && ( u32_from < u16_payload_size ) )
  {
    int  u32_to     = std::min( (const uint16_t) ( ( u32_cur_block_no + 1 ) * pcz_res_blk_status->get_block_size() ), (const uint16_t) u16_payload_size );
    int  u32_length = u32_to - u32_from;
    rb_has_more = u32_to < u16_payload_size;

    auto pau8_payload      = new uint8_t[u32_length];
    auto pu8_response_body = pcz_server_response->get_payload();
    memcpy( pau8_payload, &pu8_response_body[u32_from], u32_length );

    pcz_new_response_block->set_payload( pau8_payload, u32_length );
    pcz_new_response_block->get_option_set().set_size2( u16_payload_size ).set_block2( pcz_res_blk_status->get_szx(), rb_has_more, u32_cur_block_no );
  }
  else
  {
    pcz_new_response_block->get_option_set().set_block2( pcz_res_blk_status->get_szx(), false, u32_cur_block_no );
    rb_has_more = false;
  }

  return ( pcz_new_response_block );
}

/**
 * Entry API for server interaction.
 *
 * @param pcz_new_request - newly received coap message
 * @param is_ongoing      - is it ongoing interaction
 */
void ServerInteraction::receive_request( CoapMsg *pcz_new_request, bool is_ongoing )
{
  /*
   * Following are the cases where ongoing interaction will happen
   *
   * 1. Blockwise request.
   * 2. Blockwise response.
   * 3. Separare CON response for request.
   * 4. Observe response notification.
   */

  /* we got some new message for the previous interaction,
   * so don't need of the previous send and received exchanges. */
  delete_current_in_exchange();
  delete_current_out_exchange();

  /* mark the new message by creating new exchange */
  create_incoming_exchange( pcz_new_request );

  if( pcz_new_request->get_option_set().has_block1() )
  {
    /* blockwise request ongoing */
    handle_request_download();
  }
  /* check whether the server response is already generated and also the response it sending by blockwise */
  else if( ( _server_response != nullptr ) && pcz_new_request->get_option_set().has_block2() )
  {
    /* blockwise response ongoing */
    auto &rcz_block2_option = pcz_new_request->get_option_set().get_block2();
    auto pcz_res_blk_status = get_response_block_transfer_status();

    if( pcz_res_blk_status == nullptr )
    {
      pcz_res_blk_status = create_response_block_transfer_status( _server_response, false );
    }

    pcz_res_blk_status->set_current_block_no( rcz_block2_option.get_num() );
    pcz_res_blk_status->set_block_size( rcz_block2_option.get_size() );

    auto rb_has_more       = false;
    auto pcz_next_response = get_next_res_block( _server_response, rb_has_more );
    send_response( pcz_next_response );
  }
  else
  {
    /* check for early block2 negotiation (client requested blockwise response) */
    if( pcz_new_request->get_option_set().has_block2() )
    {
      auto &block2_option     = pcz_new_request->get_option_set().get_block2();
      auto pcz_res_blk_status = create_response_block_transfer_status( nullptr, false );
      pcz_res_blk_status->set_current_block_no( block2_option.get_num() );
      pcz_res_blk_status->set_block_size( block2_option.get_size() );
    }

    /* at this stage we got the full request message, pass it to the stack */
    _server_request = new ServerRequest{ pcz_new_request };

    handle_fully_downloaded_request();
  }
}

void ServerInteraction::send_block1_error_response( CoapMsg *pcz_request, uint8_t u8_block1_error )
{
  auto pcz_error_response = new CoapMsg{};

  switch( u8_block1_error )
  {
    case BLOCK1_ERROR_BIG_BLOCK_SIZE:
    {
      pcz_error_response->set_code( COAP_MSG_CODE_REQUEST_ENTITY_TOO_LARGE_413 );
      pcz_error_response->get_option_set().set_block1( BlockOption::size_to_szx( PREFERRED_BLOCK_SIZE ), 0, 0 );
      pcz_error_response->get_option_set().remove_size1();
      send_response( pcz_error_response );
    }
    break;
    case BLOCK1_ERROR_MAX_RES_BODY_SIZE:
    {
      pcz_error_response->set_code( COAP_MSG_CODE_REQUEST_ENTITY_TOO_LARGE_413 );
      pcz_error_response->get_option_set().set_size1( MAX_RESOURCE_BODY_SIZE );
      pcz_error_response->get_option_set().remove_block1();
      send_response( pcz_error_response );
    }
    break;
    case BLOCK1_ERROR_INVALID_BLOCK_NO:
    {
      pcz_error_response->set_code( COAP_MSG_CODE_REQUEST_ENTITY_INCOMPLETE_408 );
      pcz_error_response->get_option_set().set_block1( pcz_request->get_option_set().get_block1() );
    }
    break;
  }
}

void ServerInteraction::handle_request_download()
{
  auto pcz_in_exchange    = get_in_exchange();
  auto in_message         = pcz_in_exchange->get_originator_msg();
  auto &rcz_options_set   = in_message->get_option_set();
  auto &rcz_block1_option = rcz_options_set.get_block1();

  /* a 4.13 response with a smaller SZX in its Block1 Option than requested is a hint to try a smaller SZX.) */
  if( rcz_block1_option.get_size() > PREFERRED_BLOCK_SIZE )
  {
    send_block1_error_response( in_message, BLOCK1_ERROR_BIG_BLOCK_SIZE );
    return;
  }

  /* The error code 4.13 (Request Entity Too Large) can be returned at any time by a server that does not currently
   * have the resources to store blocks for a block-wise request payload transfer that it would intend to implement
   * in an atomic fashion. */
  if( rcz_options_set.has_size1() && ( rcz_options_set.get_size1() > MAX_RESOURCE_BODY_SIZE ) )
  {
    send_block1_error_response( in_message, BLOCK1_ERROR_MAX_RES_BODY_SIZE );
    return;
  }

  if( ( get_request_block_transfer_status() == nullptr )
    && ( create_request_block_transfer_status( in_message, true ) == nullptr ) )
  {
    /* no memory to allocate */
    send_block1_error_response( in_message, BLOCK1_ERROR_MAX_RES_BODY_SIZE );
    return;
  }

  auto pcz_req_blk_status = get_request_block_transfer_status();
  auto u32_block_no       = rcz_block1_option.get_num();
  auto has_more           = rcz_block1_option.has_more();

  if( ( u32_block_no == 0 ) && ( pcz_req_blk_status->get_next_block_no() > 0 ) )
  {
    /* client restarted the blockwise request transfer */

    /* clear the partially downloaded request */
    pcz_req_blk_status->clear();
    delete_request_block_transfer_status();
    pcz_req_blk_status = create_request_block_transfer_status( in_message, true );

    if( pcz_req_blk_status == nullptr )
    {
      send_block1_error_response( in_message, BLOCK1_ERROR_MAX_RES_BODY_SIZE );
      return;
    }
  }

  if( u32_block_no != pcz_req_blk_status->get_next_block_no() )
  {
    /* invalid block option - send error message */
    send_block1_error_response( in_message, BLOCK1_ERROR_INVALID_BLOCK_NO );
    return;
  }

  pcz_req_blk_status->add_payload( (int8_t *) in_message->get_payload(), in_message->get_payload_len() );
  pcz_req_blk_status->move_to_next_block();

  if( has_more )
  {
    auto pcz_temp_server_response = new CoapMsg{};
    pcz_temp_server_response->set_code( COAP_MSG_CODE_CONTINUE_231 );
    pcz_temp_server_response->get_option_set().set_block1( rcz_options_set.get_block1() );
    send_response( pcz_temp_server_response );
  }
  else
  {
    /* no more blocks to download and start processing the request */
    /* check for early block2 negotiation */
    if( in_message->get_option_set().has_block2() )
    {
      auto &rcz_block2_option      = in_message->get_option_set().get_block2();
      auto pcz_temp_res_blk_status = create_response_block_transfer_status( nullptr, false );
      pcz_temp_res_blk_status->set_current_block_no( rcz_block2_option.get_num() );
      pcz_temp_res_blk_status->set_block_size( rcz_block2_option.get_size() );
    }

    _server_request = new ServerRequest{ in_message };
    _server_request->set_payload( (uint8_t *) pcz_req_blk_status->get_payload_buffer(), pcz_req_blk_status->get_payload_length() );

    handle_fully_downloaded_request();
  }
}

/* this API checks if the response is greater than configure MESSAGE_SIZE and if it
 * exceeds then calculate the first block and return.
 *
 * If the response not exceeds the max size then the passed response is returned.
 *
 * This API should be called only after response is fully generated.
 *
 * This API will not disturb the passed parameter and it creates the new response message.
 */

CoapMsg * ServerInteraction::get_first_response_block( CoapMsg *pcz_response )
{
  CoapMsg *pcz_first_response = nullptr;
  auto     pcz_res_blk_status = get_response_block_transfer_status();

  if( ( pcz_response->get_payload_len() > MAX_MESSAGE_SIZE ) || ( pcz_res_blk_status != nullptr ) )
  {
    if( pcz_res_blk_status == nullptr )
    {
    	/* start the blockwise response transfer */
      pcz_res_blk_status = create_response_block_transfer_status( pcz_response, false );
    }

    auto has_more = false;
    pcz_first_response = get_next_res_block( pcz_response, has_more );

    if( !has_more )
    {
      /* no need of response block status if there are no more blocks */
      delete_response_block_transfer_status();
    }
  }
  else
  {
    pcz_first_response = new CoapMsg{ (CoapMsg &) *pcz_response };
  }

  /* check if the request downloaded in blockwise and it is downloaded blockwise then
   * need to acknowledge the last received request block*/
  auto pcz_req_blk_status = get_request_block_transfer_status();

  if( pcz_req_blk_status != nullptr )
  {
    /*
     * current interaction used the blockwise request transfer, so need to acknowledge
     * the last block of request and also need to include the start of the new BLOCK 2
     */
    pcz_first_response->get_option_set().set_block1( pcz_req_blk_status->get_szx(), false, pcz_req_blk_status->get_current_block_no() );
    delete_request_block_transfer_status();
  }

  return ( pcz_first_response );
}

ServerResponse* create_server_response_with_code( ServerRequest *pcz_server_request, uint8_t u8_msg_code )
{
  auto server_response = new ServerResponse{};

  server_response->set_code( u8_msg_code );
  server_response->set_token( pcz_server_request->get_token() );
  server_response->set_endpoint( pcz_server_request->get_endpoint() );
  /* type and id will be set in the send_response api */

  return ( server_response );
}

/**
 * This API should be called only after the request is fully downloaded.
 */
void ServerInteraction::handle_fully_downloaded_request()
{
  auto pcz_temp_server_request = _server_request;

  auto str_resource_uri_path = pcz_temp_server_request->get_option_set().get_uri_path_string();
  auto pcz_target_resource   = ResourceMgr::inst().find_resource_by_uri( str_resource_uri_path );

  if( pcz_target_resource == nullptr )
  {
    /* no resource found for the request. Send the NOT_FOUND response for the request */
    _server_response = create_server_response_with_code( _server_request, COAP_MSG_CODE_NOT_FOUND_404 );

    auto pcz_reply_message = new CoapMsg{};
    pcz_reply_message->set_code( COAP_MSG_CODE_NOT_FOUND_404 );
    send_response( pcz_reply_message );
    return;
  }

  auto u8_stack_status = pcz_target_resource->handle_request( this );

  if( u8_stack_status == STACK_STATUS_SLOW_RESPONSE )
  {
    /* resource wants some time to produce the response, so just ACK the request */
    acknowledge();
  }
  else
  {
    if( ( u8_stack_status == STACK_STATUS_OK ) && ( _server_response != nullptr ) )
    {
      /*
       * Following fields must be set by resource on request processing
       * 1. CODE
       * 2. PAYLOAD
       * 3. CONTENT-FORMAT
       *
       */
      CoapMsg *pcz_temp_response = get_first_response_block( _server_response );
      send_response( pcz_temp_response );
    }
    else
    {
      /* some error occurred in the request processing, send the appropriate error response */
      uint8_t u8_error_code = COAP_MSG_CODE_BAD_REQUEST_400;

      switch( u8_stack_status )
      {
        case STACK_STATUS_OK:
        case STACK_STATUS_OUT_OF_MEMORY:
        {
          u8_error_code = COAP_MSG_CODE_INTERNAL_SERVER_ERROR_500;
        }
        break;
        case STACK_STATUS_INVALID_INTERFACE_QUERY:
        case STACK_STATUS_INVALID_TYPE_QUERY:
        {
          u8_error_code = COAP_MSG_CODE_BAD_REQUEST_400;
        }
        break;
        case STACK_STATUS_INVALID_METHOD:
        {
          u8_error_code = COAP_MSG_CODE_METHOD_NOT_ALLOWED_405;
        }
        break;
        default:
        {
        }
        break;
      }

      /* send the error response only for the unicast request */
      if( get_server_request()->is_multicast() == false )
      {
        _server_response = create_server_response_with_code( _server_request, u8_error_code );

        auto pcz_temp_server_response = new CoapMsg{};
        pcz_temp_server_response->set_code( u8_error_code );
        send_response( pcz_temp_server_response );
      }
    }
  }
}

/*
 * This API sends the response for the request. The response can be send in three ways as ACK, CON or NON.
 *
 * If it is send by CON or NON, then new exchange need to create and also new message id needs to be assigned.
 */
void ServerInteraction::send_response( CoapMsg *pcz_response_msg )
{
  /* to send response following fields must be set
   * 1. Type        - set from resource or calculated based on request
   * 2. Code        - set from resource
   * 3. ID          - set based on the request
   * 4. Token       - need to set below
   * 5. Endpoint    - need to set below
   */
  auto pcz_originator_msg = _current_in_exchange->get_originator_msg();

  if( pcz_response_msg->get_type() == COAP_MSG_TYPE_NONE )
  {
    /* resource not decided the type so deduct it from request */
    pcz_response_msg->set_type( pcz_originator_msg->is_confirmable() ? COAP_MSG_TYPE_ACK : COAP_MSG_TYPE_NON );
  }

  /* set the same token as request */
  pcz_response_msg->set_token( pcz_originator_msg->get_token() );
  /* set the same endpoint as request */
  pcz_response_msg->set_endpoint( pcz_originator_msg->get_endpoint() );

  if( pcz_response_msg->is_ack() )
  {
    /* this is PIGGY-BACK response */
    _current_in_exchange->reply_piggyback( pcz_response_msg );
  }
  else
  {
    /* this may be CON or NON. if it is CON send ACK for previous received CON request */
    _current_in_exchange->acknowledge();

    /* For CON or NON assign new message id */
    MessageIdProvider::assign_message_id( *pcz_response_msg );

    /* create local exchange */
    auto current_out_exchange = create_outgoing_exchange( pcz_response_msg );
    current_out_exchange->deliver();
  }
}

/*
 * This API used to send the separate response for the previous request (slow response).
 *
 * If the new response is more than the configured message size then blockwise transfer will be
 * started and the first block will be sent.
 */
void ServerInteraction::send_separate_response( ServerResponse *pcz_separate_response_msg )
{
  /* set the final response for the interaction */
  set_server_response( pcz_separate_response_msg );

  /* check for the block wise transfer and if yes get the first block,
   * otherwise the passed message will be used */
  CoapMsg *pcz_temp_response = get_first_response_block( pcz_separate_response_msg );

  if( pcz_temp_response->get_type() == COAP_MSG_TYPE_NONE )
  {
    /* no preference set by server, so for CON request the response type defaulted to CON */
    pcz_temp_response->set_type( get_server_request()->is_confirmable() ? COAP_MSG_TYPE_CON : COAP_MSG_TYPE_NON );
  }

  /* For CON or NON assign new message id */
  MessageIdProvider::assign_message_id( *pcz_temp_response );

  /* create local exchange */
  auto pcz_current_out_exchange = create_outgoing_exchange( pcz_temp_response );
  pcz_current_out_exchange->deliver();
}

bool ServerInteraction::is_matched( CoapMsg *pcz_coap_msg )
{
  return ( _endpoint == pcz_coap_msg->get_endpoint() && _token == pcz_coap_msg->get_token() );
}
}
}
