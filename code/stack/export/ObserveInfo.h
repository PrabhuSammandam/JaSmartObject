/*
 * ObserveInfo.h
 *
 *  Created on: Feb 15, 2018
 *      Author: psammand
 */

#pragma once
#include "IResource.h"
#include "coap/coap_msg.h"
#include "end_point.h"

class ObserveInfo
{
  public:
    ObserveInfo( ja_iot::stack::IResource *resource, ja_iot::network::CoapMsgToken token, ja_iot::network::Endpoint endpoint )
      : _resource{ resource },
      _token{ token },
      _endpoint{ endpoint }
    {
    }

    ja_iot::stack::IResource *      _resource;
    ja_iot::network::CoapMsgToken   _token;
    ja_iot::network::Endpoint       _endpoint;
};