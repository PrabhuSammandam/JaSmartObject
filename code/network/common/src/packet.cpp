/*
 * packet.cpp
 *
 *  Created on: Oct 6, 2017
 *      Author: psammand
 */

#include <packet.h>

namespace ja_iot {
namespace network {
Packet::Packet ()
{
}

Packet::Packet( const Packet &other )
{
}

ResponsePacket::ResponsePacket( const ResponsePacket &other )
{
}

ErrorPacket::ErrorPacket( const ErrorPacket &other )
{
}
}
}