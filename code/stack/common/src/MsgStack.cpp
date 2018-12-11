#include <adapter_mgr.h>
#include <coap/coap_consts.h>
#include <coap/coap_msg.h>
#include <coap/coap_msg_codec.h>
#include <coap/CoapMsgPrinter.h>
#include <common/inc/ClientInteraction.h>
#include <common/inc/InteractionStore.h>
#include <common/inc/ServerInteraction.h>
#include <common/inc/MulticastClientInteraction.h>
#include <end_point.h>
#include <ErrCode.h>
#include <Exchange.h>
#include <MessageSender.h>
#include <MsgStack.h>
#include <OsalError.h>
#include <OsalMgr.h>
#include <OsalTimer.h>
#include <ResourceMgr.h>
#include <StackEvents.h>
#include <StackMessage.h>
#include <Task.h>
#include <iostream>
#include <stdio.h>
#include "StackEvents.h"
#include "base_datatypes.h"

#define __FILE_NAME__ "MsgStack"

#ifdef __GNUC__
#define DBG_INFO2( format, ... ) printf( "INF:%s::%s:%d# " format "\n", __FILE_NAME__, __FUNCTION__, __LINE__, ## __VA_ARGS__ )
#define DBG_WARN2( format, ... ) printf( "WRN:%s::%s:%d# " format "\n", __FILE_NAME__, __FUNCTION__, __LINE__, ## __VA_ARGS__ )
#define DBG_ERROR2( format, ... ) printf( "ERR:%s::%s:%d# " format "\n", __FILE_NAME__, __FUNCTION__, __LINE__, ## __VA_ARGS__ )
#define DBG_FATAL2( format, ... ) printf( "FTL:%s::%s:%d# " format "\n", __FILE_NAME__, __FUNCTION__, __LINE__, ## __VA_ARGS__ )
#else
#define DBG_INFO2( format, ... ) printf( "INF:%s:%d# " format "\n", __FUNCTION__, __LINE__, ## __VA_ARGS__ )
#define DBG_WARN2( format, ... ) printf( "WRN:%s:%d# " format "\n", __FUNCTION__, __LINE__, ## __VA_ARGS__ )
#define DBG_ERROR2( format, ... ) printf( "ERR:%s:%d# " format "\n", __FUNCTION__, __LINE__, ## __VA_ARGS__ )
#define DBG_FATAL2( format, ... ) printf( "FTL:%s:%d# " format "\n", __FUNCTION__, __LINE__, ## __VA_ARGS__ )
#endif

using namespace std;
using namespace ja_iot::base;
using namespace ja_iot::network;
using namespace ja_iot::osal;
using namespace ja_iot::stack;

#ifdef _OS_FREERTOS_
extern "C" uint32_t system_get_free_heap_size();
#endif

static void packet_received_callback( void *pv_user_data, Endpoint const &end_point, const uint8_t *data, uint16_t data_len );
static void STACK_TASK_handle_msg_cb( void *pv_task_arg, void *pv_user_data );
static void STACK_TASK_delete_msg_cb( void *pv_task_arg, void *pv_user_data );
static void heart_beat_timer_cb( void *user_param1, void *user_param2 );

void receive_msg( CoapMsg *in_msg );
void server_receive_request_msg( CoapMsg *server_request_msg );
void receive_empty_msg( CoapMsg *empty_msg );
void client_receive_response_msg( CoapMsg *client_response_msg );
void receive_stack_event( StackEvent *pv_stack_msg );

void HANDLE_STACK_EVENT_send_server_response( ServerResponse *server_response );
void HANDLE_STACK_EVENT_heart_beat();
void HANDLE_STACK_EVENT_receive_endpoint_data( EndpointDataStackEvent *pv_stack_msg );
void HANDLE_STACK_EVENT_send_client_request( ClientRequest *client_request, client_response_cb response_cb );

namespace ja_iot {
namespace stack {
class AdapterMgrEventHandler : public IAdapterMgrEventHandler
{
  public:
    void handle_packet_received( Endpoint const &end_point, const data_buffer_t &data_buffer )      override
    {
      const auto new_raw_data_msg = new EndpointDataStackEvent{ end_point, data_buffer._pu8_data, data_buffer._u16_data_len };

      cout << "received packet of length " << data_buffer._u16_data_len << endl;
      _msg_stack->send_stack_event( new_raw_data_msg );
    }
    void handle_error( Endpoint const &end_point, const data_buffer_t &data_buffer, ErrCode error ) override {}

