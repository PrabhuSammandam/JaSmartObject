#pragma once

#include <ErrCode.h>
#include <cstdint>
#include <memory>
#include <OcfIServer.h>

namespace ja_iot {
namespace ocfstack {
class OcfPlatformConfig;
class BaseServerImpl;

class OcfBaseServer : public OcfIServer
{
  public:
    OcfBaseServer( OcfPlatformConfig platform_cfg );
    ~OcfBaseServer ();

    base::ErrCode start() override;
    base::ErrCode stop() override;
    base::ErrCode register_resource( OcfIResource *pcz_resource ) override;
    base::ErrCode unregister_resource( OcfIResource *pcz_resource ) override;
    base::ErrCode get_supported_transport_schemes( uint16_t &u16_transport_schemes ) override;

    base::ErrCode set_device_info(  OcfDeviceInfo &device_info ) override;
    base::ErrCode set_platform_info( OcfPlatformInfo &platform_info ) override;

  private:
    std::unique_ptr<BaseServerImpl>   _pimpl;
};
}
}