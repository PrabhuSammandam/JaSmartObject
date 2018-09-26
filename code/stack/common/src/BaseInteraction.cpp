/*
 * BaseInteraction.cpp
 *
 *  Created on: Feb 14, 2018
 *      Author: psammand
 */

#include <base_utils.h>
#include <coap/coap_block_option.h>
#include <coap/coap_msg.h>
#include <coap/coap_options_set.h>
#include <common/inc/BaseInteraction.h>
#include <common/inc/BlockTransferStatus.h>
#include <Exchange.h>
#include <StackConfig.h>

using namespace ja_iot::base;
using namespace ja_iot::network;

namespace ja_iot {
	namespace stack {

		BaseInteraction::~BaseInteraction()
		{
			delete_and_clear<Exchange>(_current_in_exchange);
			delete_and_clear<Exchange>(_current_out_exchange);
			delete_and_clear<BlockTransferStatus>(_request_block_transfer_status);
			delete_and_clear<BlockTransferStatus>(_response_block_transfer_status);
		}

		Exchange * BaseInteraction::create_outgoing_exchange(CoapMsg *outgoing_msg)
		{
			_current_out_exchange = new Exchange{ outgoing_msg, EXCHANGE_ORIGIN_LOCAL };

			add_exchange(_current_out_exchange);

			if (outgoing_msg->is_confirmable())
			{
				_current_out_exchange->initiate_retransmission();
			}

			return (_current_out_exchange);
		}

		Exchange * BaseInteraction::create_incoming_exchange(CoapMsg *incoming_msg)
		{
			_current_in_exchange = new Exchange{ incoming_msg, EXCHANGE_ORIGIN_REMOTE };

			add_exchange(_current_in_exchange);

			return (_current_in_exchange);
		}

		void BaseInteraction::acknowledge()
		{
			if (_current_in_exchange != nullptr)
			{
				_current_in_exchange->acknowledge();
			}
		}

		void BaseInteraction::delete_current_in_exchange()
		{
			if (_current_in_exchange != nullptr)
			{
				// printf("BaseInteraction=> deleting in_exg\n");

				delete_exchange(_current_in_exchange);
				_current_in_exchange = nullptr;
			}
		}

		void BaseInteraction::delete_current_out_exchange()
		{
			if (_current_out_exchange != nullptr)
			{
				// printf("BaseInteraction=> deleting out_exg\n");
				delete_exchange(_current_out_exchange);
				_current_out_exchange = nullptr;
			}
		}

		BlockTransferStatus * BaseInteraction::create_request_block_transfer_status(CoapMsg *request_msg, bool is_downloading)
		{
			if (is_downloading)
			{
				/* large body is received for POST or PUT */
				auto &options = request_msg->get_option_set();
				auto max_resource_body_size = options.has_block1() && options.has_size1() ? options.get_size1() : MAX_RESOURCE_BODY_SIZE;
				_request_block_transfer_status = new BlockTransferStatus{};
				_request_block_transfer_status->set_block_size(options.get_block1().get_size());

				if (!_request_block_transfer_status->resize_buffer(max_resource_body_size))
				{
					delete_and_clear<BlockTransferStatus>(_request_block_transfer_status);
				}
			}
			else
			{
				/* large body is sent by POST or PUT */
				_request_block_transfer_status = new BlockTransferStatus{};
				_request_block_transfer_status->set_block_size(PREFERRED_BLOCK_SIZE);
			}

			return (_request_block_transfer_status);
		}

		BlockTransferStatus * BaseInteraction::create_response_block_transfer_status(CoapMsg *response_msg, bool is_downloading)
		{
			if (is_downloading)
			{
				auto &options = response_msg->get_option_set();
				auto max_resource_body_size = options.has_block2() && options.has_size2() ? options.get_size2() : MAX_RESOURCE_BODY_SIZE;
				_response_block_transfer_status = new BlockTransferStatus{};
				_response_block_transfer_status->set_block_size(PREFERRED_BLOCK_SIZE);

				if (!_response_block_transfer_status->resize_buffer(max_resource_body_size))
				{
					delete_and_clear<BlockTransferStatus>(_response_block_transfer_status);
				}
			}
			else
			{
				_response_block_transfer_status = new BlockTransferStatus{};
				_response_block_transfer_status->set_block_size(PREFERRED_BLOCK_SIZE);
			}

			return (_response_block_transfer_status);
		}

		void BaseInteraction::delete_request_block_transfer_status()
		{
			delete_and_clear<BlockTransferStatus>(_request_block_transfer_status);
		}

		void BaseInteraction::delete_response_block_transfer_status()
		{
			delete_and_clear<BlockTransferStatus>(_response_block_transfer_status);
		}

		void BaseInteraction::delete_exchange(Exchange *exchange)
		{
			if (_current_in_exchange == exchange)
			{
				_current_in_exchange = nullptr;
			}

			if (_current_out_exchange == exchange)
			{
				_current_out_exchange = nullptr;
			}
		}
	}
}



