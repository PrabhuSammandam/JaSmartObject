/*
 * IAdapter.h
 *
 *  Created on: Jun 30, 2017
 *      Author: psammand
 */

#pragma once

#include <AdapterEvent.h>
#include <network_datatypes.h>
#include <vector>
#include <functional>
#include <cstdint>
#include "end_point.h"
#include "ErrCode.h"

namespace ja_iot {
using namespace base;
namespace network {
class IAdapterEventHandler
{
  public:
    virtual ~IAdapterEventHandler () {}
    virtual void handle_event( AdapterEvent &pcz_adapter_event ) = 0;
};

class IAdapter
{
  public:
    virtual ~IAdapter ()
    {
    }

    virtual ErrCode initialize() = 0;
    virtual ErrCode terminate()  = 0;

    virtual ErrCode start_adapter() = 0;
    virtual ErrCode stop_adapter()  = 0;

    virtual ErrCode start_server() = 0;
    virtual ErrCode stop_server()  = 0;

    virtual ErrCode start_listening() = 0;
    virtual ErrCode stop_listening()  = 0;

    virtual int32_t send_unicast_data( Endpoint &endpoint, const uint8_t *data, uint16_t data_length )   = 0;
    virtual int32_t send_multicast_data( Endpoint &endpoint, const uint8_t *data, uint16_t data_length ) = 0;

    virtual void     read_data()                                                                                 = 0;
    virtual uint16_t get_type()                                                                                  = 0;
//    virtual void     set_adapter_event_cb( pfn_adapter_event_cb pfn_adapter_event_callback, void *pv_user_data ) = 0;
    virtual ErrCode  get_endpoints_list( EndpointList &rcz_endpoint_list )                            = 0;
    virtual void     set_event_handler(IAdapterEventHandler* pcz_event_handler) = 0;
};
}
}