    MsgStack * _msg_stack = nullptr;
};

static AdapterMgrEventHandler _gs_adapter_event_handler;
MsgStack *MsgStack::_pcz_instance{ nullptr };

OptionsCallback               required_options_callback = [] ( uint16_t option_no ) -> bool {
    return ( true );
  };
MsgStack::MsgStack ()
{
}
MsgStack::~MsgStack ()
{
}

MsgStack & MsgStack::inst()
{
  if( _pcz_instance == nullptr )
  {
    static MsgStack _instance{};
    _pcz_instance = &_instance;
  }

  return ( *_pcz_instance );
}

void MsgStack::initialize( const uint16_t configured_adapter_types )
{
  _gs_adapter_event_handler._msg_stack = this;
//  AdapterManager::Inst().set_packet_received_cb( packet_received_callback, this );
  AdapterManager::Inst().set_event_handler( &_gs_adapter_event_handler );

  auto ret_status = AdapterManager::Inst().initialize_adapters( configured_adapter_types );

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR2( "initialize_adapters FAILED" );
  }

  DBG_INFO2( "Initialized" );

  ret_status = AdapterManager::Inst().start_adapter( configured_adapter_types );

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR2( "start_adapter FAILED" );
  }

  DBG_INFO2( "adapter started" );

  ret_status = AdapterManager::Inst().start_servers();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR2( "start_servers FAILED" );
  }

  DBG_INFO2( "servers started" );

  _task = OsalMgr::Inst()->AllocTask();

  _task->Init( { SIMPLE_STACK_TASK_NAME, SIMPLE_STACK_TASK_PRIORITY,
                 SIMPLE_STACK_TASK_STACK_SIZE, &_task_msg_q,
                 STACK_TASK_handle_msg_cb, this,
                 STACK_TASK_delete_msg_cb, this } );

  _task->Start();

  // ResourceMgr::inst().init_default_resources();

  _heart_beat_timer = new OsalTimer{ 10000000, heart_beat_timer_cb, this };
  _heart_beat_timer->start();
}

void MsgStack::send_stack_event( StackEvent *pcz_stack_msg )
{
  if( pcz_stack_msg != nullptr )
  {
    _task->SendMsg( pcz_stack_msg );
  }
}
}
}

/*{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{*/
/*{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{ STACK MESSAGE HANDLING FUNCTIONS {{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{*/
/*{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{*/

void HANDLE_STACK_EVENT_heart_beat()
{
  auto &i_store = InteractionStore::inst();

  i_store.check_remove_expired_server_exchanges();
  i_store.remove_expired_server_interaction();

  // i_store.print_server_exchanges();
  // i_store.print_server_interactions();
  //
  // i_store.print_client_exchanges();
  // i_store.print_client_interactions();
}

/*
 * This API is used by the client to send the request.
 * The request may be unicast or multicast request. Then handling of the request will be different based on the
 * request type.
 *
 * 1. UNICAST request
 * 2. MULTICAST request
 *
 */
void HANDLE_STACK_EVENT_send_client_request( ClientRequest *client_request, client_response_cb response_cb )
{
  if( client_request->get_endpoint().is_multicast() )
  {
    auto mcast_client_interaction = InteractionStore::inst().create_multicast_client_interaction( client_request );

    mcast_client_interaction->set_response_cb( response_cb );
    mcast_client_interaction->send_request();
  }
  else
  {
    /* thi is unicast request */
    auto client_interaction = InteractionStore::inst().create_client_interaction( client_request );

    client_interaction->set_response_cb( response_cb );
    client_interaction->send_request();
  }
}

/**
 * Handles the received data from the lower layer.
 *
 * This api performs all basic coap related validation of messages.
 *
 * @param pv_stack_msg
 */
