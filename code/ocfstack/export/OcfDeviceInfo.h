#pragma once

#include <algorithm>
#include <string>

namespace ja_iot {
namespace ocfstack {
class OcfDeviceInfo
{
  public:
    OcfDeviceInfo( const std::string &name, const std::string &device_id, const std::string &server_version, const std::string &data_model_version )
      : _name{ std::move( name ) },
      _device_id{ std::move( device_id ) },
      _server_version{ std::move( server_version ) },
      _data_model_version{ std::move( data_model_version ) }
    {
    }

    std::string& get_name() { return ( _name ); }
    std::string& get_device_id() { return ( _device_id ); }
    std::string& get_server_version() { return ( _server_version ); }
    std::string& get_data_model_version() { return ( _data_model_version ); }

  private:
    std::string   _name;// n
    std::string   _device_id;// di
    std::string   _server_version;// icv
    std::string   _data_model_version;// dmv
};
}
}