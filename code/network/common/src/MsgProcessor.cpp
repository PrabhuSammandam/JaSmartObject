/*
 * MsgProcessor.cpp
 *
 *  Created on: Sep 16, 2017
 *      Author: psammand
 */

#include <common/inc/MsgProcessor.h>

using  namespace ja_iot::base;
//using namespace ja_iot::osal;
//using namespace ja_iot::network;
//using namespace ja_iot::memory;

namespace ja_iot {
namespace network {
MsgProcessor::MsgProcessor ()
{
}

ErrCode MsgProcessor::initialize( AdapterType adapter_type )
{
}


ErrCode MsgProcessor::terminate()
{
}

ErrCode MsgProcessor::set_msg_processor_event_handler( MsgProcessorEventHandler *msg_processor_event_handler )
{
	this->msg_processor_event_handler_ = msg_processor_event_handler;

	return ErrCode::OK;
}

MsgProcessorEventHandler::MsgProcessorEventHandler ()
{
}

MsgProcessorEventHandler::~MsgProcessorEventHandler ()
{
}

}
}
