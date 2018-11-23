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

/* oic.sec.credusagetype */
constexpr uint8_t CREDENTIAL_USAGE_TYPE_NONE         = 0x00;
constexpr uint8_t CREDENTIAL_USAGE_TYPE_TRUST_CA     = 0x01;
constexpr uint8_t CREDENTIAL_USAGE_TYPE_CERT         = 0x02;
constexpr uint8_t CREDENTIAL_USAGE_TYPE_ROLE_CERT    = 0x03;
constexpr uint8_t CREDENTIAL_USAGE_TYPE_MFG_TRUST_CA = 0x04;
constexpr uint8_t CREDENTIAL_USAGE_TYPE_MFG_CERT     = 0x05;

constexpr uint8_t CREDENTIAL_TYPE_NONE                             = 0x00;
constexpr uint8_t CREDENTIAL_TYPE_SYMMETRIC_PAIR_WISE_KEY          = 0x01;
constexpr uint8_t CREDENTIAL_TYPE_SYMMETRIC_GROUP_KEY              = 0x02;
constexpr uint8_t CREDENTIAL_TYPE_ASYMMETRIC_SIGNING_KEY           = 0x04;
constexpr uint8_t CREDENTIAL_TYPE_ASYMMETRIC_SIGNING_KEY_WITH_CERT = 0x08;
constexpr uint8_t CREDENTIAL_TYPE_PIN_OR_PASSWORD                  = 0x10;
constexpr uint8_t CREDENTIAL_TYPE_ASYMMETRIC_ENCRYPTION_KEY        = 0x20;

constexpr uint8_t CREDENTIAL_ENCODING_TYPE_JWT    = 0x00;
constexpr uint8_t CREDENTIAL_ENCODING_TYPE_CWT    = 0x01;
constexpr uint8_t CREDENTIAL_ENCODING_TYPE_BASE64 = 0x02;
constexpr uint8_t CREDENTIAL_ENCODING_TYPE_URI    = 0x03;
constexpr uint8_t CREDENTIAL_ENCODING_TYPE_PEM    = 0x04;
constexpr uint8_t CREDENTIAL_ENCODING_TYPE_DER    = 0x05;
constexpr uint8_t CREDENTIAL_ENCODING_TYPE_RAW    = 0x06;
constexpr uint8_t CREDENTIAL_ENCODING_TYPE_HANDLE = 0x07;

/* oic.sec.dostype, STATES */
constexpr uint8_t DEVICE_OPERATION_STATE_RESET                 = 0x00;// RESET
constexpr uint8_t DEVICE_OPERATION_STATE_OWNER_TRANSFER_METHOD = 0x01;// RFOTM
constexpr uint8_t DEVICE_OPERATION_STATE_PROVISIONING_STATE    = 0x02;// RFPRO
constexpr uint8_t DEVICE_OPERATION_STATE_NORMAL_OPERATION      = 0x03;// RFNOP
constexpr uint8_t DEVICE_OPERATION_STATE_SOFT_RESET            = 0x04;// SRESET

/* oic.sec.dpmtype */
constexpr uint8_t DEVICE_PROVISIONING_STATE_MANU_RESET                            = 0x01;
constexpr uint8_t DEVICE_PROVISIONING_STATE_DEVICE_PAIRING_OWNER_TRANSFER         = 0x02;
constexpr uint8_t DEVICE_PROVISIONING_STATE_PROVISIONING_CREDENTIAL_MGMT_SERVICES = 0x08;
constexpr uint8_t DEVICE_PROVISIONING_STATE_PROVISIONING_ACCESS_MGMT_SERVICES     = 0x10;
constexpr uint8_t DEVICE_PROVISIONING_STATE_PROVISIONING_LOCAL_ACLS               = 0x20;
constexpr uint8_t DEVICE_PROVISIONING_STATE_INITIATE_SW_VESRION_VALIDATION        = 0x40;
constexpr uint8_t DEVICE_PROVISIONING_STATE_INITIATE_SECURE_SW_UPDATE             = 0x80;

/* oic.sec.pomtype */
constexpr uint8_t DEVICE_PROVISIONING_MODE_SERVER_MULTIPLE = 0x01;
constexpr uint8_t DEVICE_PROVISIONING_MODE_SERVER_SINGLE   = 0x02;
constexpr uint8_t DEVICE_PROVISIONING_MODE_CLIENT          = 0x04;

constexpr uint8_t ACE_SUBJECT_TYPE_NONE       = 0x00;
constexpr uint8_t ACE_SUBJECT_TYPE_UUID       = 0x01;
constexpr uint8_t ACE_SUBJECT_TYPE_ROLE       = 0x02;
constexpr uint8_t ACE_SUBJECT_TYPE_CONNECTION = 0x03;

constexpr uint8_t ACE_SUBJECT_CONNECTION_TYPE_AUTHENTICATED_ENCRYPTED = 0x00;
constexpr uint8_t ACE_SUBJECT_CONNECTION_TYPE_ANONYMOUS_UNENCRYPTED   = 0x01;

constexpr uint8_t ACL_WILDCARD_NONE                 = 0x00;
constexpr uint8_t ACL_WILDCARD_ALL_DISCOVERABLE     = 0x01;
constexpr uint8_t ACL_WILDCARD_ALL_NON_DISCOVERABLE = 0x02;
constexpr uint8_t ACL_WILDCARD_ALL_RESOURCES        = 0x03;

constexpr uint8_t CRUDN_PERMISSION_NONE     = 0x00;
constexpr uint8_t CRUDN_PERMISSION_CREATE   = 0x01;
constexpr uint8_t CRUDN_PERMISSION_RETRIEVE = 0x02;
constexpr uint8_t CRUDN_PERMISSION_UPDATE   = 0x04;
constexpr uint8_t CRUDN_PERMISSION_DELETE   = 0x08;
constexpr uint8_t CRUDN_PERMISSION_NOTIFY   = 0x10;