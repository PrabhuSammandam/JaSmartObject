#pragma once

#include <ErrCode.h>
#include <cstdint>

namespace ja_iot {
namespace ocfstack {
class OcfIResource;
class OcfDeviceInfo;
class OcfPlatformInfo;

class OcfIServer
{
  public:
    virtual ~OcfIServer () = default;

    virtual base::ErrCode start() = 0;
    virtual base::ErrCode stop()  = 0;

    virtual base::ErrCode register_resource( OcfIResource *pcz_resource )   = 0;
    virtual base::ErrCode unregister_resource( OcfIResource *pcz_resource ) = 0;

    virtual base::ErrCode get_supported_transport_schemes( uint16_t &u16_transport_schemes ) = 0;

    virtual base::ErrCode set_device_info(  OcfDeviceInfo &device_info ) = 0;

    virtual base::ErrCode set_platform_info(  OcfPlatformInfo &platform_info ) = 0;
};
}
}