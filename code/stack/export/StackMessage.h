#pragma once

#include <cstdint>
#include "coap/coap_msg.h"
#include "end_point.h"

constexpr uint8_t CLIENT_RESPONSE_STATUS_OK       = 0;
constexpr uint8_t CLIENT_RESPONSE_STATUS_REJECTED = 1;

namespace ja_iot {
namespace stack {
class ClientResponse;
typedef void ( *client_response_cb ) ( ClientResponse *client_response, uint8_t status );

class BaseMessage : public ja_iot::network::CoapMsg
{
  public:
    BaseMessage () :   ja_iot::network::CoapMsg{}
    {
    }
};

class EmptyMessage : public BaseMessage
{
  public:
    EmptyMessage () : BaseMessage{}
    {
    }
    EmptyMessage( uint8_t msg_type ) : BaseMessage{}
    {
      set_type( msg_type );
    }
    EmptyMessage( uint8_t msg_type, uint16_t msg_id, ja_iot::network::Endpoint &endpoint ) : BaseMessage{}
    {
      set_type( msg_type );
      set_id( msg_id );
      set_endpoint( endpoint );
    }
};

class ServerRequest : public BaseMessage
{
  public:
    ServerRequest () {}
    ServerRequest( ja_iot::network::CoapMsg *other )
    {
      *( (ja_iot::network::CoapMsg *) this ) = *other;
    }
};

class ServerResponse : public BaseMessage
{
  public:
    ServerResponse () : BaseMessage{}
    {
    }
};

class ClientRequest : public BaseMessage
{
  public:
    ClientRequest () {}
    ClientRequest( ja_iot::network::CoapMsg *other )
    {
      *( (ja_iot::network::CoapMsg *) this ) = *other;
    }
};

class ClientResponse : public BaseMessage
{
  public:
    ClientResponse () {}
    ClientResponse( ja_iot::network::CoapMsg *other )
    {
      *( (ja_iot::network::CoapMsg *) this ) = *other;
    }
};
}
}