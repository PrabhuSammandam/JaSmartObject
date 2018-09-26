#pragma once

#include <common/inc/BaseInteraction.h>
#include <Exchange.h>
#include <vector>

namespace ja_iot {
namespace stack {
class ServerInteraction;
class ClientInteraction;
class MulticastClientInteraction;

typedef std::vector<Exchange *> ExchangeList;
typedef std::vector<BaseInteraction *> InteractionList;
typedef std::vector<MulticastClientInteraction *> MulticastInteractionList;


class InteractionStore
{
  public:
    static InteractionStore& inst();

    Exchange*          find_server_exchange( ja_iot::network::CoapMsg &rcz_coap_msg );
    bool               add_server_exchange( Exchange *server_exchange );
    bool               delete_server_exchange( Exchange *server_exchange );
    ServerInteraction* find_server_interaction( ja_iot::network::CoapMsg *coap_msg, bool create_if_null = false );
    ServerInteraction* create_server_interaction( ja_iot::network::CoapMsg *coap_msg );
    bool               delete_server_interaction( ServerInteraction *server_interaction );
    void               remove_expired_server_interaction();
    void               check_remove_expired_server_exchanges();

    Exchange*          find_client_exchange( ja_iot::network::CoapMsg &rcz_coap_msg);
    bool               add_client_exchange( Exchange *client_exchange );
    bool               delete_client_exchange( Exchange *client_exchange );
    ClientInteraction* find_client_interaction( ja_iot::network::CoapMsg *coap_msg );
    ClientInteraction* create_client_interaction( ja_iot::network::CoapMsg *coap_msg );

    ExchangeList   & get_server_exchange_list() { return ( _server_exchanges ); }
    InteractionList& get_server_interaction_list() { return ( _server_interactions ); }

    ExchangeList            & get_client_exchange_list() { return ( _client_exchanges ); }
    InteractionList         & get_client_interaction_list() { return ( _client_interactions ); }
    MulticastInteractionList& get_mcast_interaction_list() { return ( _mcast_client_interactions ); }

    void print_server_exchanges();
    void print_server_interactions();
    void print_client_exchanges();
    void print_client_interactions();

  private:
    InteractionStore ();
    ~InteractionStore ();
    static InteractionStore * _pcz_instance;
    InteractionStore( const InteractionStore &other )                   = delete;
    InteractionStore( InteractionStore &&other ) noexcept               = delete;
    InteractionStore & operator = ( const InteractionStore &other )     = delete;
    InteractionStore & operator = ( InteractionStore &&other ) noexcept = delete;

    ExchangeList                                _server_exchanges;
    InteractionList                             _server_interactions;
    ExchangeList                                _client_exchanges;
    InteractionList                             _client_interactions;
    std::vector<MulticastClientInteraction *>   _mcast_client_interactions;
};
}
}