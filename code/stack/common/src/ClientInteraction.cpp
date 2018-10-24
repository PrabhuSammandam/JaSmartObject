/*
 * ClientInteraction.cpp
 *
 *  Created on: Feb 14, 2018
 *      Author: psammand
 */

#include <string.h>
#include "common/inc/ClientInteraction.h"
#include "common/inc/InteractionStore.h"
#include "Exchange.h"
#include "common/inc/MessageIdProvider.h"
#include "common/inc/TokenProvider.h"
#include "coap/coap_msg.h"
#include "StackConfig.h"
#include "StackConsts.h"
#include "common/inc/BlockTransferStatus.h"

using namespace ja_iot::base;
using namespace ja_iot::network;

namespace ja_iot {
namespace stack {
ClientInteraction::ClientInteraction( ClientRequest *client_request ) : _client_request{ client_request }
{
}
ClientInteraction::ClientInteraction( ClientRequest *client_request, client_response_cb response_cb )
  : _client_request{ client_request }, _client_response_cb{ response_cb }
{
}
ClientInteraction::~ClientInteraction ()
{
}

void ClientInteraction::send_request()
{
  if( ( _client_request == nullptr ) || ( _client_response_cb == nullptr ) )
  {
    return;
  }

  MessageIdProvider::assign_message_id( *_client_request );// assign message id for new message
  TokenProvider::assign_next_token( *_client_request );// assign token id for new interaction

  set_token( _client_request->get_token() );// update this interaction with request token
  set_endpoint( _client_request->get_endpoint() );// update this interaction with the request endpoint

  CoapMsg *coap_request_msg = nullptr;

  auto     is_post_request       = _client_request->get_code() == COAP_MSG_CODE_POST;
  auto     is_put_request        = _client_request->get_code() == COAP_MSG_CODE_PUT;
  auto     is_blockwise_required = _client_request->get_payload_len() > MAX_MESSAGE_SIZE;

  /* check whether the client request exceeds the max configured message size and it usually comes
   * only for POST or PUT request */
  if( ( is_post_request || is_put_request ) && is_blockwise_required )
  {
    /* send the request in blockwise */
    auto req_transfer_status = create_request_block_transfer_status( _client_request, false );
    coap_request_msg = client_blockwise_get_next_request_block( _client_request, req_transfer_status );
  }
  else
  {
    /* no blockwise transfer of request */
    coap_request_msg = new CoapMsg{ *( (CoapMsg *) _client_request ) };
  }

  /* to send request, following fields must be set
   * 1. Type        - set from resource or calculated based on request
   * 2. Code        - set from resource
   * 3. ID          - set based on the request
   * 4. Token       - need to set below
   * 5. Endpoint    - need to set below
   */

  send_single_request_msg( coap_request_msg );
}

/*
 * The client can receive response for following reasons
 *
 * 1. ACK for the CON request (with both block1 and block2)[possible empty ACK][possible retransmission]
 *              delete old exchange
 *              if (no block options)
 *              {
 *                      deliver response.
 *              }
 *              else
 *              {
 *                      send CON request for further blocks.
 *                      set as current exchange
 *              }
 * 2. CON for the NON request (with both block1 and block2)[possible duplicate response]
 *              1. Acknowledge for the CON response
 *              2. Add this exchange [for deduplication or RST]
 *              3. delete old exchange. [old exchange is NON and there is no deduplication]
 *
 *              if (no block options)
 *              {
 *                      deliver response.
 *              }
 *              else
 *              {
 *                      send NON request for further blocks.
 *              }
 *
 * 3. CON for the CON request (with both block1 and block2)[possible empty ACK][possible duplicate response] [possible retransmission]
 *              1. Acknowledge for the CON response
 *              2. Add this exchange. [for deduplication or RST]
 *              3. delete old exchange. [required response got, no need of this exchange]
 *              if (no block options)
 *              {
 *                      deliver response.
 *              }
 *              else
 *              {
 *                      send CON request for further blocks.
 *              }
 * 4. NON for the NON request (with both block1 and block2)
 *              1. add this axchange. [for RST from server]
 *              2. delete old exchange. [old exchange is NON and there is no deduplication]
 *              if (no block options)
 *              {
 *                      deliver response.
 *              }
 *              else
 *              {
 *                      send NON request for further blocks.
 *              }
 *
 * 5. NON for the CON request (with both block1 and block2)[possible empty ACK][possible retransmission]
 *              1. add this exchange. [for RST from server]
 *              2. delete old exchange. [required response got, no need of this exchange]
 *              if (no block options)
 *              {
 *                      deliver response.
 *              }
 *              else
 *              {
 *                      send CON request for further blocks.
 *              }
 *
 *
 *              1. delete old exchange
 *
 *              if(received msg is CON or NON)
 *              {
 *                      add_exchange
 *
 *                      if(received msg is CON)
 *                      {
 *                              acknowledge
 *                      }
 *              }
 *
 *              if (no block options)
 *              {
 *                      deliver response.
 *              }
 *              else
 *              {
 *                      if(ACK response || CON request)
 *                              send CON request for further blocks.
 *                      else
 *                              send NON request for further blocks.
 *              }
 *
 *
 *              <---------- CON block2 6th block
 *              ----------> ACK
 *
 *              ----------> CON block2 7th block
 *
 *              <---------- CON block2 6th block [retransmission]
 *              ----------> ACK
 *
 *              <---------- CON block1 CONTINUE 1st block
 *              ----------> ACK
 *
 *              ----------> CON block1 2nd block
 *
 *              <---------- CON block1 CONTINUE 1st block [retransmission]
 *              ----------> ACK
 *
 */

void ClientInteraction::receive_response( CoapMsg *pcz_response )
{
  /* delete old exchange */
  delete_current_out_exchange();

  if( pcz_response->is_confirmable() || pcz_response->is_non_confirmable() )
  {
    /* an incoming CON or NON exchange is meant to be that it is a reply for previous message
     * and also whatever send previous also not valid */
    delete_current_in_exchange();

    /* mark the new received message by creating exchange */
    create_incoming_exchange( pcz_response );

    if( pcz_response->is_confirmable() )
    {
      acknowledge();
    }
  }

  if( pcz_response->has_block_option() == false )
  {
    /* received single complete message */
    auto client_response = new ClientResponse{ pcz_response };
    notify_response( client_response, CLIENT_RESPONSE_STATUS_OK );
  }
  else
  {
    if( pcz_response->get_option_set().has_block1() )
    {
      client_handle_block1_transfer( pcz_response );
    }

    if( pcz_response->get_option_set().has_block2() )
    {
      client_handle_block2_transfer( pcz_response );
    }
  }
}

void ClientInteraction::notify_response( ClientResponse *pcz_response, uint8_t u8_response_status )
{
  _client_response = pcz_response;

  if( _client_response_cb != nullptr )
  {
    _client_response_cb( pcz_response, u8_response_status );
  }
}

void ClientInteraction::add_exchange( Exchange *pcz_new_exchange )
{
  if( pcz_new_exchange != nullptr )
  {
    pcz_new_exchange->set_interaction( this );
    InteractionStore::inst().add_client_exchange( pcz_new_exchange );
    _no_of_exchanges++;
  }
}

void ClientInteraction::delete_exchange( Exchange *pcz_old_exchange )
{
  BaseInteraction::delete_exchange( pcz_old_exchange );

  if( InteractionStore::inst().delete_client_exchange( pcz_old_exchange ) )
  {
    _no_of_exchanges--;
  }
}

void ClientInteraction::client_handle_block1_transfer( CoapMsg *pcz_response_msg )
{
  auto pcz_req_block_status = get_request_block_transfer_status();

  if( pcz_req_block_status == nullptr )
  {
	  /* blockwise request transfer not found */
    delete pcz_response_msg;
    return;
  }

  /* possible cases for the block1 handling
   *
   * 1. Server sends reply that it is not able to download the request, since it is too large.
   * 2. Servers sends new block size for the transfer.
   * 3. No block2 option and it means final response and send it too upper layer.
   */

  if( pcz_response_msg->get_code() == COAP_MSG_CODE_CONTINUE_231 )
  {
    auto u16_current_size   = pcz_req_block_status->get_block_size();
    auto u16_new_block_size = u16_current_size;

    /* check if the block size changed */
    if( pcz_response_msg->get_option_set().get_block1().get_size() < u16_current_size )
    {
      u16_new_block_size = pcz_response_msg->get_option_set().get_block1().get_size();
    }

    auto u32_next_block_no = pcz_req_block_status->get_current_block_no() + u16_current_size / u16_new_block_size;

    pcz_req_block_status->set_current_block_no( u32_next_block_no );
    pcz_req_block_status->set_block_size( u16_new_block_size );

    auto pcz_next_req_block = client_blockwise_get_next_request_block( get_client_request(), pcz_req_block_status );
    send_single_request_msg( pcz_next_req_block );
  }
  else if( pcz_response_msg->get_code() == COAP_MSG_CODE_REQUEST_ENTITY_INCOMPLETE_408 )
  {
    delete_request_block_transfer_status();
    pcz_req_block_status = create_request_block_transfer_status( get_client_request(), false );

    auto u16_new_block_size = pcz_response_msg->get_option_set().has_block1() ? pcz_response_msg->get_option_set().get_block1().get_size() : PREFERRED_BLOCK_SIZE;
    pcz_req_block_status->set_block_size( u16_new_block_size );

    auto coap_request_msg = client_blockwise_get_next_request_block( get_client_request(), pcz_req_block_status );
    send_single_request_msg( coap_request_msg );
  }
  else if( pcz_response_msg->get_code() == COAP_MSG_CODE_REQUEST_ENTITY_TOO_LARGE_413 )
  {
    auto &rcz_options_set = pcz_response_msg->get_option_set();

    if( rcz_options_set.has_block1() )
    {
      auto u16_new_block_size = rcz_options_set.get_block1().get_size();

      if( u16_new_block_size != pcz_req_block_status->get_block_size() )
      {
        pcz_req_block_status->set_block_size( u16_new_block_size );
        pcz_req_block_status->set_current_block_no( 0 );

        auto coap_request_msg = client_blockwise_get_next_request_block( get_client_request(), pcz_req_block_status );
        send_single_request_msg( coap_request_msg );
      }
    }
    else
    {
      /* resource request exceeds the max size of the resource body */
    }
  }
  else if( !pcz_response_msg->get_option_set().has_block2() )
  {
    /* there is no block2 transfer, it means this is the final response */
    auto pcz_client_response = new ClientResponse{ pcz_response_msg };
    notify_response( pcz_client_response, CLIENT_RESPONSE_STATUS_OK );
  }
}

void ClientInteraction::client_handle_block2_transfer( CoapMsg *pcz_response_msg )
{
  auto &rcz_options_set = pcz_response_msg->get_option_set();

  auto pcz_res_blk_status = get_response_block_transfer_status();

  if( pcz_res_blk_status == nullptr )
  {
	  /* blockwise response not started previously, so start it now*/
    pcz_res_blk_status = create_response_block_transfer_status( pcz_response_msg, true );
    pcz_res_blk_status->set_block_size( rcz_options_set.get_block2().get_size() );
  }

  if( rcz_options_set.get_block2().get_num() == pcz_res_blk_status->get_next_block_no() )
  {
    pcz_res_blk_status->add_payload( (int8_t *) pcz_response_msg->get_payload(), pcz_response_msg->get_payload_len() );

    if( rcz_options_set.get_block2().has_more() )
    {
      pcz_res_blk_status->move_to_next_block();

      auto new_client_request = get_client_request()->create_msg_with_options();
      MessageIdProvider::assign_message_id( *new_client_request );
      new_client_request->set_endpoint( get_endpoint() );
      new_client_request->get_option_set().set_block2( pcz_res_blk_status->get_szx(), false, pcz_res_blk_status->get_next_block_no() );
      auto current_out_exchange = create_outgoing_exchange( new_client_request );
      current_out_exchange->deliver();
    }
    else
    {
    	/* no more response blocks, now it is time to assemble all the received block2 messages and pass
    	 * it to the stack */
      auto client_response = new ClientResponse{};
      client_response->copy_with_options( pcz_response_msg );
      client_response->set_id( pcz_response_msg->get_id() );
      client_response->set_payload( (uint8_t *) pcz_res_blk_status->get_payload_buffer(), pcz_res_blk_status->get_payload_length() );
      /* need to check where the payload_buffer memory is deleted */
      delete_response_block_transfer_status();
      notify_response( client_response, CLIENT_RESPONSE_STATUS_OK );
    }
  }
}

CoapMsg * ClientInteraction::client_blockwise_get_next_request_block( CoapMsg *client_request, BlockTransferStatus *req_blk_transfer_status )
{
  auto block_size       = req_blk_transfer_status->get_block_size();
  auto current_block_no = req_blk_transfer_status->get_current_block_no();
  auto from             = current_block_no * block_size;
  auto to               = std::min( (const uint16_t) ( ( current_block_no + 1 ) * block_size ), (const uint16_t) client_request->get_payload_len() );
  auto length           = to - from;
  auto payload          = new int8_t[length];

  auto orig_payload = client_request->get_payload();

  memcpy( payload, &orig_payload[from], length );

  auto new_req_block = new CoapMsg{};
  new_req_block->set_type( client_request->get_type() );
  new_req_block->set_code( client_request->get_code() );
  new_req_block->get_option_set() = client_request->get_option_set();

  new_req_block->set_payload( (uint8_t *) payload, length );
  new_req_block->get_option_set().set_block1( req_blk_transfer_status->get_szx(), to < client_request->get_payload_len(), current_block_no );
  new_req_block->get_option_set().set_size1( client_request->get_payload_len() );

  return ( new_req_block );
}

void ClientInteraction::send_single_request_msg( CoapMsg *pcz_new_request )
{
  /* check whether the message type is set in the request. If it is not set
   * then default it will be CON message */
  if( pcz_new_request->get_type() == COAP_MSG_TYPE_NONE )
  {
    pcz_new_request->set_type( COAP_MSG_TYPE_CON );
  }

  /* assign new message id, if it is not not set */
  MessageIdProvider::assign_message_id( *pcz_new_request );

  pcz_new_request->set_token( get_token() );
  pcz_new_request->set_endpoint( get_endpoint() );

  auto pcz_outgoing_exchange = create_outgoing_exchange( pcz_new_request );
  pcz_outgoing_exchange->deliver();
}

bool ClientInteraction::is_msg_matched( CoapMsg *pcz_response_msg )
{
  return ( _token == pcz_response_msg->get_token() && _endpoint == pcz_response_msg->get_endpoint() );
}
}
}
