#pragma once

#include <cstdint>

namespace ja_iot {
namespace stack {
constexpr uint8_t OCF_RESOURCE_PROP_NONE                  = 0x00;
constexpr uint8_t OCF_RESOURCE_PROP_DISCOVERABLE          = 0x01;
constexpr uint8_t OCF_RESOURCE_PROP_OBSERVABLE            = 0x02;
constexpr uint8_t OCF_RESOURCE_PROP_ACTIVE                = 0x04;
constexpr uint8_t OCF_RESOURCE_PROP_SLOW                  = 0x08;
constexpr uint8_t OCF_RESOURCE_PROP_EXPLICIT_DISCOVERABLE = 0x10;
constexpr uint8_t OCF_RESOURCE_PROP_SECURE                = 0x20;

enum class ResInterfaceType : uint8_t
{
  BaseLine,
  LinksList,
  Batch,
  ReadOnly,
  ReadWrite,
  Actuator,
  Sensor,
  none
};
}
}