/*
 * data_types.h
 *
 *  Created on: Sep 16, 2017
 *      Author: psammand
 */
#pragma once

#include <cstdint>

namespace ja_iot {
namespace network {

enum class DtlsPskType : uint8_t
{
	HINT,
	IDENTITY,
	KEY
};

enum class InterfaceStatusFlag
{
  UP,
  DOWN
};

enum class AddressingMethod
{
  UNICAST = 0,
  MULTICAST
};

enum class MsgDataType
{
  NONE = 0,
  REQ,
  RES,
  ERR,
  RES_FOR_RES
};

enum class ErrorValue
{
  OK = 0
};


enum class CoapMsgType
{
  CON = 0,
  NON,
  ACK,
  RST,
  NONE
};

enum class CoapMsgCode
{
  EMPTY                          = 0,
  GET                            = 1,
  POST                           = 2,
  PUT                            = 3,
  DEL                            = 4,
  CREATED_201                    = 65,
  DELETED_202                    = 66,
  VALID_203                      = 67,
  CHANGED_204                    = 68,
  CONTENT_205                    = 69,
  CONTINUE_231                   = 95,
  BAD_REQUEST_400                = 128,
  UNAUTHORIZED_401               = 129,
  BAD_OPTION_402                 = 130,
  FORBIDDEN_403                  = 131,
  NOT_FOUND_404                  = 132,
  METHOD_NOT_ALLOWED_405         = 133,
  NOT_ACCEPTABLE_406             = 134,
  REQUEST_ENTITY_INCOMPLETE_408  = 136,
  PRECONDITION_FAILED_412        = 140,
  REQUEST_ENTITY_TOO_LARGE_413   = 141,
  UNSUPPORTED_CONTENT_FORMAT_415 = 143,
  INTERNAL_SERVER_ERROR_500      = 160,
  NOT_IMPLEMENTED_501            = 161,
  BAD_GATEWAY_502                = 162,
  SERVICE_UNAVAILABLE_503        = 163,
  GATEWAY_TIMEOUT_504            = 164,
  PROXYING_NOT_SUPPORTED_505     = 165
};

enum class eContentFormat
{
  NONE              = 0xFFFF,
  TEXT__PLAIN       = 0,
  APP__LINK_FORMAT  = 40,
  APP__XML          = 41,
  APP__OCTET_STREAM = 42,
  APP__RDF_XML      = 43,
  APP__EXI          = 47,
  APP__JSON         = 50,
  APP__CBOR         = 60,
  APP__VND_OCF_CBOR = 10000
};


enum class ePacketType
{
  NONE = 0,
  REQ,
  RES,
  EMPTY,
  ERR,
  RES_FOR_RES
};

enum class eDeviceType
{
  NONE = 0,
  CLIENT,
  SERVER,
  CLIENT_SERVER,
  GATEWAY
};

enum class eRestMethod
{
  NONE = 0,
  GET,
  POST,
  PUT,
  DEL,
  OBSERVE,
  OBSERVE_ALL,
  PRESENCE,
  DISCOVER
};

enum class eQos
{
  NONE = 0,
  LOW,
  MEDIUM,
  HIGH,
};
}
}
