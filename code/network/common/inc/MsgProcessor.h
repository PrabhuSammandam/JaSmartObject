/*
 * MsgProcessor.h
 *
 *  Created on: Sep 16, 2017
 *      Author: psammand
 */

#ifndef NETWORK_COMMON_INC_MSGPROCESSOR_H_
#define NETWORK_COMMON_INC_MSGPROCESSOR_H_

#include <ErrCode.h>
#include <TransportType.h>

using ErrCode = ja_iot::base::ErrCode;

namespace ja_iot {
namespace network {
enum class MsgProcessorEventType
{
  REQUEST,
  RESPONSE,
  ERROR,
  NETWORK_CHANGED
};

class MsgProcessorEvent
{
  public:

    MsgProcessorEvent( MsgProcessorEventType msg_processor_event_type ) : msg_processor_event_type_{ msg_processor_event_type } {}

  private:
    MsgProcessorEventType   msg_processor_event_type_ = MsgProcessorEventType::REQUEST;
};

class MsgProcessorEventHandler
{
  public:

    MsgProcessorEventHandler ();

    virtual ~MsgProcessorEventHandler ();

    virtual void handle_msg_processor_event( MsgProcessorEvent *msg_processor_event ) = 0;
};

class MsgProcessor
{
  public:

    MsgProcessor ();

    ErrCode initialize( AdapterType adapter_type );
    ErrCode terminate();
    ErrCode set_msg_processor_event_handler( MsgProcessorEventHandler *msg_processor_event_handler );

  private:
    MsgProcessorEventHandler * msg_processor_event_handler_ = nullptr;
};
}
}



#endif /* NETWORK_COMMON_INC_MSGPROCESSOR_H_ */
