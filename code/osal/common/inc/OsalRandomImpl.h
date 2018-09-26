#pragma once

#include <cstdint>
#include "OsalError.h"

namespace ja_iot {
namespace osal {
class OsalRandomImpl
{
  public:
    static OsalError get_random_bytes( uint8_t *random_buffer, uint16_t buffer_len );
};
}
}