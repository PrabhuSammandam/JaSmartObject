/*
 * ObserverMgr.h
 *
 *  Created on: Feb 15, 2018
 *      Author: psammand
 */

#pragma once

#include <vector>
#include "ObserveInfo.h"
#include "IResource.h"
#include "coap/coap_msg.h"
#include "end_point.h"

class ObserverMgr
{
  public:
    static ObserverMgr& inst();

    ObserveInfo* find_observer( ja_iot::stack::IResource *resource, ja_iot::network::CoapMsgToken &token, ja_iot::network::Endpoint &endpoint );
    ObserveInfo* register_observer( ja_iot::stack::IResource *resource, ja_iot::network::CoapMsgToken &token, ja_iot::network::Endpoint &endpoint );
    void         unregister_observer( ja_iot::stack::IResource *resource, ja_iot::network::CoapMsgToken &token, ja_iot::network::Endpoint &endpoint );

  private:
    std::vector<ObserveInfo *>   _observers_list;

  private:
    ObserverMgr ();
    ~ObserverMgr ();
    static ObserverMgr * _p_instance;
    ObserverMgr( const ObserverMgr &other )                   = delete;
    ObserverMgr( ObserverMgr &&other ) noexcept               = delete;
    ObserverMgr & operator = ( const ObserverMgr &other )     = delete;
    ObserverMgr & operator = ( ObserverMgr &&other ) noexcept = delete;
};