void HANDLE_STACK_EVENT_receive_endpoint_data( EndpointDataStackEvent *pv_stack_msg )
{
  if( ( pv_stack_msg == nullptr ) || ( pv_stack_msg->data == nullptr ) || ( pv_stack_msg->data_len == 0 ) )
  {
    return;
  }

  cout << "received packet of length " << pv_stack_msg->data_len << endl;

  auto is_multicast_msg = pv_stack_msg->endpoint.is_multicast();
  CoapMsgHdr coap_msg_hdr{};

  /* parse the coap message headers and get the basic information about the message */
  auto ret_status = CoapMsgCodec::parse_coap_header( pv_stack_msg->data, pv_stack_msg->data_len, coap_msg_hdr );

  if( ret_status == ErrCode::MSG_FORMAT_ERROR )
  {
    /* received message is not valid and there may be some failure. */
    if( !is_multicast_msg && coap_msg_hdr.is_confirmable() && coap_msg_hdr.has_mid() )
    {
      /* reject erroneous reliably transmitted message as mandated by CoAP spec */
      // printf( "received msg with format_error, sending RST msg\n" );
      MessageSender::send_empty_msg( pv_stack_msg->endpoint, COAP_MSG_TYPE_RST, coap_msg_hdr._msg_id );
    }

    /* ignore erroneous messages that are not transmitted reliably */
    // printf( "received msg with format_error, ignoring message\n" );
    delete[] pv_stack_msg->data;
    return;
  }
  else if( ret_status != ErrCode::OK )
  {
    /* ignore erroneous messages that are not transmitted reliably */
    printf( "received erroneous message, returning %d\n", (int) ret_status );
    delete[] pv_stack_msg->data;
    return;
  }

  /* if the message is not EMPTY message and if token is zero length do not process */
  if( !coap_msg_hdr.is_empty_msg() && ( coap_msg_hdr._token_len == 0 ) )
  {
    if( !is_multicast_msg )
    {
      MessageSender::send_empty_msg( pv_stack_msg->endpoint, COAP_MSG_TYPE_RST, coap_msg_hdr._msg_id );
    }

    printf( "received message without token, returning\n" );
    delete[] pv_stack_msg->data;
    return;
  }

  if( !is_multicast_msg && coap_msg_hdr.is_confirmable() && coap_msg_hdr.is_empty_msg() )
  {
    /* coap ping */
    MessageSender::send_empty_msg( pv_stack_msg->endpoint, COAP_MSG_TYPE_RST, coap_msg_hdr._msg_id );
    delete[] pv_stack_msg->data;
    return;
  }

  if( coap_msg_hdr.is_request() && is_multicast_msg )
  {
    if( coap_msg_hdr.is_confirmable() )
    {
      // MessageSender::send_empty_msg( pv_stack_msg->endpoint, COAP_MSG_TYPE_RST, coap_msg_hdr._msg_id );
      DBG_WARN2( "recvd CON msg from MCAST address" );
      delete[] pv_stack_msg->data;
      return;
    }

    if( coap_msg_hdr._code != COAP_MSG_CODE_GET )
    {
      // MessageSender::send_empty_msg( pv_stack_msg->endpoint, COAP_MSG_TYPE_RST, coap_msg_hdr._msg_id );
      DBG_WARN2( "recvd non GET msg from MCAST address" );
      delete[] pv_stack_msg->data;
      return;
    }
  }

  CoapMsg *coap_msg = new CoapMsg{};
  coap_msg->set_endpoint( pv_stack_msg->endpoint );

  ret_status = CoapMsgCodec::decode_coap_msg( pv_stack_msg->data, pv_stack_msg->data_len, *coap_msg, required_options_callback );

  // CoapMsgPrinter::print_coap_msg( *coap_msg, 0 );

  receive_msg( coap_msg );// api common for both client and server
  delete[] pv_stack_msg->data;
}

void HANDLE_STACK_EVENT_send_server_response( ServerResponse *server_response )
{
  if( server_response != nullptr )
  {
    /* find the server interaction based on the token value */
    auto server_interaction = InteractionStore::inst().find_server_interaction( server_response );

    if( server_interaction == nullptr )
    {
      delete server_response;
      return;
    }

    server_interaction->send_separate_response( server_response );
  }
}
/*}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}*/
/*}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}} STACK MESSAGE HANDLING FUNCTIONS }}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}*/
/*}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}*/

/*{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{*/
/*{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{ COMMON FUNCTIONS {{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{*/
/*{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{*/

