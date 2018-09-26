#pragma once

#include "coap/coap_msg.h"

namespace ja_iot {
namespace network {

class CoapMsgPrinter
{
public:
	static void print_coap_msg(ja_iot::network::CoapMsg& coap_msg, uint8_t direction);
};

}
}
