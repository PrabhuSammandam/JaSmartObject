#pragma once

#include <cstdint>

namespace ja_iot {
namespace ocfstack {
enum class ObserveAction : uint8_t
{
  REGISTER   = 0,
  UNREGISTER = 1,
  NONE       = 2,
};

class OcfObserveInfo
{
  public:
    ObserveAction& get_action() { return ( _action ); }
    void         set_action( const ObserveAction action ) { _action = action; }
    uint8_t      & get_id() { return ( _id ); }
    void         set_id( const uint8_t id ) { _id = id; }

  private:
    ObserveAction   _action = ObserveAction::NONE;
    uint8_t         _id     = 0;
};
}
}