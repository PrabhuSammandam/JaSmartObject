/*
 * SoStackFacade.h
 *
 *  Created on: Oct 30, 2018
 *      Author: psammand
 */

#pragma once

#include "ErrCode.h"
#include "DeviceInfo.h"

namespace ja_iot {
namespace so_stack {
using namespace ja_iot::stack;

class SoStackFacade
{
  public:
    static SoStackFacade& inst();

  public:
    ja_iot::base::ErrCode initialize();

    ja_iot::base::ErrCode set_device_info( DeviceInfo &device_info );

    ja_iot::base::ErrCode start();

    SoStackFacade& enable_ipv4(bool is_enabled);
    SoStackFacade& enable_ipv4_mcast(bool is_enabled);
    SoStackFacade& enable_ipv6(bool is_enabled);
    SoStackFacade& enable_ipv6_mcast(bool is_enabled);
  private:
    SoStackFacade ();
    ~SoStackFacade ();

  private:
    static SoStackFacade * _p_instance;
    SoStackFacade( const SoStackFacade &other )                   = delete;
    SoStackFacade( SoStackFacade &&other ) noexcept               = delete;
    SoStackFacade & operator = ( const SoStackFacade &other )     = delete;
    SoStackFacade & operator = ( SoStackFacade &&other ) noexcept = delete;
};
}
}
