#pragma once
#include <cstdint>
#include <string>

namespace ja_iot {
namespace ocfstack {
class OcfHeaderOption
{
  public:
    OcfHeaderOption( const uint16_t option_id, const std::string &option_data )
      : _option_id{ option_id },
      _option_data{ option_data }
    {
    }

    uint16_t   & get_option_id() { return ( _option_id ); }
    std::string& get_option_data() { return ( _option_data ); }

  private:
    uint16_t      _option_id;
    std::string   _option_data;
};
}
}