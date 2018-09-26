#pragma once

#include <cstdint>
#include <data_types.h>
#include "base_consts.h"

namespace ja_iot {
namespace ocfstack {
class OcfPlatformConfig
{
  public:
    OcfPlatformConfig( const network::eDeviceType device_type, const uint16_t server_network_flags = base::k_network_flag_none, const uint16_t client_network_flags = base::k_network_flag_none, const uint16_t transport_flags = base::k_adapter_type_default )
      : _device_type{ device_type },
      _server_network_flags{ server_network_flags },
      _client_network_flags{ client_network_flags },
      _transport_flags{ transport_flags }
    {
    }

    bool is_server_enabled() const
    {
      return ( _device_type == network::eDeviceType::CLIENT_SERVER
             || _device_type == network::eDeviceType::SERVER
             || _device_type == network::eDeviceType::GATEWAY );
    }

    bool is_client_enabled() const
    {
      return ( _device_type == network::eDeviceType::CLIENT_SERVER
             || _device_type == network::eDeviceType::CLIENT
             || _device_type == network::eDeviceType::GATEWAY );
    }

    network::eDeviceType get_device_type() const { return ( _device_type ); }
    uint16_t             get_server_network_flags() const { return ( _server_network_flags ); }
    uint16_t             get_client_network_flags() const { return ( _client_network_flags ); }
    uint16_t             get_transport_flags() const { return ( _transport_flags ); }

  private:
    network::eDeviceType   _device_type          = network::eDeviceType::CLIENT_SERVER;
    uint16_t               _server_network_flags = base::k_network_flag_none;
    uint16_t               _client_network_flags = base::k_network_flag_none;
    uint16_t               _transport_flags      = base::k_adapter_type_default;
};
}
}