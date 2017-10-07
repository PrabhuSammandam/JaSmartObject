/*
 * ConnectivityManager.h
 *
 *  Created on: Sep 15, 2017
 *      Author: psammand
 */

#ifndef NETWORK_EXPORT_CONNECTIVITY_MGR_H_
#define NETWORK_EXPORT_CONNECTIVITY_MGR_H_

#include <end_point.h>
#include <ErrCode.h>
#include <data_types.h>
#include <packet_event_handler.h>
#include <packet.h>

using ErrCode = ja_iot::base::ErrCode;

namespace ja_iot {
namespace network {
class ConnectivityManagerImpl;

class ConnectivityManager
{
  public:
    static ConnectivityManager& Inst();

  public:
    ErrCode initialize( uint16_t adapter_types );
    ErrCode terminate();
    ErrCode start_listening_server();
    ErrCode stop_listening_server();
    ErrCode start_discovery_server();
    ErrCode select_network( uint16_t adapter_type );
    ErrCode unselect_network( uint16_t adapter_type );
    ErrCode send_request( RequestPacket* pcz_request_packet );
    ErrCode send_response( ResponsePacket* pcz_response_packet );
    ErrCode handle_request_response();
    ErrCode set_packet_event_handler( PacketEventHandler *pcz_packet_event_handler );

  private:

    ConnectivityManager ();

    ~ConnectivityManager ();

  private:
    static ConnectivityManager * p_instance_;
    ConnectivityManagerImpl *    pcz_impl = nullptr;
};
}
}



#endif /* NETWORK_EXPORT_CONNECTIVITY_MGR_H_ */
