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

using ErrCode = ja_iot::base::ErrCode;

namespace ja_iot {
namespace network {
enum class ConnectivityManagerEventType
{
  NONE = 0,
  REQUEST,
  RESPONSE,
  ERROR
};

class ConnectivityManagerEvent
{
  public:

    ConnectivityManagerEvent( ConnectivityManagerEventType event_type );

  private:
    ConnectivityManagerEventType   event_type_ = ConnectivityManagerEventType::NONE;
};

class IConnectivityManagerEventHandler
{
  public:

    IConnectivityManagerEventHandler () {}

    virtual ~IConnectivityManagerEventHandler () {}

  public:
    virtual void handle_event( ConnectivityManagerEvent *connectivity_mgr_event ) = 0;
};

class ConnectivityManager
{
  public:
    static ConnectivityManager& Inst();

  public:
    ErrCode initialize( AdapterType adapter_types );
    ErrCode terminate();
    ErrCode start_listening_server();
    ErrCode stop_listening_server();
    ErrCode start_discovery_server();
    ErrCode select_network( AdapterType adapter_type );
    ErrCode unselect_network( AdapterType adapter_type );
    ErrCode send_request( Endpoint *end_point );
    ErrCode send_response( Endpoint *end_point );
    ErrCode handle_request_response();
    ErrCode set_event_handler( IConnectivityManagerEventHandler *event_handler );

  private:

    ConnectivityManager ();

    ~ConnectivityManager ();

  private:
    static ConnectivityManager *       p_instance_;
    bool                               is_initialized_ = false;
    IConnectivityManagerEventHandler * event_handler_  = nullptr;
};
}
}



#endif /* NETWORK_EXPORT_CONNECTIVITY_MGR_H_ */
