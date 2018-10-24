/*
 * BaseInteraction.h
 *
 *  Created on: Feb 14, 2018
 *      Author: psammand
 */

#pragma once

#include <Interaction.h>
#include <cstdint>


namespace ja_iot {
namespace stack {
class Exchange;
class BlockTransferStatus;

/**
 * Base class for interaction which contains base data structures and common api's for both client and server interaction.
 *
 * Interaction is basic communication between client and server. There are two types of interation client interaction
 * and server interaction.
 * Each interaction is uniquely identified by token.
 *
 * Creation of interaction.
 * ------------------------
 * In client whenever there is new request send ClientInteraction will be created.
 * In server whenever there is new request received ServerInteraction will be created.
 *
 * Each interaction will contain two exchanges, one for received and one for send. This is common for both client and
 * server.
 */
class BaseInteraction : public Interaction
{
  public:
    BaseInteraction () {}
    virtual ~BaseInteraction ();

    virtual ServerRequest*  get_server_request()                         override { return ( nullptr ); }
    virtual void            set_server_request( ServerRequest *request ) override {}
    virtual ServerResponse* get_server_response()                           override { return ( nullptr ); }
    virtual void            set_server_response( ServerResponse *response ) override {}

    virtual void add_exchange( Exchange *new_exchange ) {}
    virtual void delete_exchange( Exchange *exchange );

    Exchange* create_outgoing_exchange( ja_iot::network::CoapMsg *outgoing_msg );
    Exchange* create_incoming_exchange( ja_iot::network::CoapMsg *incoming_msg );

    void                 acknowledge();
    void                 delete_current_in_exchange();
    void                 delete_current_out_exchange();
    BlockTransferStatus* get_request_block_transfer_status() { return ( _request_block_transfer_status ); }
    BlockTransferStatus* get_response_block_transfer_status() { return ( _response_block_transfer_status ); }
    BlockTransferStatus* create_request_block_transfer_status( ja_iot::network::CoapMsg *request_msg, bool is_downloading );
    BlockTransferStatus* create_response_block_transfer_status( ja_iot::network::CoapMsg *response_msg, bool is_downloading );
    void                 delete_request_block_transfer_status();
    void                 delete_response_block_transfer_status();
    uint16_t             get_no_of_exchanges() { return ( _no_of_exchanges ); }
    bool                 has_exchanges() { return ( _no_of_exchanges != 0 ); }
    Exchange*            get_in_exchange() { return ( _current_in_exchange ); }
    Exchange*            get_out_exchange() { return ( _current_out_exchange ); }

  protected:
    Exchange * _current_in_exchange  = nullptr;
    Exchange * _current_out_exchange = nullptr;
    uint16_t   _no_of_exchanges      = 0;

  private:
    BlockTransferStatus * _request_block_transfer_status  = nullptr;
    BlockTransferStatus * _response_block_transfer_status = nullptr;
};
}
}
