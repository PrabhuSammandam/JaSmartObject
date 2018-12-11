/*
 * SecureContextImpl.cpp
 *
 *  Created on: Dec 6, 2018
 *      Author: psammand
 */

#include <base_consts.h>
#include <base_utils.h>
#include <common/inc/logging_network.h>
#include <common/src/SecureContextImpl.h>
#include <data_types.h>
#include <end_point.h>
#include <mbedtls/asn1.h>
#include <mbedtls/oid.h>
#include <mbedtls/version.h>
#include <OsalMgr.h>
#include <ScopedMutex.h>
#include <SecureContextUtils.h>
#include <cstring>
#include <iterator>
#include <string>

#define __FILE_NAME__ "SecureContextImpl"

namespace ja_iot {
using namespace osal;
namespace network {
void SecureContextImpl::configure_client_cipher_suite( const Endpoint &peer_endpoint )
{
  /* if the adapter is IP or BTLE then select DTLS config and for tcp select TLS config */
  if( peer_endpoint.is_ip_adapter() || peer_endpoint.is_btle_adapter() )
  {
    configure_cipher_suite( _client_dtls_cfg );
  }
  else
  {
#ifdef _ENABLE_TLS_
    configure_cipher_suite( _client_tls_cfg );
#endif
  }
}
void SecureContextImpl::configure_server_cipher_suite( const Endpoint &peer_endpoint )
{
  /* if the adapter is IP or BTLE then select DTLS config and for tcp select TLS config */
  if( peer_endpoint.is_ip_adapter() || peer_endpoint.is_btle_adapter() )
  {
    configure_cipher_suite( _server_dtls_cfg );
  }
  else
  {
#ifdef _ENABLE_TLS_
    configure_cipher_suite( _server_tls_cfg );
#endif
  }
}

void SecureContextImpl::configure_cipher_suite( mbedtls_ssl_config &ssl_config )
{
  if( _cb_get_credential_type == nullptr )
  {
    return;
  }

  bool        use_psk         = false;
  bool        use_certificate = false;
  std::string identity;

  _cb_get_credential_type( identity, use_psk, use_certificate );

  if( ( _selected_cipher == SecureContextCipherType::ECDHE_PSK_WITH_AES_128_CBC_SHA256 )
    || ( use_psk == true ) )
  {
    init_psk_identity( ssl_config );
  }

  if( use_certificate )
  {
    // init_pkik();
  }

  /* clear the cipher array */
  memset( &_cipher_array[0], 0, ( (uint8_t) SecureContextCipherType::MAX ) * sizeof( SecureContextCipherType ) );

  uint8_t ciper_count = 0;

  /* first preference to user selected */
  if( _selected_cipher != SecureContextCipherType::MAX )
  {
    _cipher_array[ciper_count++] = intl_convert_from_stack_cipher_suite( _selected_cipher );
  }

  /* user requested PSK credential and also the selected is not ECDHE_PSK_WITH_AES_128_CBC_SHA256, then add it as second priority */
  if( ( use_psk == true )
    && ( _selected_cipher != SecureContextCipherType::ECDHE_PSK_WITH_AES_128_CBC_SHA256 ) )
  {
    _cipher_array[ciper_count++] = intl_convert_from_stack_cipher_suite( SecureContextCipherType::ECDHE_PSK_WITH_AES_128_CBC_SHA256 );
  }

  if( use_certificate == true )
  {
    for( uint8_t i = 0; i < (uint8_t) SecureContextCipherType::MAX - 1; i++ )
    {
      if( ( (SecureContextCipherType) i != SecureContextCipherType::ECDHE_PSK_WITH_AES_128_CBC_SHA256 )
        && ( (SecureContextCipherType) i != _selected_cipher ) )
      {
        _cipher_array[ciper_count++] = intl_convert_from_stack_cipher_suite( (SecureContextCipherType) i );
      }
    }
  }

  /* set the list of cipher to mbedtls */
  mbedtls_ssl_conf_ciphersuites( &ssl_config, &_cipher_array[0] );
}

SecureSession * SecureContextImpl::create_session_endpoint( const Endpoint &peer_endpoint, bool is_server_role )
{
  bool is_configure_failed  = false;
  auto new_session_endpoint = new SecureSession{};

  if( peer_endpoint.is_ip_adapter() || peer_endpoint.is_btle_adapter() )
  {
    is_configure_failed = new_session_endpoint->configure( ( is_server_role ) ? _server_dtls_cfg : _client_dtls_cfg ) != ErrCode::OK;
  }
  else
  {
#ifdef _ENABLE_TLS_
    is_configure_failed = new_session_endpoint->configure( ( is_server_role ) ? _server_tls_cfg : _client_tls_cfg ) != ErrCode::OK;
#endif
  }

  if( is_configure_failed == true )
  {
    delete_and_clear( new_session_endpoint );
  }
  else
  {
    new_session_endpoint->set_endpoint( peer_endpoint );
    new_session_endpoint->set_event_handler(_event_handler);
  }

  return ( new_session_endpoint );
}

bool SecureContextImpl::init_psk_identity( mbedtls_ssl_config &ssl_config )
{
  if( _cb_get_psk_credential == nullptr )
  {
    return ( false );
  }

  std::string identity;

  if( _cb_get_psk_credential( DtlsPskType::IDENTITY, std::string(), identity ) != ErrCode::OK )
  {
    return ( false );
  }

  if( 0 != mbedtls_ssl_conf_psk( &ssl_config, (const uint8_t *) identity.c_str(), 1, (const uint8_t *) identity.c_str(), identity.length() ) )
  {
    return ( false );
  }

  return ( true );
}

/**
 * This is callback function called from mbedtls. This api will get called during the handshake.
 *
 * @param pv_user_data
 * @param ssl_ctx
 * @param pu8_psk_identity		: identity of requesting object
 * @param u16_psk_identity_len	: identity length of requesting object
 * @return
 */
int intl_get_psk_credential( void *pv_user_data, mbedtls_ssl_context *ssl_ctx, const unsigned char *pu8_psk_identity, size_t u16_psk_identity_len )
{
  if( ( ssl_ctx == nullptr )
    || ( pu8_psk_identity == nullptr )
    || pv_user_data )
  {
    return ( -1 );
  }

  auto secure_context_impl = (SecureContextImpl *) pv_user_data;

  if( secure_context_impl->_cb_get_psk_credential == nullptr )
  {
    return ( -1 );
  }

  std::string pre_shared_key;

  if( secure_context_impl->_cb_get_psk_credential( DtlsPskType::KEY, std::string( (const char *) pu8_psk_identity, u16_psk_identity_len ), pre_shared_key ) == ErrCode::OK )
  {
    return ( mbedtls_ssl_set_hs_psk( ssl_ctx, (const uint8_t *) pre_shared_key.c_str(), pre_shared_key.length() ) );
  }

  return ( -1 );
}

/* COMPLETE */
/**
 * Initialize the mbedtls config structure.
 *
 * @param pst_ssl_config
 * @param transport_type
 * @param mode
 * @return
 */
ErrCode SecureContextImpl::init_config( mbedtls_ssl_config *pst_ssl_config, int transport_type, int mode )
{
  mbedtls_ssl_config_init( pst_ssl_config );

  if( mbedtls_ssl_config_defaults( pst_ssl_config, mode, transport_type, MBEDTLS_SSL_PRESET_DEFAULT ) != 0 )
  {
    DBG_INFO2( "Config initialization failed!" );
    return ( ErrCode::ERR );
  }

  mbedtls_ssl_conf_psk_cb( pst_ssl_config, intl_get_psk_credential, this );
  mbedtls_ssl_conf_rng( pst_ssl_config, mbedtls_ctr_drbg_random, &_random_ctx );
  mbedtls_ssl_conf_authmode( pst_ssl_config, MBEDTLS_SSL_VERIFY_REQUIRED );

  if( ( MBEDTLS_SSL_TRANSPORT_DATAGRAM == transport_type )
    && ( MBEDTLS_SSL_IS_SERVER == mode ) )
  {
    mbedtls_ssl_conf_dtls_cookies( pst_ssl_config, mbedtls_ssl_cookie_write, mbedtls_ssl_cookie_check, &_ssl_cookie_ctx );
  }

  // mbedtls_ssl_conf_dbg(ssl_config, DebugSsl, NULL);

  return ( ErrCode::OK );
}

ErrCode SecureContextImpl::initialize()
{
  _access_mutex = OsalMgr::Inst()->AllocMutex();

  if( _access_mutex == nullptr )
  {
    return ( ErrCode::OUT_OF_MEM );
  }

  DBG_INFO2( "mbedtls version %s", MBEDTLS_VERSION_STRING );

  mbedtls_entropy_init( &_entropy_ctx );
  mbedtls_ctr_drbg_init( &_random_ctx );
  mbedtls_pk_init( &_public_key_ctx );
  mbedtls_ssl_cookie_init( &_ssl_cookie_ctx );

  if( 0 != mbedtls_ctr_drbg_seed( &_random_ctx,
    mbedtls_entropy_func,
    &_entropy_ctx,
    nullptr,
    0 ) )
  {
    DBG_ERROR2( "Seed initialization failed!" );
    return ( ErrCode::ERR );
  }

  mbedtls_ctr_drbg_set_prediction_resistance( &_random_ctx, MBEDTLS_CTR_DRBG_PR_ON );

  if( 0 != mbedtls_ssl_cookie_setup( &_ssl_cookie_ctx,
    mbedtls_ctr_drbg_random,
    &_random_ctx ) )
  {
    DBG_ERROR2( "Cookie setup failed!" );
    return ( ErrCode::ERR );
  }

  if( init_config( &_client_dtls_cfg, MBEDTLS_SSL_TRANSPORT_DATAGRAM, MBEDTLS_SSL_IS_CLIENT ) != ErrCode::OK )
  {
    DBG_ERROR2( "client config failed!" );
    return ( ErrCode::ERR );
  }

  if( init_config( &_server_dtls_cfg, MBEDTLS_SSL_TRANSPORT_DATAGRAM, MBEDTLS_SSL_IS_SERVER ) != ErrCode::OK )
  {
    DBG_ERROR2( "server config failed!" );
    return ( ErrCode::ERR );
  }

#ifdef _ENABLE_TLS_

  if( init_config( &_client_tls_cfg, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_IS_CLIENT ) != ErrCode::OK )
  {
    DBG_ERROR2( "client config failed!" );
    return ( ErrCode::ERR );
  }

  if( init_config( &_server_tls_cfg, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_IS_SERVER ) != ErrCode::OK )
  {
    DBG_ERROR2( "server config failed!" );
    return ( ErrCode::ERR );
  }

#endif

#ifdef _ENABLE_CERT_
  mbedtls_x509_crt_init( &_cert_authority );
  mbedtls_x509_crt_init( &_crt );
  mbedtls_x509_crl_init( &_cert_revocation_list );
#endif

  return ( ErrCode::OK );
}

ErrCode SecureContextImpl::uninitialize()
{
	return (ErrCode::OK);
}

SecureSession * SecureContextImpl::get_session_endpoint( const Endpoint &peer_endpoint )
{
  ScopedMutex lock{ _access_mutex };

  for( auto loop_session_ep : _session_ep_list )
  {
    if( *loop_session_ep == peer_endpoint )
    {
      return ( loop_session_ep );
    }
  }

  return ( nullptr );
}

void SecureContextImpl::remove_session_endpoint( const Endpoint &peer_endpoint )
{
  ScopedMutex lock{ _access_mutex };

  bool found = false;
  int  idx   = -1;

  for( auto temp_session_ep : _session_ep_list )
  {
    idx++;

    if( temp_session_ep->get_endpoint() == (Endpoint &) peer_endpoint )
    {
      delete temp_session_ep;
      found = true;
      break;
    }
  }

  if( found )
  {
    _session_ep_list.erase( _session_ep_list.cbegin() + idx );
  }
}

void SecureContextImpl::remove_all_session_endpoints( uint16_t u16_adapter_type )
{
  ScopedMutex lock{ _access_mutex };

  for( auto ep : _session_ep_list )
  {
    if( ( u16_adapter_type == k_adapter_type_all )
      || ( ep->get_endpoint().get_adapter_type() == u16_adapter_type ) )
    {
      delete ep;
    }
  }

  _session_ep_list.clear();
}

ErrCode SecureContextImpl::extract_certificate_common_name( const mbedtls_x509_crt *peerCert )
{
  ErrCode res = ErrCode::OK;

#ifdef _ENABLE_CERT_
  auto    subject = (mbedtls_asn1_named_data *) &peerCert->subject;

  while( nullptr != subject )
  {
    if( 0 == MBEDTLS_OID_CMP( MBEDTLS_OID_AT_CN, &( subject->oid ) ) )
    {
      break;
    }

    subject = subject->next;
  }

  if( nullptr != _cb_certificate_common_name_verify )
  {
    if( nullptr != subject )
    {
      res = _cb_certificate_common_name_verify( std::string( (const char *) subject->val.p, subject->val.len ) );
    }
    else
    {
      res = _cb_certificate_common_name_verify( std::string() );
    }
  }

#endif

  return ( res );
}

void SecureContextImpl::set_cipher_suite( SecureContextCipherType e_cipher_suite )
{
  if( e_cipher_suite != SecureContextCipherType::MAX )
  {
    auto cipher = intl_convert_from_stack_cipher_suite( e_cipher_suite );

    if( cipher != 0 )
    {
      memset( &_cipher_array[0], 0, ( (uint8_t) SecureContextCipherType::MAX ) * sizeof( SecureContextCipherType ) );
      _selected_cipher = e_cipher_suite;
      _cipher_array[0] = cipher;
      mbedtls_ssl_conf_ciphersuites( &_client_dtls_cfg, _cipher_array );
      mbedtls_ssl_conf_ciphersuites( &_server_dtls_cfg, _cipher_array );
    }
  }
}

ErrCode SecureContextImpl::encrypt( Endpoint &endpoint, data_buffer_t& rcz_data_buffer )
{
  ScopedMutex lock{ _access_mutex };

  auto session_ep = get_session_endpoint( endpoint );

  if( session_ep == nullptr )
  {
    session_ep = create_session_endpoint( endpoint, false );

    if( session_ep == nullptr )
    {
      return ( ErrCode::ERR );
    }

    configure_client_cipher_suite( endpoint );

    if( !session_ep->initiate_handshake() )
    {
      delete session_ep;
      return ( ErrCode::ERR );
    }

    _session_ep_list.push_back( session_ep );
  }

  if( session_ep->encrypt( rcz_data_buffer ) == ErrCode::OK )
  {
    remove_session_endpoint( endpoint );
    return ( ErrCode::ERR );
  }

  return ( ErrCode::OK );
}

ErrCode SecureContextImpl::decrypt( Endpoint &endpoint, data_buffer_t& rcz_data_buffer )
{
  ScopedMutex lock{ _access_mutex };

  auto session_ep = get_session_endpoint( endpoint );

  if( session_ep == nullptr )
  {
    session_ep = create_session_endpoint( endpoint, true );

    if( session_ep == nullptr )
    {
      return ( ErrCode::ERR );
    }

    configure_server_cipher_suite( endpoint );
    _session_ep_list.push_back( session_ep );
  }

  if( !session_ep->is_handshake_over()
    && ( session_ep->initiate_server_handshake() == -1 ) )
  {
    remove_session_endpoint( endpoint );
    return ( ErrCode::ERR );
  }

  if( session_ep->is_handshake_over() )
  {
    return ( session_ep->decrypt( rcz_data_buffer ) );
  }

  return ( ErrCode::ERR );
}
}
}
