#pragma once
#include "OcfMsgHeader.h"
#include "ResPropValue.h"

namespace ja_iot {
namespace ocfstack {
class OcfResponse : public OcfMsgHeader
{
  public:
    ResRepresentation& get_representation() { return ( _representation ); }

  private:
    std::string         _new_uri;
    ResRepresentation   _representation;
};
}
}