void receive_stack_event( ja_iot::stack::StackEvent *pv_stack_msg )
{
  switch( pv_stack_msg->get_msg_type() )
  {
    case SoStackMsgType::HEART_BEAT_TIMER:
    {
      // printf( "Got heart beat msg, timestamp[%I64d]\n", OsalTimer::get_system_time() );
      HANDLE_STACK_EVENT_heart_beat();
#ifdef _OS_FREERTOS_
      printf( "free mem %d\n", system_get_free_heap_size() );
#endif
    }
    break;
    case SoStackMsgType::ENDPOINT_DATA:
    {
      HANDLE_STACK_EVENT_receive_endpoint_data( static_cast<EndpointDataStackEvent *>( pv_stack_msg ) );
    }
    break;
    case SoStackMsgType::SERVER_SEND_RESPONSE:
    {
      auto send_response_stack_msg = static_cast<SendServerResponseStackEvent *>( pv_stack_msg );
      HANDLE_STACK_EVENT_send_server_response( send_response_stack_msg->_server_response );
    }
    break;
    case SoStackMsgType::CLIENT_SEND_REQUEST:
    {
      auto client_send_request_stack_msg = static_cast<ClientSendRequestStackEvent *>( pv_stack_msg );
      HANDLE_STACK_EVENT_send_client_request( client_send_request_stack_msg->_client_request, client_send_request_stack_msg->_client_response_cb );
    }
    break;

    case SoStackMsgType::NONE: break;
    default:;
  }
}

void receive_msg( ja_iot::network::CoapMsg *in_msg )
{
  if( in_msg->is_request() )
  {
    /* server will always receive the message in the form of request */
    server_receive_request_msg( in_msg );
  }
  else if( in_msg->is_empty_msg() )
  {
    /* this is the generic handling of empty message both for client and server */
    receive_empty_msg( in_msg );
  }
  else
  {
    /* client will always receive the message in the form of response */
    client_receive_response_msg( in_msg );
  }
}

/* the empty message will be received
 * 1. ACK for local exchange.
 * 2. RST for local exchange.
 */
void receive_empty_msg( CoapMsg *empty_msg )
{
  if( empty_msg == nullptr )
  {
    return;
  }

  auto &i_store = InteractionStore::inst();

  if( empty_msg->is_ack() )
  {
    /* check is there any server exchange for this empty message.
     * Since the message is replied only for the other message received, so we need to find
     * the exchange of previoulsy send message */
    auto server_exchange = InteractionStore::inst().find_server_exchange( *empty_msg );

    if( server_exchange != nullptr )
    {
      // printf( "got the ACK for CON with MID [%x]\n", empty_msg->get_id() );

      if( server_exchange->get_interaction() != nullptr )
      {
        server_exchange->get_interaction()->delete_exchange( server_exchange );
      }
    }
    else
    {
      auto client_exchange = i_store.find_client_exchange( *empty_msg );

      if( client_exchange != nullptr )
      {
        if( client_exchange->get_interaction() != nullptr )
        {
          client_exchange->get_interaction()->delete_exchange( client_exchange );
        }
      }
    }

    // printf( "got empty msg for unknown transaction\n" );
  }
  else if( empty_msg->is_rst() )
  {
    auto exchange = InteractionStore::inst().find_server_exchange( *empty_msg );

    if( ( exchange != nullptr ) && ( exchange->get_interaction() != nullptr ) )
    {
      exchange->get_interaction()->delete_exchange( exchange );
    }
    else
    {
      auto client_exchange = i_store.find_client_exchange( *empty_msg );

      if( ( client_exchange != nullptr ) && ( client_exchange->get_interaction() != nullptr ) )
      {
        ( (ja_iot::stack::ClientInteraction *) client_exchange->get_interaction() )->notify_response( nullptr, CLIENT_RESPONSE_STATUS_REJECTED );
        client_exchange->get_interaction()->delete_exchange( client_exchange );
      }
    }
  }

  delete empty_msg;
}

void server_receive_request_msg( CoapMsg *server_request_msg )
{
  /* check for deduplication. If the received message is CON then check for whether
   * this request message is already received.
   * If it is already received then send the same response previously send for the request
   */
  if( server_request_msg->is_confirmable() )
  {
    auto exchange = InteractionStore::inst().find_server_exchange( *server_request_msg );

    if( exchange != nullptr )
    {
      /* duplicate request message received, send the already created reply message */
      exchange->resend_reply();
      delete server_request_msg;
      return;
    }
  }

  /* we didn't received the duplicate message, so this may be new request or continuation of previous request */
  auto server_interaction = InteractionStore::inst().find_server_interaction( server_request_msg, true );
  server_interaction->receive_request( server_request_msg, true );
}

/**
 * Response handling for client.
 * There are two types of response client can receive.
 * 1. Unicast response.
 * 2. Multicast response.
 *
 * For unicast response it is straight forward, find the interaction and handle it.
 *
 * For multicast response it is little bit more work because the response can come from any number of servers.
 * So it is required to create a new interaction for each response from different servers. Then onwards the
 * created interaction will be used for further message handling.
 *
 * @param client_response_msg
 */
