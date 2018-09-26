#pragma once

#include <ErrCode.h>
#include <cstdint>
#include <memory>
#include "OcfResource.h"

namespace ja_iot {
namespace ocfstack {
class OcfPlatformConfig;
class OcfDeviceInfo;
class OcfPlatformInfo;
class OcfFacadeImpl;

class OcfFacade
{
  public:
    static OcfFacade& inst();
    base::ErrCode   initialise( OcfPlatformConfig platform_cfg ) const;
    base::ErrCode   start() const;
    base::ErrCode   stop() const;

    /* server interface */
    base::ErrCode get_supported_transport_schemes( uint16_t &u16_transport_schemes );
    base::ErrCode register_resource( OcfResource *pcz_resource );
    base::ErrCode unregister_resource( OcfResource *pcz_resource );
    base::ErrCode set_device_info(  OcfDeviceInfo &device_info ) const;
    base::ErrCode set_platform_info(  OcfPlatformInfo &platform_info ) const;

    /* client interface */

  private:
    static OcfFacade * _p_instance;
    std::unique_ptr<OcfFacadeImpl> _pimpl{};
  private:
    OcfFacade ();
    ~OcfFacade ();
    OcfFacade( const OcfFacade &other )                   = delete;
    OcfFacade( OcfFacade &&other ) noexcept               = delete;
    OcfFacade & operator = ( const OcfFacade &other )     = delete;
    OcfFacade & operator = ( OcfFacade &&other ) noexcept = delete;
};
}
}