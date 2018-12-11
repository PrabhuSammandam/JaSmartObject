/*
 * SecureContextUtils.cpp
 *
 *  Created on: Dec 6, 2018
 *      Author: psammand
 */

#include <mbedtls/ssl_ciphersuites.h>
#include "SecureContextUtils.h"
namespace ja_iot {
namespace network {
SecureContextCipherType intl_convert_to_stack_cipher_suite( uint16_t u16_cipher_suite )
{
  switch( u16_cipher_suite )
  {
    case MBEDTLS_TLS_RSA_WITH_AES_256_CBC_SHA256:
    { return ( SecureContextCipherType::RSA_WITH_AES_256_CBC_SHA256 ); }
    case MBEDTLS_TLS_RSA_WITH_AES_128_GCM_SHA256:
    { return ( SecureContextCipherType::RSA_WITH_AES_128_GCM_SHA256 ); }
    case MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256:
    { return ( SecureContextCipherType::ECDHE_ECDSA_WITH_AES_128_GCM_SHA256 ); }
    case MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CCM_8:
    { return ( SecureContextCipherType::ECDHE_ECDSA_WITH_AES_128_CCM_8 ); }
    case MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CCM:
    { return ( SecureContextCipherType::ECDHE_ECDSA_WITH_AES_128_CCM ); }
    case MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256:
    { return ( SecureContextCipherType::ECDHE_ECDSA_WITH_AES_128_CBC_SHA256 ); }
    case MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384:
    { return ( SecureContextCipherType::ECDHE_ECDSA_WITH_AES_256_CBC_SHA384 ); }
    case MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384:
    { return ( SecureContextCipherType::ECDHE_ECDSA_WITH_AES_256_GCM_SHA384 ); }
    case MBEDTLS_TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA256:
    { return ( SecureContextCipherType::ECDHE_PSK_WITH_AES_128_CBC_SHA256 ); }
    case MBEDTLS_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256:
    { return ( SecureContextCipherType::ECDHE_RSA_WITH_AES_128_CBC_SHA256 );
      // case MBEDTLS_TLS_ECDH_ANON_WITH_AES_128_CBC_SHA256:return SecureContextCipherType::ECDH_ANON_WITH_AES_128_CBC_SHA256;
    }
    default: return ( SecureContextCipherType::MAX );
  }
}

uint16_t intl_convert_from_stack_cipher_suite( SecureContextCipherType e_cipher_suite )
{
  if( e_cipher_suite == SecureContextCipherType::RSA_WITH_AES_256_CBC_SHA256 )
  {
    return ( MBEDTLS_TLS_RSA_WITH_AES_256_CBC_SHA256 );
  }
  else if( e_cipher_suite == SecureContextCipherType::RSA_WITH_AES_128_GCM_SHA256 )
  {
    return ( MBEDTLS_TLS_RSA_WITH_AES_128_GCM_SHA256 );
  }
  else if( e_cipher_suite == SecureContextCipherType::ECDHE_ECDSA_WITH_AES_128_GCM_SHA256 )
  {
    return ( MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256 );
  }
  else if( e_cipher_suite == SecureContextCipherType::ECDHE_ECDSA_WITH_AES_128_CCM_8 )
  {
    return ( MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CCM_8 );
  }
  else if( e_cipher_suite == SecureContextCipherType::ECDHE_ECDSA_WITH_AES_128_CCM )
  {
    return ( MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CCM );
  }
  else if( e_cipher_suite == SecureContextCipherType::ECDHE_ECDSA_WITH_AES_128_CBC_SHA256 )
  {
    return ( MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256 );
  }
  else if( e_cipher_suite == SecureContextCipherType::ECDHE_ECDSA_WITH_AES_256_CBC_SHA384 )
  {
    return ( MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384 );
  }
  else if( e_cipher_suite == SecureContextCipherType::ECDHE_ECDSA_WITH_AES_256_GCM_SHA384 )
  {
    return ( MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384 );
  }
  else if( e_cipher_suite == SecureContextCipherType::ECDHE_PSK_WITH_AES_128_CBC_SHA256 )
  {
    return ( MBEDTLS_TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA256 );
  }
  else if( e_cipher_suite == SecureContextCipherType::ECDHE_RSA_WITH_AES_128_CBC_SHA256 )
  {
    return ( MBEDTLS_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256 );
  }
  else if( e_cipher_suite == SecureContextCipherType::ECDH_ANON_WITH_AES_128_CBC_SHA256 )
  {
    // return ( MBEDTLS_TLS_ECDH_ANON_WITH_AES_128_CBC_SHA256 );
  }

  return ( 0 );
}

uint8_t intl_get_event_cb_data__index_for_adapter( uint16_t u16_adapter_type )
{
  switch( u16_adapter_type )
  {
    case k_adapter_type_ip:
    { return ( 0 ); }
    case k_adapter_type_tcp:
    { return ( 1 ); }
    case k_adapter_type_bt_le: return ( 2 );
  }

  return ( -1 );
}

}
}
