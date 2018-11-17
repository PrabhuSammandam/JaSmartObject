/*
 * SecurityDataTypes.h
 *
 *  Created on: Nov 17, 2018
 *      Author: psammand
 */

#pragma once

#include "cstdint"

enum class DeviceOwnerXferMethodType : uint8_t
{
  JUST_WORKS               = 0,
  RANDOM_PIN               = 1,
  MANUFACTURER_CERTIFICATE = 2
};

constexpr uint8_t CREDENTIAL_TYPE_NONE                             = 0x00;
constexpr uint8_t CREDENTIAL_TYPE_SYMMETRIC_PAIR_WISE_KEY          = 0x01;
constexpr uint8_t CREDENTIAL_TYPE_SYMMETRIC_GROUP_KEY              = 0x02;
constexpr uint8_t CREDENTIAL_TYPE_ASYMMETRIC_SIGNING_KEY           = 0x04;
constexpr uint8_t CREDENTIAL_TYPE_ASYMMETRIC_SIGNING_KEY_WITH_CERT = 0x08;
constexpr uint8_t CREDENTIAL_TYPE_PIN_OR_PASSWORD                  = 0x10;
constexpr uint8_t CREDENTIAL_TYPE_ASYMMETRIC_ENCRYPTION_KEY        = 0x20;
