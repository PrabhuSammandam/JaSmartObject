#pragma once

#include <cstdint>
#include <end_point.h>
#include <StackMessage.h>
#include "Exchange.h"

namespace ja_iot {
	namespace stack {

		class MessageSender
		{
		public:
			static void send_empty_msg(ja_iot::network::Endpoint &endpoint, uint8_t msg_type, uint16_t msg_id);
			static void send(BaseMessage *message);
			static void send(ja_iot::network::CoapMsg *message);
		};
	}
}