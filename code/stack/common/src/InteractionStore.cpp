#include <stdio.h>
#include "common/inc/InteractionStore.h"
#include "OsalTimer.h"
#include "base_utils.h"
#include "common/inc/ServerInteraction.h"
#include "common/inc/ClientInteraction.h"
#include "common/inc/MulticastClientInteraction.h"
#include "logging_stack.h"

#define __FILE_NAME__ "i_store"

using namespace ja_iot::osal;
using namespace ja_iot::base;
using namespace ja_iot::network;

namespace ja_iot {
namespace stack {
InteractionStore *InteractionStore::_pcz_instance{ nullptr };
InteractionStore::InteractionStore ()
{
  _server_exchanges.reserve( 10 );
  _server_interactions.reserve( 10 );
  _client_exchanges.reserve( 10 );
  _client_interactions.reserve( 10 );
  _mcast_client_interactions.reserve( 10 );
}
InteractionStore::~InteractionStore ()
{
}

InteractionStore & InteractionStore::inst()
{
  if( _pcz_instance == nullptr )
  {
    static InteractionStore _scz_instance{};
    _pcz_instance = &_scz_instance;
  }

  return ( *_pcz_instance );
}

/**
 * Finds the server exchange for the coap message.
 * The match criteria is the coap message should be from same endpoint and same message id.
 *
 * @param rcz_coap_msg - received coap message.
 * @return
 */
Exchange * InteractionStore::find_server_exchange( CoapMsg &rcz_coap_msg )
{
  for( auto &pcz_loop_exchange : _server_exchanges )
  {
    if( pcz_loop_exchange->is_msg_matched( rcz_coap_msg ) )
    {
      return ( pcz_loop_exchange );
    }
  }

  return ( nullptr );
}

/**
 * Adds new server exchange to store.
 *
 * @param pcz_server_exchange
 * @return
 */
bool InteractionStore::add_server_exchange( Exchange *pcz_server_exchange )
{
  _server_exchanges.push_back( pcz_server_exchange );
  return ( true );
}

bool InteractionStore::delete_server_exchange( Exchange *pcz_server_exchange )
{
  if( pcz_server_exchange == nullptr )
  {
    return ( false );
  }

  for( size_t i = 0; i < _server_exchanges.size(); ++i )
  {
    if( ( _server_exchanges[i] != nullptr ) &&
      ( pcz_server_exchange == _server_exchanges[i] ) )
    {
      _server_exchanges.erase( _server_exchanges.cbegin() + i );
      delete_and_clear<Exchange>( pcz_server_exchange );
      return ( true );
    }
  }

  return ( false );
}
/**
 * Finds the server interaction for this message. The search criteria is the message should be from same
 * endpoint and also with same token.
 *
 * @param pcz_coap_msg   - received message
 * @param create_if_null - if true then new interaction will be created if there is no match.
 * @return
 */
ServerInteraction * InteractionStore::find_server_interaction( CoapMsg *pcz_coap_msg, bool create_if_null )
{
  ServerInteraction *pcz_temp_server_interaction = nullptr;

  for( auto &pcz_loop_server_interaction : _server_interactions )
  {
    if( static_cast<ServerInteraction *>( pcz_loop_server_interaction )->is_matched( pcz_coap_msg ) )
    {
      pcz_temp_server_interaction = static_cast<ServerInteraction *>( pcz_loop_server_interaction );
      break;
    }
  }

  if( ( pcz_temp_server_interaction == nullptr ) && create_if_null )
  {
    /* there is no server interaction for this message and user also request to create new server interaction
     * if there is no such
     */
    pcz_temp_server_interaction = create_server_interaction( pcz_coap_msg );
  }

  return ( pcz_temp_server_interaction );
}

ServerInteraction * InteractionStore::create_server_interaction( CoapMsg *pcz_coap_msg )
{
  auto pcz_server_interaction = new ServerInteraction{ pcz_coap_msg->get_token(), pcz_coap_msg->get_endpoint() };

  if( pcz_server_interaction != nullptr )
  {
    _server_interactions.push_back( pcz_server_interaction );
  }

  return ( pcz_server_interaction );
}

bool InteractionStore::delete_server_interaction( ServerInteraction *pcz_server_interaction )
{
  DBG_INFO2( "Deleting Interaction \n" );

  if( pcz_server_interaction == nullptr )
  {
    return ( false );
  }

  for( size_t i = 0; i < _server_interactions.size(); i++ )
  {
    if( ( _server_interactions[i] != nullptr ) &&
      ( _server_interactions[i] == pcz_server_interaction ) )
    {
      _server_interactions.erase( _server_interactions.cbegin() + i );
      delete_and_clear<ServerInteraction>( pcz_server_interaction );
      return ( true );
    }
  }

  return ( false );
}

void InteractionStore::remove_expired_server_interaction()
{
  InteractionList cz_expired_transactions_list{};

  for( auto &pcz_loop_server_interaction : _server_interactions )
  {
    if( !pcz_loop_server_interaction->has_exchanges() )
    {
      cz_expired_transactions_list.push_back( pcz_loop_server_interaction );
    }
  }

  for( auto &pcz_loop_expired_server_interaction : cz_expired_transactions_list )
  {
    delete_server_interaction( (ServerInteraction *) pcz_loop_expired_server_interaction );
  }
}

void InteractionStore::check_remove_expired_server_exchanges()
{
  ExchangeList cz_expired_exchanges_list{};

  for( auto it = _server_exchanges.cbegin(); it != _server_exchanges.cend(); ++it )
  {
    auto pcz_loop_exchange = ( *it );

    if( pcz_loop_exchange->is_expired() )
    {
      DBG_INFO2( "Exchange expired, MID:[0x%04X]\n", pcz_loop_exchange->get_originator_msg()->get_id() );
      cz_expired_exchanges_list.push_back( pcz_loop_exchange );
    }
    else if( pcz_loop_exchange->is_con_outgoing() )
    {
      if( pcz_loop_exchange->is_timedout() )
      {
        DBG_INFO2( "Exchange TimedOut, MID:[0x%04X]\n", pcz_loop_exchange->get_originator_msg()->get_id() );
        cz_expired_exchanges_list.push_back( pcz_loop_exchange ); continue;
      }

      pcz_loop_exchange->retransmit();
    }
  }

  for( auto &pcz_loop_expired_exchange : cz_expired_exchanges_list )
  {
    pcz_loop_expired_exchange->get_interaction()->delete_exchange( pcz_loop_expired_exchange );
  }
}

Exchange * InteractionStore::find_client_exchange( CoapMsg &rcz_coap_msg )
{
  for( auto &pcz_loop_exchange : _client_exchanges )
  {
    if( pcz_loop_exchange->is_msg_matched( rcz_coap_msg ) )
    {
      return ( pcz_loop_exchange );
    }
  }

  return ( nullptr );
}

bool InteractionStore::add_client_exchange( Exchange *pcz_client_exchange )
{
  _client_exchanges.push_back( pcz_client_exchange );
  return ( true );
}

bool InteractionStore::delete_client_exchange( Exchange *pcz_client_exchange )
{
  for( size_t i = 0; i < _client_exchanges.size(); ++i )
  {
    if( pcz_client_exchange == _client_exchanges[i] )
    {
      _client_exchanges.erase( _client_exchanges.cbegin() + i );
      delete_and_clear<Exchange>( pcz_client_exchange );
      return ( true );
    }
  }

  return ( false );
}

ClientInteraction * InteractionStore::create_client_interaction( CoapMsg *pcz_coap_msg )
{
  auto pcz_new_client_interaction = new ClientInteraction{ (ClientRequest *) pcz_coap_msg };

  _client_interactions.push_back( pcz_new_client_interaction );

  return ( pcz_new_client_interaction );
}

ClientInteraction * InteractionStore::find_client_interaction( CoapMsg *pcz_coap_msg )
{
  for( auto &pcz_loop_client_interaction : _client_interactions )
  {
    auto pcz_temp_client_interaction = static_cast<ClientInteraction *>( pcz_loop_client_interaction );

    if( pcz_temp_client_interaction->is_msg_matched( pcz_coap_msg ) )
    {
      return ( pcz_temp_client_interaction );
    }
  }

  return ( nullptr );
}

MulticastClientInteraction * InteractionStore::create_multicast_client_interaction( ja_iot::network::CoapMsg *coap_msg )
{
  auto mcast_client_interaction = new MulticastClientInteraction{ (ClientRequest *) coap_msg };

  _mcast_client_interactions.push_back( mcast_client_interaction );

  return ( mcast_client_interaction );
}

MulticastClientInteraction * InteractionStore::find_multicast_client_interaction( ja_iot::network::CoapMsg *coap_msg )
{
	return nullptr;
}

void InteractionStore::print_server_exchanges()
{
  if( _server_exchanges.size() == 0 )
  {
    return;
  }

  DBG_INFO2( "=====[SERVER EXCHANGES]=====================================\n" );

  for( auto it = _server_exchanges.cbegin(); it != _server_exchanges.cend(); ++it )
  {
    auto pcz_loop_server_exchange = ( *it );
    auto pcz_originator_coap_msg  = pcz_loop_server_exchange->get_originator_msg();

    DBG_INFO2( "i:0x%04X, t:", pcz_originator_coap_msg->get_id() );
    pcz_originator_coap_msg->get_token().print();
    DBG_INFO2( ", EXPIRY:[%d]\n", pcz_loop_server_exchange->get_remaining_lifetime() );
  }

  DBG_INFO2( "=====================================================\n" );
}

void InteractionStore::print_server_interactions()
{
  if( _server_interactions.size() == 0 )
  {
    return;
  }

  DBG_INFO2( "=====[SERVER INTERACTIONS]=====================================\n" );

  for( auto it = _server_interactions.cbegin(); it != _server_interactions.cend(); ++it )
  {
    auto pcz_loop_server_interaction = static_cast<ServerInteraction *>( *it );

    if( pcz_loop_server_interaction->get_server_request() != nullptr )
    {
      DBG_INFO2( "t:" );
      pcz_loop_server_interaction->get_server_request()->get_token().print();
    }

    DBG_INFO2( ", EXCHANGES:[%d]", pcz_loop_server_interaction->get_no_of_exchanges() );
    DBG_INFO2( "\n" );
  }

  DBG_INFO2( "=====================================================\n" );
}

void InteractionStore::print_client_exchanges()
{
  if( _client_exchanges.size() == 0 )
  {
    return;
  }

  DBG_INFO2( "=====[CLIENT EXCHANGES]=====================================\n" );

  for( auto it = _client_exchanges.cbegin(); it != _client_exchanges.cend(); ++it )
  {
    auto pcz_client_exchange = ( *it );
    auto pcz_temp_coap_msg   = pcz_client_exchange->get_originator_msg();

    DBG_INFO2( "i:0x%04X, t:", pcz_temp_coap_msg->get_id() );
    pcz_temp_coap_msg->get_token().print();
    DBG_INFO2( ", EXPIRY:[%d]", pcz_client_exchange->get_remaining_lifetime() );
    DBG_INFO2( "\n" );
  }

  DBG_INFO2( "=====================================================\n" );
}

void InteractionStore::print_client_interactions()
{
  if( _client_interactions.size() == 0 )
  {
    return;
  }

  DBG_INFO2( "=====[CLIENT INTERACTIONS]=====================================\n" );

  for( auto it = _client_interactions.cbegin(); it != _client_interactions.cend(); ++it )
  {
    auto pcz_loop_client_interaction = (ClientInteraction *) ( *it );
    auto pcz_client_request          = pcz_loop_client_interaction->get_client_request();

    if( pcz_client_request != nullptr )
    {
      DBG_INFO2( "t:" );
      pcz_client_request->get_token().print();
    }

    DBG_INFO2( ", EXCHANGES:[%d]", pcz_loop_client_interaction->get_no_of_exchanges() );
    DBG_INFO2( "\n" );
  }

  DBG_INFO2( "=====================================================\n" );
}
}
}