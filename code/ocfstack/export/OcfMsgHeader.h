#pragma once

#include <cstdint>
#include "OcfMsgToken.h"
#include <vector>
#include "OcfHeaderOption.h"
#include <map>
#include "coap/coap_options_set.h"

namespace ja_iot {
namespace ocfstack {
enum class OcfMsgType : uint8_t
{
  CON,
  NON,
  ACK,
  RST
};

enum class OcfMsgCode : uint8_t
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
  PROXYING_NOT_SUPPORTED_505     = 165,
};

class OcfMsgHeader
{
  public:
    OcfMsgHeader() : _token{0,nullptr}{}
    OcfMsgCode             & get_code();
    void                   set_code( const OcfMsgCode code );
    OcfMsgType             & get_type() { return ( _type ); }
    void                   set_type( const OcfMsgType type ) { _type = type; }
    uint16_t               & get_id() { return ( _id ); }
    void                   set_id( const uint16_t id ) { _id = id; }
    OcfMsgToken            & get_token() { return ( _token ); }
    void                   set_token( const OcfMsgToken &token ) { _token = token; }
    network::CoapOptionsSet& get_options() { return ( _options ); }

  private:
    OcfMsgType                _type = OcfMsgType::CON;
    OcfMsgCode                _code = OcfMsgCode::EMPTY;
    uint16_t                  _id   = 0;
    OcfMsgToken               _token;
    network::CoapOptionsSet   _options;
};

inline OcfMsgCode & OcfMsgHeader::get_code()
{
  return ( _code );
}

inline void OcfMsgHeader::set_code( const OcfMsgCode code )
{
  _code = code;
}
}
}