#pragma once
#include "OcfResource.h"

namespace ja_iot {
namespace ocfstack {
class OcfWellKnownResource : public OcfResource
{
  public:
    OcfWellKnownResource ();

    uint8_t handle_request( OcfExchange &exchange ) override;
    uint8_t get_representation( OcfResInterfaceType interface_type, ResRepresentation &representation ) override;
    uint8_t get_discovery_representation( ResRepresentation &representation ) override;

  private:
    void init();

    uint8_t get_baseline_representation( ResRepresentation &representation );
    uint8_t get_links_list_representation( ResRepresentation &representation ) const;
};
}
}