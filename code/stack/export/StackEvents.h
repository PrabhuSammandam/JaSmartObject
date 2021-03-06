#pragma once

#include <StackMessage.h>
#include <cstdint>
#include "end_point.h"

namespace ja_iot {
namespace stack {
enum class SoStackMsgType : uint8_t
{
  NONE,
  ENDPOINT_DATA,
  SERVER_SEND_RESPONSE,
  CLIENT_SEND_REQUEST,
  HEART_BEAT_TIMER
};

class StackEvent
{
  public:
    StackEvent( const SoStackMsgType type ) : msg_type{ type } {}

    SoStackMsgType get_msg_type() const { return ( msg_type ); }
    void           set_msg_type( const SoStackMsgType msg_type ) { this->msg_type = msg_type; }

  private:
    SoStackMsgType   msg_type;
};

class EndpointDataStackEvent : public StackEvent
{
  public:
    explicit EndpointDataStackEvent( const ja_iot::network::Endpoint &end_point, uint8_t *pu8_data, const uint16_t u16_data_len ) :
      StackEvent{ SoStackMsgType::ENDPOINT_DATA },
      endpoint{ end_point },
      data{ pu8_data },
      data_len{ u16_data_len }
    {
    }
    ~EndpointDataStackEvent ()
    {
    }

    ja_iot::network::Endpoint   endpoint;
    uint8_t *                   data;
    uint16_t                    data_len;
};

class SendServerResponseStackEvent : public StackEvent
{
  public:
    SendServerResponseStackEvent( ServerResponse *server_res ) :
      StackEvent{ SoStackMsgType::SERVER_SEND_RESPONSE }, _server_response{ server_res }
    {
    }

    ServerResponse *_server_response{};
};

class ClientSendRequestStackEvent : public StackEvent
{
  public:
    ClientSendRequestStackEvent( ClientRequest *client_request, client_response_cb client_response_cb ) :
      StackEvent{ SoStackMsgType::CLIENT_SEND_REQUEST },
      _client_request{ client_request },
      _client_response_cb{ client_response_cb }
    {
    }

    ClientRequest *      _client_request     = nullptr;
    client_response_cb   _client_response_cb = nullptr;
};

class HeartBeatTimerStackEvent : public StackEvent
{
  public:
    HeartBeatTimerStackEvent () : StackEvent{ SoStackMsgType::HEART_BEAT_TIMER }
    {
    }
};
}
}