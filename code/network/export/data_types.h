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
enum class ReqMethod
{
  NONE = 0,
  GET  = 1,
  POST,
  PUT,
  DELETE
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