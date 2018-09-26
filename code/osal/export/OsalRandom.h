#pragma once

#include <cstdint>
#include "OsalError.h"

namespace ja_iot {
namespace osal {
class OsalRandom
{
  public:
    static uint32_t  get_random();
    static uint32_t  get_random_range( uint32_t range_first, uint32_t range_second );
    static OsalError get_random_bytes( uint8_t *random_buffer, uint16_t buffer_len );
};
}
}