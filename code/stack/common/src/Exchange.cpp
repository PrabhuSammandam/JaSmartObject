#include <stdio.h>
#include <cstdint>
#include "Exchange.h"
#include "OsalTimer.h"
#include "OsalRandom.h"
#include "MessageSender.h"
#include "base_utils.h"
#include "StackConfig.h"

using namespace ja_iot::base;
using namespace ja_iot::network;
using namespace ja_iot::osal;

namespace ja_iot {
namespace stack {
Exchange::Exchange( CoapMsg *initiating_message, uint8_t origin ) :
  _origin{ origin }
{
  if( initiating_message != nullptr )
  {
    if( origin == EXCHANGE_ORIGIN_REMOTE )
    {
      in_msg          = initiating_message;
      _expiry_time_us = OsalTimer::get_system_time() + ( initiating_message->is_confirmable() ? CON_EXCHANGE_LIFETIME_US : NON_EXCHANGE_LIFETIME_US );
    }
    else
    {
      out_msg = initiating_message;

      /* set expire for local generated NON exchange, because for local generated CON message message retransmission will take care  */
      if( out_msg->is_non_confirmable() )
      {
        _expiry_time_us = OsalTimer::get_system_time() + NON_EXCHANGE_LIFETIME_US;
      }
    }
  }
}
Exchange::~Exchange ()
{
  set_interaction( nullptr );
  delete_and_clear<CoapMsg>( this->in_msg );
  delete_and_clear<CoapMsg>( this->out_msg );
}

void Exchange::acknowledge()
{
  if( is_origin_remote() && ( in_msg != nullptr ) && in_msg->is_confirmable() )
  {
    out_msg = new CoapMsg{ COAP_MSG_TYPE_ACK };
    out_msg->set_id( in_msg->get_id() );
    out_msg->set_endpoint( in_msg->get_endpoint() );
    deliver();
  }
}

void Exchange::reject()
{
  if( is_origin_remote() && ( in_msg != nullptr ) )
  {
    out_msg = new CoapMsg{ COAP_MSG_TYPE_RST };
    out_msg->set_id( in_msg->get_id() );
    out_msg->set_endpoint( in_msg->get_endpoint() );
    deliver();
  }
}

void Exchange::resend_reply()
{
  if( is_origin_remote() && ( out_msg != nullptr ) && ( in_msg != nullptr ) && in_msg->is_confirmable() )
  {
    deliver();
  }
}

void Exchange::reply_piggyback( ja_iot::network::CoapMsg *response )
{
  response->set_id( in_msg->get_id() );
  out_msg = response;
  deliver();
}

void Exchange::deliver()
{
  if( out_msg != nullptr )
  {
    MessageSender::send( out_msg );
  }
}

bool Exchange::is_msg_matched( CoapMsg &rcz_coap_msg )
{
  auto exchange_msg = ( is_origin_local() ) ? out_msg : in_msg;

  if( ( exchange_msg != nullptr )
    && ( exchange_msg->get_endpoint() == rcz_coap_msg.get_endpoint() )
    && ( exchange_msg->get_id() == rcz_coap_msg.get_id() ) )
  {
    return ( true );
  }

  return ( false );
}

CoapMsg * Exchange::get_originator_msg()
{
  return ( ( is_origin_local() ) ? out_msg : in_msg );
}

bool Exchange::is_expired()
{
  if( ( is_origin_remote() || ( is_origin_local() && out_msg->is_non_confirmable() ) )
    && ( OsalTimer::get_system_time() >= _expiry_time_us ) )
  {
    return ( true );
  }

  return ( false );
}

void Exchange::initiate_retransmission()
{
  _retransmit_count = 0;
  _timeout_us       = OsalRandom::get_random_range( 2000000U, (uint32_t) ( 2000000 * 1.5 ) );
  _expiry_time_us   = OsalTimer::get_system_time() + _timeout_us;
}

void Exchange::retransmit()
{
  if( ( _retransmit_count < 4 ) && ( OsalTimer::get_system_time() >= _expiry_time_us ) )
  {
    _retransmit_count++;
    _expiry_time_us = OsalTimer::get_system_time() + _timeout_us * 2;
    deliver();
  }
}

uint32_t Exchange::get_remaining_lifetime()
{
  return ( uint32_t( _expiry_time_us - OsalTimer::get_system_time() ) );
}
}
}
