/*
 * TokenProvider.h
 *
 *  Created on: Feb 14, 2018
 *      Author: psammand
 */

#pragma once
#include "coap/coap_msg.h"

namespace ja_iot {
	namespace stack {

		class TokenProvider
		{
		public:
			static void assign_next_token(ja_iot::network::CoapMsg& message);
		};
	}
}