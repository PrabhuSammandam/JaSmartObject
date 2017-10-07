/*
 * MsgProcessor.h
 *
 *  Created on: Sep 16, 2017
 *      Author: psammand
 */

#ifndef NETWORK_COMMON_INC_MSG_PROCESSOR_H_
#define NETWORK_COMMON_INC_MSG_PROCESSOR_H_

#include <adapter_mgr.h>
#include <ErrCode.h>
#include <data_types.h>
#include <packet.h>
#include <packet_event_handler.h>

using ErrCode = ja_iot::base::ErrCode;

namespace ja_iot {
namespace network {

class MsgProcessorImpl;

class MsgProcessor
{
  public:

    MsgProcessor ();

    ErrCode initialize( uint16_t u16_adapter_type );
    ErrCode terminate();
    ErrCode send_msg( Packet *pcz_packet );
    ErrCode set_packet_event_handler( PacketEventHandler *pcz_packet_event_handler );
    ErrCode handle_req_res_callbacks();

  private:

    MsgProcessorImpl * pimpl_ = nullptr;
};


}
}

#endif /* NETWORK_COMMON_INC_MSG_PROCESSOR_H_ */
