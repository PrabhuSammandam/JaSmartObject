#pragma once
#include <string>
#include "BaseResource.h"
#include "Exchange.h"
#include "DeviceInfo.h"

namespace ja_iot {
namespace stack {
class OcfDeviceInfo;

class DeviceResource : public BaseResource
{
  public:

    DeviceResource ();

    uint8_t get_representation( ResInterfaceType interface_type, ResRepresentation &representation ) override;
    bool    is_method_supported( uint8_t method ) override;
    uint8_t handle_get( ja_iot::stack::QueryContainer &query_container, Interaction *interaction ) override;

  private:
    void init();
};
}
}