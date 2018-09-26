/*
 * MulticastClientInteraction.h
 *
 *  Created on: Feb 27, 2018
 *      Author: psammand
 */

#pragma once

#include <cstdint>
#include <coap/coap_msg.h>
#include <StackMessage.h>

namespace ja_iot {
	namespace stack {

		class MulticastClientInteraction
		{
		public:
			MulticastClientInteraction(ClientRequest *client_request);

			client_response_cb get_response_cb() { return (_client_response_cb); }
			void               set_response_cb(client_response_cb response_cb) { _client_response_cb = response_cb; }
			void               send_request();
			ClientRequest*     get_client_request() { return (_client_request); }

			void add_client_interaction() { _no_of_client_interactions++; }
			void delete_client_interaction() { _no_of_client_interactions--; }
			bool has_client_interactions() { return (_no_of_client_interactions != 0); }

		private:
			uint16_t             _no_of_client_interactions = 0;
			ClientRequest *      _client_request;
			client_response_cb   _client_response_cb = nullptr;
		};
	}
}