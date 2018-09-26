/*
 * ClientInteraction.h
 *
 *  Created on: Feb 14, 2018
 *      Author: psammand
 */

#pragma once

#include <common/inc/BaseInteraction.h>
#include <StackMessage.h>
#include <cstdint>

namespace ja_iot {
namespace stack {
class ClientInteraction : public BaseInteraction
{
  public:
    ClientInteraction( ClientRequest *client_request );
    ClientInteraction( ClientRequest *client_request, client_response_cb response_cb );
    virtual ~ClientInteraction ();

    void add_exchange( Exchange *new_exchange );
    void delete_exchange( Exchange *exchange );

    ClientRequest*  get_client_request() { return ( _client_request ); }
    void            set_client_request( ClientRequest *client_request ) { _client_request = client_request; }
    ClientResponse* get_client_response() { return ( _client_response ); }
    void            set_client_response( ClientResponse *client_response ) { _client_response = client_response; }

    client_response_cb get_response_cb() { return ( _client_response_cb ); }
    void               set_response_cb( client_response_cb response_cb ) { _client_response_cb = response_cb; }

    void                      send_request();
    void                      receive_response( ja_iot::network::CoapMsg *client_response_msg );
    void                      notify_response( ClientResponse *client_response, uint8_t response_status );
    client_response_cb        get_response_callback() { return ( _client_response_cb ); }
    void                      client_handle_block1_transfer( ja_iot::network::CoapMsg *client_response_msg );
    void                      client_handle_block2_transfer( ja_iot::network::CoapMsg *client_response_msg );
    ja_iot::network::CoapMsg* client_blockwise_get_next_request_block( ja_iot::network::CoapMsg *client_request, BlockTransferStatus *req_blk_transfer_status );
    void                      send_single_request_msg( ja_iot::network::CoapMsg *new_request );

    bool is_multicast() { return ( _is_multicast ); }
    void set_multicast( bool value ) { _is_multicast = value; }

    ja_iot::network::Endpoint& get_endpoint() { return ( _endpoint ); }
    void                     set_endpoint( const ja_iot::network::Endpoint &endpoint ) { _endpoint = endpoint; }

    ja_iot::network::CoapMsgToken& get_token() { return ( _token ); }
    void                         set_token( const ja_iot::network::CoapMsgToken &token ) { _token = token; }

    bool is_msg_matched( ja_iot::network::CoapMsg *client_response_msg );

  private:
    ClientRequest *                 _client_request     = nullptr;
    ClientResponse *                _client_response    = nullptr;
    client_response_cb              _client_response_cb = nullptr;
    bool                            _is_multicast       = false;
    ja_iot::network::Endpoint       _endpoint;
    ja_iot::network::CoapMsgToken   _token;
};
}
}