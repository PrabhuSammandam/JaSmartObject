/*
 * SecureSession.h
 *
 *  Created on: Dec 6, 2018
 *      Author: psammand
 */

#pragma once
#include "base_utils.h"
#include "base_datatypes.h"
#include <end_point.h>
#include <ErrCode.h>
#include <mbedtls/ssl.h>
#include <SecureContextTypes.h>

namespace ja_iot {
using namespace base;
namespace network {
class SecureSession
{
  public:
    SecureSession ();
    ~SecureSession ();

    bool operator == ( const Endpoint &peer_endpoint );

    void    clear_cache();
    ErrCode cache_message( uint8_t *pu8_data, uint16_t u16_data_len );
    ErrCode decrypt( data_buffer_t &rcz_data_buffer );
    ErrCode encrypt( data_buffer_t &rcz_data_buffer );
    bool    is_handshake_over();
    Endpoint& get_endpoint();
    void    set_endpoint( const Endpoint &endpoint );
    int     mbedtls_send_encrypted_data_to_socket( const unsigned char *buf, size_t len );
    int     mbedtls_receive_encoded_data_from_socket( const unsigned char *buf, size_t len );
    int     initiate_handshake();
    int     initiate_server_handshake();
    ErrCode configure( mbedtls_ssl_config &ssl_config );
    void    set_encrypted_data( data_buffer_t &rcz_data_buffer );
    void    set_event_handler( ISecureContextEventHandler *event_handler );

  private:
    mbedtls_ssl_context            _ssl_ctx;
    Endpoint                       _endpoint;
    std::vector<data_buffer_t *>   _msg_cache_list;
    uint8_t *                      _encrypted_data        = nullptr;
    uint16_t                       _encrypted_data_len    = 0;
    uint16_t                       _encrypted_data_copied = 0;
    uint8_t                        _master_key[48];
    uint8_t                        _random_key[64];
    ISecureContextEventHandler *   _event_handler = nullptr;
    SecureContextCipherType        _cipher        = SecureContextCipherType::MAX;
};

inline void SecureSession::clear_cache()
{
  delete_list( _msg_cache_list );
}

inline bool SecureSession::is_handshake_over()
{
  return ( _ssl_ctx.state == MBEDTLS_SSL_HANDSHAKE_OVER );
}

inline ja_iot::network::Endpoint & SecureSession::get_endpoint()
{
  return ( _endpoint );
}

inline void SecureSession::set_endpoint( const Endpoint &endpoint )
{
  _endpoint = endpoint;
  _endpoint.set_network_flags( _endpoint.get_network_flags() | k_network_flag_secure );
}

inline void SecureSession::set_encrypted_data( data_buffer_t &rcz_data_buffer )
{
  _encrypted_data        = rcz_data_buffer._pu8_data;
  _encrypted_data_len    = rcz_data_buffer._u16_data_len;
  _encrypted_data_copied = 0;
}

inline bool SecureSession::operator == ( const Endpoint &peer_endpoint )
{
  return ( _endpoint == (Endpoint &) peer_endpoint );
}

inline void SecureSession::set_event_handler( ISecureContextEventHandler *event_handler )
{
  _event_handler = event_handler;
}
}
}