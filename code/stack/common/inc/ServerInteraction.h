/*
 * ServerInteraction.h
 *
 *  Created on: Feb 14, 2018
 *      Author: psammand
 */

#pragma once

#include <coap/coap_msg.h>
#include <common/inc/BaseInteraction.h>
#include <end_point.h>
#include <cstdint>

namespace ja_iot {
namespace stack {
/**
 * Server Interaction implementation.
 *
 * Instance of this class will be created whenever new request comes from client.
 */
class ServerInteraction : public BaseInteraction
{
  public:
    ServerInteraction( ja_iot::network::CoapMsgToken &token, ja_iot::network::Endpoint &endpoint );
    virtual ~ServerInteraction ();

    virtual ServerRequest*  get_server_request()                         override { return ( _server_request ); }
    virtual void            set_server_request( ServerRequest *request ) override { _server_request = request; }
    virtual ServerResponse* get_server_response()                           override { return ( _server_response ); }
    virtual void            set_server_response( ServerResponse *response ) override { _server_response = response; }


    void add_exchange( Exchange *new_exchange ) override;
    void delete_exchange( Exchange *exchange ) override;

    void                      receive_request( ja_iot::network::CoapMsg *new_request_msg, bool is_ongoing );
    void                      handle_fully_downloaded_request();
    void                      handle_request_download();
    void                      send_response( ja_iot::network::CoapMsg *response_msg );
    void                      send_separate_response( ServerResponse *separate_response_msg );
    void                      send_block1_error_response( ja_iot::network::CoapMsg *request_msg, uint8_t block1_error );
    ja_iot::network::CoapMsg* get_first_response_block( ja_iot::network::CoapMsg *out_message );
    ja_iot::network::CoapMsg* get_next_res_block( ja_iot::network::CoapMsg *response, bool &has_more );

    bool is_matched( ja_iot::network::CoapMsg *request );

  private:
    ja_iot::network::CoapMsgToken   _token;
    ja_iot::network::Endpoint       _endpoint;
    ServerRequest *                 _server_request  = nullptr;// data received from client
    ServerResponse *                _server_response = nullptr;// data to send to client
};
}
}
