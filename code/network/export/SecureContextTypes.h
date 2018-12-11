/*
 * SecureContextTypes.h
 *
 *  Created on: Dec 6, 2018
 *      Author: psammand
 */

#pragma once
#include <base_datatypes.h>
#include <string>
#include <data_types.h>
#include <end_point.h>
#include <ErrCode.h>

namespace ja_iot {
namespace network {
typedef ErrCode ( *get_psk_credential_cb_t ) ( DtlsPskType _dtls_psk_type, const std::string &psk_identity, std::string &result );
typedef ErrCode ( *certificate_common_name_verify_cb_t ) ( const std::string &common_name );
typedef void ( *get_credential_type_cb_t ) ( const std::string &device_id, bool &use_psk, bool &use_certificate );

enum class SecureContextCipherType : uint8_t
{
  RSA_WITH_AES_256_CBC_SHA256,
  RSA_WITH_AES_128_GCM_SHA256,
  ECDHE_ECDSA_WITH_AES_128_GCM_SHA256,
  ECDHE_ECDSA_WITH_AES_128_CCM_8,
  ECDHE_ECDSA_WITH_AES_128_CCM,
  ECDHE_ECDSA_WITH_AES_128_CBC_SHA256,
  ECDHE_ECDSA_WITH_AES_256_CBC_SHA384,
  ECDHE_ECDSA_WITH_AES_256_GCM_SHA384,
  ECDHE_PSK_WITH_AES_128_CBC_SHA256,
  ECDHE_RSA_WITH_AES_128_CBC_SHA256,
  ECDH_ANON_WITH_AES_128_CBC_SHA256,
  MAX
};

enum class SecureContextEventType : uint8_t
{
  RECEIVE,
  SEND,
  ERROR
};

class SecureContextEvent
{
  public:
    SecureContextEventType   _type = SecureContextEventType::ERROR;
    Endpoint *               _endpoint;
    data_buffer_t            _data;
    int16_t                  _processed_data_len = -1;
    uint16_t                 _result             = 0;
};

class ISecureContextEventHandler
{
  public:
    virtual ~ISecureContextEventHandler () {}
    virtual void handle_event( SecureContextEvent &rcz_secure_context_event ) = 0;
};

typedef void ( *secure_context_event_cb_t ) ( const SecureContextEvent &event, void *pv_user_data );
}
}
