#pragma once
#include "BaseResource.h"
#include "ResourceConsts.h"

namespace ja_iot {
namespace resources {
using namespace stack;

class WellKnownResource : public BaseResource
{
  public:
    WellKnownResource ();

    uint8_t handle_request( Interaction *interaction ) override;
    uint8_t get_discovery_representation( ResRepresentation &representation ) override;

  private:
    void init();

    uint8_t get_baseline_representation( ResRepresentation &representation );
    uint8_t get_links_list_representation( ResRepresentation &representation ) const;
};
}
}