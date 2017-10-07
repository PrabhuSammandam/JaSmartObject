/*
 * packet_event_handler.h
 *
 *  Created on: Oct 6, 2017
 *      Author: psammand
 */

#pragma once

#include <packet.h>

namespace ja_iot {
namespace network {
class PacketEvent
{
  public:

    PacketEvent( Packet *pcz_packet ) : _pcz_packet{ pcz_packet } {}

    Packet* get_packet() { return ( _pcz_packet ); }
    void    set_packet( Packet *pcz_packet ) { this->_pcz_packet = pcz_packet; }

  private:
    Packet * _pcz_packet = nullptr;
};

class PacketEventHandler
{
  public:

    PacketEventHandler () {}

    virtual ~PacketEventHandler () {}

    virtual void handle_packet_event( PacketEvent *pcz_packet_event ) = 0;
};

#define DECLARE_PACKET_EVENT_HANDLER_CLASS( HNDLR_CLASS, HOST, HNDL_FUNC ) class HNDLR_CLASS : public ja_iot::network::PacketEventHandler \
{ \
    public: \
      HNDLR_CLASS( HOST * host ) : host_{ host } {} \
      void handle_packet_event( PacketEvent * pcz_packet_event ) override{ host_->HNDL_FUNC( pcz_packet_event ); } \
    private: \
      HOST * host_; \
}; \

}
}