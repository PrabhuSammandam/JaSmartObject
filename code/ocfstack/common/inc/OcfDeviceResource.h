#pragma once
#include "OcfResource.h"
#include <string>

namespace ja_iot {
namespace ocfstack {
class OcfDeviceInfo;

class OcfDeviceResource : public OcfResource
{
  public:
    OcfDeviceResource ();

    uint8_t handle_request( OcfExchange &exchange ) override;
    void    set_device_info(OcfDeviceInfo &device_info );
    uint8_t get_representation( OcfResInterfaceType interface_type, ResRepresentation &representation ) override;
    uint8_t get_discovery_representation(ResRepresentation &representation ) override;

  private:
    void init();

    std::string   _name;// n
    std::string   _device_id;// di
    std::string   _server_version;// icv
    std::string   _data_model_version;// dmv
};
}
}