void client_receive_response_msg( CoapMsg *client_response_msg )
{
  auto               &i_store           = InteractionStore::inst();
  ClientInteraction *client_interaction = nullptr;

  auto               mcast_interaction_list = i_store.get_mcast_interaction_list();

  for( MulticastClientInteraction * &mcast_interaction : mcast_interaction_list )
  {
    if( mcast_interaction->get_client_request()->get_token() == client_response_msg->get_token() )
    {
      for( auto &base_interaction : i_store.get_client_interaction_list() )
      {
        if( static_cast<ClientInteraction *>( base_interaction )->get_endpoint() == client_response_msg->get_endpoint() )
        {
          /* found the previous client interaction for the multicast request */
          client_interaction = static_cast<ClientInteraction *>( base_interaction );
          break;
        }
      }

      if( client_interaction == nullptr )
      {
        /* not found the previous interaction so it is new response came from the server */
        client_interaction = new ClientInteraction{ mcast_interaction->get_client_request(), mcast_interaction->get_response_cb() };
        client_interaction->set_token( mcast_interaction->get_client_request()->get_token() );
        client_interaction->set_endpoint( client_response_msg->get_endpoint() );
        client_interaction->set_multicast( true );

        i_store.get_client_interaction_list().push_back( client_interaction );
        mcast_interaction->add_client_interaction();
      }

      break;
    }
  }

  if( client_interaction == nullptr )
  {
    /* it is not multicast response, then search for normal request */
    client_interaction = i_store.find_client_interaction( client_response_msg );
  }

  if( client_interaction == nullptr )
  {
    DBG_WARN2( "received msg for unknown interaction" );
    delete client_response_msg;
  }
  else
  {
    /* check for deduplication */
    if( client_response_msg->is_confirmable() )
    {
      auto client_exchange = i_store.find_client_exchange( *client_response_msg );

      if( client_exchange != nullptr )
      {
        /* deduplication */
        client_exchange->deliver();
        delete client_response_msg;
        return;
      }
    }

    client_interaction->receive_response( client_response_msg );
  }
}
/*}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}*/
/*}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}} COMMON FUNCTIONS }}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}*/
/*}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}*/

/*{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{*/
/*{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{ PRIVATE CALLBACK FUNCTIONS {{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{*/
/*{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{*/

/**
 * This is the main callback function for the input data from lower stack.
 * @param pv_user_data
 * @param rcz_end_point - endpoint from where the data comes from
 * @param pu8_data      - received data
 * @param u16_data_len  - received data length
 */
static void packet_received_callback( void *pv_user_data, Endpoint const &rcz_end_point, const uint8_t *pu8_data, const uint16_t u16_data_len )
{
  if( pv_user_data )
  {
    const auto new_raw_data_msg = new EndpointDataStackEvent{ rcz_end_point, const_cast<uint8_t *>( pu8_data ), u16_data_len };
    // cout << "received packet of length " << data_len << endl;
    static_cast<MsgStack *>( pv_user_data )->send_stack_event( new_raw_data_msg );
  }
}

static void STACK_TASK_handle_msg_cb( void *pv_task_arg, void *pv_user_data )
{
  if( pv_user_data != nullptr )
  {
    receive_stack_event( static_cast<StackEvent *>( pv_task_arg ) );
  }
}

static void STACK_TASK_delete_msg_cb( void *pv_task_arg, void *pv_user_data )
{
  const auto stack_msg = static_cast<StackEvent *>( pv_task_arg );

  switch( stack_msg->get_msg_type() )
  {
    case SoStackMsgType::ENDPOINT_DATA:
      delete static_cast<EndpointDataStackEvent *>( pv_task_arg );
      break;
    case SoStackMsgType::HEART_BEAT_TIMER:
      delete static_cast<HeartBeatTimerStackEvent *>( pv_task_arg );
      break;
    case SoStackMsgType::SERVER_SEND_RESPONSE:
      delete static_cast<SendServerResponseStackEvent *>( pv_task_arg );
      break;
    case SoStackMsgType::CLIENT_SEND_REQUEST:
      delete static_cast<ClientSendRequestStackEvent *>( pv_task_arg );
      break;
    default:
      break;
  }
}

static void heart_beat_timer_cb( void *user_param1, void *user_param2 )
{
  if( user_param1 )
  {
    ( (MsgStack *) user_param1 )->send_stack_event( new HeartBeatTimerStackEvent{} );
  }
}