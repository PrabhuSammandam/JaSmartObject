/*
 * ErrCode.h
 *
 *  Created on: Jun 30, 2017
 *      Author: psammand
 */

#ifndef BASE_EXPORT_ERRCODE_H_
#define BASE_EXPORT_ERRCODE_H_

namespace ja_iot
{
  namespace base
  {
    enum class ErrCode
    {
      OK = 0,
      ERR,
      INVALID_PARAMS,
      OUT_OF_MEM,
      NULL_VALUE,
      SEND_DATA_FAILED,
      NOT_INITIALIZED,
      SOCKET_INVALID,
      MSG_FORMAT_ERROR,
      PAYLOAD_TOO_LARGE,
      INVALID_COAP_VERSION,
      INVALID_URI,
      NO_RESOURCE,
      WRONG_ENDPOINT,
      INVALID_REQUEST,
      RESOURCE_ERROR,
      RESOURCE_CREATED,
      RESOURCE_DELETED,
      RESOURCE_CHANGED,
      DUPLICATE_REQUEST,
      SLOW_RESPONSE,
      CONTINUE,
      NO_OBSERVERS
    };
  }
}


#endif /* BASE_EXPORT_ERRCODE_H_ */
