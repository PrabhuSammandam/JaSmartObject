#pragma once

#include <cstdint>

namespace ja_iot {
namespace osal {
class OsTime
{
  public:
    static uint32_t get_current_time_us();
};
}
}