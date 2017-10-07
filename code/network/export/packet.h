/*
 * packet.h
 *
 *  Created on: Sep 18, 2017
 *      Author: psammand
 */

#pragma once

#include <data_types.h>
#include <end_point.h>
#include <base_consts.h>
#include <coap/coap_msg.h>
#include <konstants_network.h>

namespace ja_iot {
namespace network {
class Packet
{
  public:

    Packet ();

    Packet( const Packet &other );

    PacketType get_packet_type() { return ( _packet_type ); }
    void       set_packet_type( PacketType packet_type ) { _packet_type = packet_type; }

    Endpoint* get_end_point() { return ( &_cz_end_point ); }
    CoapMsg*  get_msg() { return ( &_cz_coap_msg ); }

    uint8_t* get_resource_uri() { return ( &_resource_uri[0] ); }

    AddressingMethod get_addressing_method() const { return ( _addressing_method ); }
    void             set_addressing_method( AddressingMethod addressingMethod ) { _addressing_method = addressingMethod; }

    bool is_multicast_addressed()
    {
      if( ( _addressing_method == AddressingMethod::MULTICAST )
        && ( ( _cz_end_point.get_adapter_type() == ja_iot::base::kAdapterType_default )
           || ( _cz_end_point.get_adapter_type() == ja_iot::base::kAdapterType_all ) ) )
      {
        return ( true );
      }

      return ( false );
    }

  protected:
    PacketType         _packet_type       = PacketType::NONE;
    AddressingMethod   _addressing_method = AddressingMethod::UNICAST;
    Endpoint           _cz_end_point;
    CoapMsg            _cz_coap_msg;
    uint8_t            _resource_uri[k_resource_uri_path_max_size];
};

class RequestPacket : public ja_iot::network::Packet
{
  public:

    RequestPacket () { set_packet_type( PacketType::REQ ); }

    RequestPacket( const RequestPacket &other ) : Packet( other ) {}
};

class ResponsePacket : public ja_iot::network::Packet
{
  public:

    ResponsePacket () { set_packet_type( PacketType::RES ); }

    ResponsePacket( const ResponsePacket &other );
};

class ErrorPacket : public ja_iot::network::Packet
{
  public:

    ErrorPacket () { set_packet_type( PacketType::ERR ); }

    ErrorPacket( const ErrorPacket &other );
};
}
}
