/*
 * SecureContextImpl.h
 *
 *  Created on: Dec 6, 2018
 *      Author: psammand
 */

#pragma once

#include <common/inc/SecureSession.h>
#include <ErrCode.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/pk.h>
#include <mbedtls/ssl.h>
#include <mbedtls/ssl_cookie.h>
#include <mbedtls/x509_crl.h>
#include <mbedtls/x509_crt.h>
#include <SecureContextTypes.h>
#include <cstdint>
#include <vector>
#include <Mutex.h>

namespace ja_iot {
using namespace osal;

namespace network {
class SecureContextImpl
{
  public:
    ErrCode        initialize();
    ErrCode        uninitialize();
    ErrCode        init_config( mbedtls_ssl_config *ssl_config, int transport_type, int mode );
    SecureSession* get_session_endpoint( const Endpoint &peer_endpoint );
    SecureSession* create_and_initiate_handshake( const Endpoint &peer_endpoint );
    void           remove_session_endpoint( const Endpoint &peer_endpoint );
    void           remove_all_session_endpoints( uint16_t u16_adapter_type );
    ErrCode        extract_certificate_common_name( const mbedtls_x509_crt *peerCert );
    void           set_event_cb( uint16_t u16_adapter_type, secure_context_event_cb_t pfn_secure_context_event_cb, void *pv_user_data );
    bool           init_psk_identity( mbedtls_ssl_config &ssl_config );
    void           configure_cipher_suite( mbedtls_ssl_config &ssl_config );
    void           set_cipher_suite( SecureContextCipherType e_cipher_suite );
    ErrCode        decrypt( Endpoint &endpoint, data_buffer_t &rcz_data_buffer );
    ErrCode        encrypt( Endpoint &endpoint, data_buffer_t &rcz_data_buffer );

    SecureSession* create_session_endpoint( const Endpoint &peer_endpoint, bool is_server_role );
    void           configure_client_cipher_suite( const Endpoint &peer_endpoint );
    void           configure_server_cipher_suite( const Endpoint &peer_endpoint );

  public:
    std::vector<SecureSession *>   _session_ep_list;
    Mutex *                        _access_mutex    = nullptr;
    SecureContextCipherType        _selected_cipher = SecureContextCipherType::MAX;
    int32_t                        _active_cipher   = (int32_t) SecureContextCipherType::MAX;
    mbedtls_entropy_context        _entropy_ctx;
    mbedtls_ctr_drbg_context       _random_ctx;
    mbedtls_pk_context             _public_key_ctx;
    mbedtls_ssl_cookie_ctx         _ssl_cookie_ctx;
    get_psk_credential_cb_t        _cb_get_psk_credential  = nullptr;
    get_credential_type_cb_t       _cb_get_credential_type = nullptr;
    int32_t                        _cipher_array[(uint8_t) SecureContextCipherType::MAX] = { 0 };
    mbedtls_ssl_config             _client_dtls_cfg;
    mbedtls_ssl_config             _server_dtls_cfg;
    ISecureContextEventHandler *   _event_handler = nullptr;

    #ifdef _ENABLE_TLS_
    mbedtls_ssl_config   _client_tls_cfg;
    mbedtls_ssl_config   _server_tls_cfg;
#endif
#ifdef _ENABLE_CERT_
    certificate_common_name_verify_cb_t   _cb_certificate_common_name_verify = nullptr;
    mbedtls_x509_crt                      _cert_authority;
    mbedtls_x509_crt                      _crt;
    mbedtls_x509_crl                      _cert_revocation_list;
#endif
};
}
}
