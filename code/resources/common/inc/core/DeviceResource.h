#pragma once
#include <string>
#include "BaseResource.h"

namespace ja_iot {
namespace resources {
using namespace stack;

class DeviceResource : public BaseResource
{
  public:
    DeviceResource ();

    bool    is_method_supported( uint8_t method ) override;
    uint8_t handle_get( ja_iot::stack::QueryContainer &query_container, Interaction *interaction ) override;

  private:
    void init();
};
}
}