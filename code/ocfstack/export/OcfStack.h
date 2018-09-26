/*
 * Stack.h
 *
 *  Created on: Oct 10, 2017
 *      Author: psammand
 */

#pragma once

#include "packet.h"
#include <memory>
#include "ErrCode.h"

namespace ja_iot {
namespace ocfstack {
class OcfPlatformInfo;
class OcfDeviceInfo;
class OcfPlatformConfig;
class OcfStackImpl;

class OcfStack
{
  public:
    static OcfStack& inst();

  public:
    base::ErrCode initialise( OcfPlatformConfig platform_cfg ) const;
    bool          is_multicast_server_stopped();
    void          set_multicast_server_stopped( bool is_stopped );
    base::ErrCode send_direct_response( network::Packet *pcz_response_packet );
    base::ErrCode set_device_info( OcfDeviceInfo &device_info ) const;
    base::ErrCode set_platform_info( OcfPlatformInfo &platform_info ) const;
    base::ErrCode check_and_process_received_data() const;

  private:
    OcfStack ();
    ~OcfStack ();

  private:
    static OcfStack *               _p_instance;
    std::unique_ptr<OcfStackImpl>   _pcz_impl;
};
}
}