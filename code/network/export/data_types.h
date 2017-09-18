/*
 * data_types.h
 *
 *  Created on: Sep 16, 2017
 *      Author: psammand
 */

#ifndef NETWORK_EXPORT_DATA_TYPES_H_
#define NETWORK_EXPORT_DATA_TYPES_H_

namespace ja_iot {
namespace network {

enum AdapterType
{
  TYPE_DEFAULT       = 0x00,
  IP            = 0x01,
  TCP           = 0x02,
  NFC           = 0x04,
  REMOTE_ACCESS = 0x08,
  BT_LE         = 0x10,
  BT_EDR        = 0x20,
  ALL           = 0xFF,
};

enum NetworkFlag
{
  FLAG_DEFAULT               = 0,
  SCOPE_INTERFACE_LOCAL = 1,
  SCOPE_LINK_LOCAL      = 2,
  SCOPE_REALM_LOCAL     = 3,
  SCOPE_ADMIN_LOCAL     = 4,
  SCOPE_SITE_LOCAL      = 5,
  SCOPE_ORG_LOCAL       = 8,
  SCOPE_GLOBAL          = 0xE,
  SECURE                = ( 1 << 4 ),
  IPV4                  = ( 1 << 5 ),
  IPV6                  = ( 1 << 6 ),
  MULTICAST             = ( 1 << 7 )
};

enum class InterfaceStatusFlag
{
  UP,
  DOWN
};

enum class ReqMethod
{
  NONE = 0,
  GET  = 1,
  POST,
  PUT,
  DEL
};

enum class ResResult
{
  EMPTY = 0
};

enum class ErrorValue
{
  OK = 0
};

class Request
{
  public:

    Request ();

  private:
    ReqMethod   method_   = ReqMethod::GET;
    bool        is_mcast_ = false;
};

class Response
{
  public:

    Response ();

  private:
    ResResult   result_  = ResResult::EMPTY;
    bool        is_mcast = false;
};

class Error
{
  public:

    Error ();

  private:
    ErrorValue   error_value_ = ErrorValue::OK;
};
}
}



#endif /* NETWORK_EXPORT_DATA_TYPES_H_ */
