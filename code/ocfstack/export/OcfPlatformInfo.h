#pragma once

#include <algorithm>
#include <string>

namespace ja_iot {
namespace ocfstack {
class OcfPlatformInfo
{
  public:
    OcfPlatformInfo( const std::string &platform_id, const std::string &manu_name )
      : _platform_id{ std::move( platform_id ) },
      _manu_name{ std::move( manu_name ) }
    {
    }

  private:
    std::string   _platform_id;
    std::string   _manu_name;
};
}
}
