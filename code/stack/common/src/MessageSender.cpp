#include <adapter_mgr.h>
#include <coap/coap_msg.h>
#include <coap/coap_msg_codec.h>
#include <coap/CoapMsgPrinter.h>
#include <ErrCode.h>
#include <MessageSender.h>

using namespace ja_iot::base;
using namespace ja_iot::network;

namespace ja_iot {
namespace stack {
void MessageSender::send_empty_msg( Endpoint &rcz_endpoint, uint8_t u8_msg_type, uint16_t u16_msg_id )
{
  uint16_t u16_data_length = 0;
  uint8_t  au8_data_buffer[4];

  CoapMsgCodec::encode_empty_coap_msg( u8_msg_type, u16_msg_id, &au8_data_buffer[0], u16_data_length );
  AdapterManager::Inst().send_unicast_data( rcz_endpoint, &au8_data_buffer[0], u16_data_length );
}

void MessageSender::send( BaseMessage *pcz_coap_msg )
{
  send( (CoapMsg *) pcz_coap_msg );
}

void MessageSender::send( CoapMsg *pcz_coap_msg )
{
  static uint8_t sau8_encode_msg_buffer[1024];
  uint16_t       u16_data_length = 0;

  CoapMsgPrinter::print_coap_msg( *pcz_coap_msg, 1 );

  CoapMsgCodec::encode_coap_msg( *pcz_coap_msg, &sau8_encode_msg_buffer[0], u16_data_length );

  /* only request should be send as multicast */
  if( pcz_coap_msg->get_endpoint().is_multicast() && pcz_coap_msg->is_request() )
  {
    AdapterManager::Inst().send_multicast_data( pcz_coap_msg->get_endpoint(), &sau8_encode_msg_buffer[0], u16_data_length );
  }
  else
  {
    AdapterManager::Inst().send_unicast_data( pcz_coap_msg->get_endpoint(), &sau8_encode_msg_buffer[0], u16_data_length );
  }
}
}
}