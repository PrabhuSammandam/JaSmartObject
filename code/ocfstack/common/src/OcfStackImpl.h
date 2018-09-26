#pragma once
#include "ErrCode.h"
#include "OcfStackRequestHandler.h"
#include "OcfPlatformConfig.h"
#include "Mutex.h"
#include "OcfDeviceInfo.h"
#include "OcfPlatformInfo.h"

namespace ja_iot {
namespace network {
class RequestPacket;
class ResponsePacket;
class ErrorPacket;
class Packet;
}
}

namespace ja_iot {
namespace ocfstack {
class OcfStackImpl
{
  public:
    OcfStackImpl ();
    virtual ~OcfStackImpl () = default;
    base::ErrCode initialise( OcfPlatformConfig platform_cfg );

    base::ErrCode handle_response( network::ResponsePacket *pcz_response_packet );
    base::ErrCode handle_errror( network::ErrorPacket *pcz_error_packet );

    void          handle_packet_event_cb( network::Packet *pcz_packet );
    base::ErrCode send_direct_response( network::ResponsePacket *pcz_response_packet );
    base::ErrCode check_and_process_received_data() const;
    base::ErrCode set_device_info(  OcfDeviceInfo &device_info ) const;
    base::ErrCode set_platform_info( OcfPlatformInfo &platform_info ) const;

  public:
    OcfPlatformConfig _platform_cfg{ network::eDeviceType::CLIENT_SERVER };
    bool _is_initialised{ false };
    osal::Mutex *_access_mutex{};

  private:
    OcfStackRequestHandler   _stack_req_handler;
};
}
}