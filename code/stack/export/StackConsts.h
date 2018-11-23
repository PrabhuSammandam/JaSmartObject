#pragma once

#include <cstdint>

namespace ja_iot {
namespace stack {
constexpr uint8_t STACK_STATUS_OK                      = 0;
constexpr uint8_t STACK_STATUS_OUT_OF_MEMORY           = 1;
constexpr uint8_t STACK_STATUS_INVALID_INTERFACE_QUERY = 2;
constexpr uint8_t STACK_STATUS_INVALID_TYPE_QUERY      = 3;
constexpr uint8_t STACK_STATUS_INVALID_METHOD          = 4;
constexpr uint8_t STACK_STATUS_SLOW_RESPONSE           = 5;
constexpr uint8_t STACK_STATUS_REJECT                  = 6;
constexpr uint8_t STACK_STATUS_INVALID_PARAMETER       = 7;
constexpr uint8_t STACK_STATUS_BAD_REQUEST             = 8;
}
}