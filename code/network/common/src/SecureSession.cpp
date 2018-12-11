/*
 * SecureSession.cpp
 *
 *  Created on: Dec 6, 2018
 *      Author: psammand
 */

#include <base_utils.h>
#include <mbedtls/ssl.h>
#include <mbedtls/ssl_internal.h>
#include <SecureContextUtils.h>
#include "common/inc/SecureSession.h"

namespace ja_iot {
namespace network {
bool intl_is_valid_ssl_status( int ret );
int  intl_send_data_cb( void *ctx, const unsigned char *buf, size_t len );
int  intl_receive_data_cb( void *ctx, unsigned char *buf, size_t len );
SecureSession::SecureSession ()
{
}
SecureSession::~SecureSession ()
{
  if( _ssl_ctx.state == MBEDTLS_SSL_HANDSHAKE_OVER )
  {
    int ret = 0;

    do
    {
      ret = mbedtls_ssl_close_notify( &_ssl_ctx );
    }
    while( MBEDTLS_ERR_SSL_WANT_WRITE == ret );
  }

  mbedtls_ssl_free( &_ssl_ctx );
  delete_list( _msg_cache_list );
}

/**
 * This function should be called when data is received in the secure socket.
 * It will decrypt the data and return the clear data.
 *
 * Before calling this function the ssl context should be initialized.
 *
 * @param rpu8_data
 * @param ru16_data_len
 * @return
 */
ErrCode SecureSession::decrypt( data_buffer_t &rcz_data_buffer )
{
  auto decrypted_data = new uint8_t[2048];
  int  ret_value      = 0;

  set_encrypted_data( rcz_data_buffer );

  do
  {
    ret_value = mbedtls_ssl_read( &_ssl_ctx, decrypted_data, 2048 );
  } while( MBEDTLS_ERR_SSL_WANT_READ == ret_value );

  if( ( MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY == ret_value ) ||
    ( ( MBEDTLS_ERR_SSL_FATAL_ALERT_MESSAGE == ret_value ) &&
    ( MBEDTLS_SSL_ALERT_LEVEL_FATAL == _ssl_ctx.in_msg[0] ) &&
    ( MBEDTLS_SSL_ALERT_MSG_CLOSE_NOTIFY == _ssl_ctx.in_msg[1] ) ) )
  {
    delete_and_clear( decrypted_data );
    return ( ErrCode::ERR );
  }

  if( _event_handler )
  {
    SecureContextEvent new_receive_to_event;
    new_receive_to_event._type           = SecureContextEventType::RECEIVE;
    new_receive_to_event._endpoint       = &_endpoint;
    new_receive_to_event._data._pu8_data = decrypted_data;
    new_receive_to_event._data._u16_data_len = (uint16_t)ret_value;

    _event_handler->handle_event( new_receive_to_event );
  }

  return ( ErrCode::OK );
}

/**
 * This function should be called when the upper protocol needs to send data that needs to be encrypted.
 * The passed data buffer is owned by this api, so care need to take for the caller, not to delete it.
 *
 * @param pu8_data
 * @param u16_data_len
 * @return
 */
ErrCode SecureSession::encrypt( data_buffer_t &rcz_data_buffer )
{
  if( is_handshake_over() )
  {
    uint16_t data_written = 0;

    do
    {
      auto ret = mbedtls_ssl_write( &_ssl_ctx, rcz_data_buffer._pu8_data + data_written, rcz_data_buffer._u16_data_len - data_written );

      if( ret < 0 )
      {
        if( MBEDTLS_ERR_SSL_WANT_WRITE != ret )
        {
          delete_and_clear( rcz_data_buffer._pu8_data );
          return ( ErrCode::ERR );
        }

        continue;
      }

      data_written += ret;
    } while( rcz_data_buffer._u16_data_len > data_written );

    delete_and_clear( rcz_data_buffer._pu8_data );
  }
  else
  {
    return ( cache_message( rcz_data_buffer._pu8_data, rcz_data_buffer._u16_data_len ) );
  }

  return ( ErrCode::OK );
}

int SecureSession::initiate_server_handshake()
{
  int ret_value = 0;

  while( !is_handshake_over() )
  {
    ret_value = mbedtls_ssl_handshake_step( &_ssl_ctx );

    if( ret_value == MBEDTLS_ERR_SSL_CONN_EOF )
    {
      break;
    }

    if( !intl_is_valid_ssl_status( ret_value ) )
    {
      return ( -1 );
    }

    if( MBEDTLS_ERR_SSL_HELLO_VERIFY_REQUIRED == ret_value )
    {
      mbedtls_ssl_session_reset( &_ssl_ctx );
      mbedtls_ssl_set_client_transport_id( &_ssl_ctx, (const uint8_t *) _endpoint.get_addr().get_addr(), _endpoint.get_addr().get_addr_size() );
      continue;
    }

    if( MBEDTLS_SSL_CLIENT_CHANGE_CIPHER_SPEC == _ssl_ctx.state )
    {
      memcpy( _master_key, _ssl_ctx.session_negotiate->master, sizeof( _master_key ) );
      _cipher = intl_convert_to_stack_cipher_suite( _ssl_ctx.session_negotiate->ciphersuite );
    }

    if( MBEDTLS_SSL_CLIENT_KEY_EXCHANGE == _ssl_ctx.state )
    {
      memcpy( _random_key, &_ssl_ctx.handshake->randbytes[0], sizeof( _random_key ) );
    }
  }

  return ( ret_value );
}


int SecureSession::initiate_handshake()
{
  int ret_value = 0;

  while( MBEDTLS_SSL_HANDSHAKE_OVER > _ssl_ctx.state )
  {
    ret_value = mbedtls_ssl_handshake_step( &_ssl_ctx );

    if( MBEDTLS_ERR_SSL_CONN_EOF == ret_value )
    {
      break;
    }
    else if( -1 == ret_value )
    {
      return ( ret_value );
    }

    if( !intl_is_valid_ssl_status( ret_value ) )
    {
      return ( -1 );
    }
  }

  return ( ret_value );
}

bool intl_is_valid_ssl_status( int ret )
{
  if( ( ret != 0 ) &&
    ( MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY != ret ) &&
    ( MBEDTLS_ERR_SSL_HELLO_VERIFY_REQUIRED != ret ) &&
    ( MBEDTLS_ERR_SSL_WANT_READ != ret ) &&
    ( MBEDTLS_ERR_SSL_WANT_WRITE != ret ) &&
    ( MBEDTLS_ERR_SSL_NON_FATAL != ret ) &&
    ( MBEDTLS_SSL_ALERT_MSG_USER_CANCELED != ret ) &&
    ( MBEDTLS_SSL_ALERT_MSG_NO_RENEGOTIATION != ret ) &&
    ( MBEDTLS_SSL_ALERT_MSG_UNSUPPORTED_EXT != ret ) &&
    ( MBEDTLS_SSL_ALERT_MSG_CLOSE_NOTIFY != ret ) &&
    ( MBEDTLS_SSL_ALERT_MSG_NO_CERT != ret ) &&
    ( MBEDTLS_SSL_ALERT_MSG_BAD_CERT != ret ) &&
    ( MBEDTLS_SSL_ALERT_MSG_UNSUPPORTED_CERT != ret ) &&
    ( MBEDTLS_SSL_ALERT_MSG_CERT_REVOKED != ret ) &&
    ( MBEDTLS_SSL_ALERT_MSG_CERT_EXPIRED != ret ) &&
    ( MBEDTLS_SSL_ALERT_MSG_CERT_UNKNOWN != ret ) &&
    ( MBEDTLS_SSL_ALERT_MSG_INAPROPRIATE_FALLBACK != ret ) &&
    ( MBEDTLS_SSL_ALERT_MSG_UNRECOGNIZED_NAME != ret ) &&
    ( MBEDTLS_SSL_ALERT_MSG_UNKNOWN_PSK_IDENTITY != ret ) &&
    ( MBEDTLS_SSL_ALERT_MSG_NO_APPLICATION_PROTOCOL != ret ) )
  {
    return ( false );
  }

  return ( true );
}

/************************************************************************************************************/

ErrCode SecureSession::configure( mbedtls_ssl_config &rst_ssl_config )
{
  /* initialize the ssl context based on the passed ssl config */
  if( 0 != mbedtls_ssl_setup( &_ssl_ctx, &rst_ssl_config ) )
  {
    return ( ErrCode::ERR );
  }

  /* setup the callback that are called during the decrypt & encrypt operation */
  mbedtls_ssl_set_bio( &_ssl_ctx, (void *) this, intl_send_data_cb, intl_receive_data_cb, nullptr );

  /* if the transport is UDP and the ssl context is for server, then set the client address in ssl context */
  if( MBEDTLS_SSL_TRANSPORT_DATAGRAM == rst_ssl_config.transport )
  {
    // mbedtls_ssl_set_timer_cb(&tep->ssl, &tep->timer, mbedtls_timing_set_delay, mbedtls_timing_get_delay);

    if( MBEDTLS_SSL_IS_SERVER == rst_ssl_config.endpoint )
    {
      if( 0 != mbedtls_ssl_set_client_transport_id( &_ssl_ctx, (const uint8_t *) _endpoint.get_addr().get_addr(), _endpoint.get_addr().get_addr_size() ) )
      {
        mbedtls_ssl_free( &_ssl_ctx );
        return ( ErrCode::ERR );
      }
    }
  }

  return ( ErrCode::OK );
}

ErrCode SecureSession::cache_message( uint8_t *pu8_data, uint16_t u16_data_len )
{
  _msg_cache_list.push_back( new data_buffer_t{ pu8_data, u16_data_len } );

  return ( ErrCode::OK );
}

/**
 * Callback from mbedtls. Care should be taken that memory was allocated in mbedtls and it should not be deleted.
 *
 * @param u8_encrypted_data
 * @param i16_encrypted_data_len
 * @return
 */
int SecureSession::mbedtls_send_encrypted_data_to_socket( const uint8_t *u8_encrypted_data, size_t i16_encrypted_data_len )
{
  int16_t final_data_len = (int16_t) ( i16_encrypted_data_len > INT_MAX ) ? INT_MAX : i16_encrypted_data_len;

  if( _event_handler )
  {
    SecureContextEvent new_send_to_event;
    new_send_to_event._type               = SecureContextEventType::SEND;
    new_send_to_event._endpoint           = &_endpoint;
    new_send_to_event._data._pu8_data     = (uint8_t*)u8_encrypted_data;
    new_send_to_event._data._u16_data_len = final_data_len;

    _event_handler->handle_event( new_send_to_event );

    return new_send_to_event._processed_data_len;
  }

  return ( -1 );
}

int SecureSession::mbedtls_receive_encoded_data_from_socket( const uint8_t *u8_receive_buf, size_t i16_receive_buf_len )
{
  int32_t retLen = ( ( _encrypted_data_len > _encrypted_data_copied ) ? ( _encrypted_data_len - _encrypted_data_copied ) : 0 );

  retLen = ( retLen < (int32_t) i16_receive_buf_len ) ? retLen : i16_receive_buf_len;

  if( retLen > INT_MAX )
  {
    return ( 0 );
  }

  memcpy( (void *) u8_receive_buf, _encrypted_data + _encrypted_data_copied, retLen );
  _encrypted_data_copied += retLen;

  return ( retLen );
}

int intl_send_data_cb( void *ctx, const unsigned char *buf, size_t len )
{
  if( ( ctx != nullptr ) && ( buf != nullptr ) )
  {
    return ( ( (SecureSession *) ctx )->mbedtls_send_encrypted_data_to_socket( buf, len ) );
  }

  return ( -1 );
}

int intl_receive_data_cb( void *ctx, unsigned char *buf, size_t len )
{
  if( ( ctx != nullptr ) && ( buf != nullptr ) )
  {
    return ( ( (SecureSession *) ctx )->mbedtls_receive_encoded_data_from_socket( buf, len ) );
  }

  return ( -1 );
}
}
}
