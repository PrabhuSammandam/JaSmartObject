/*
 * SecureContext.cpp
 *
 *  Created on: Nov 26, 2018
 *      Author: psammand
 */

#include <common/inc/SecureSession.h>
#include <common/src/SecureContextImpl.h>
#include <ScopedMutex.h>
#include <SecureContext.h>
#include <cstdint>

#define __FILE_NAME__ "SecureContext"

namespace ja_iot {
using namespace base;
namespace network {
SecureContext::SecureContext ()
{
  _impl = new SecureContextImpl{};
}

ErrCode SecureContext::close_connection( Endpoint &endpoint )
{
  _impl->remove_session_endpoint( endpoint );
  return ( ErrCode::OK );
}

ErrCode SecureContext::close_all_connection( uint16_t u16_adapter_type )
{
  _impl->remove_all_session_endpoints( u16_adapter_type );
  return ( ErrCode::OK );
}

/**
 * This function should be called when the upper protocol needs to send data that needs to be encrypted.
 *
 * @param endpoint
 * @param pu8_data
 * @param u16_data_len
 * @return
 */
ErrCode SecureContext::encrypt( Endpoint &endpoint, data_buffer_t &rcz_data_buffer )
{
  if( rcz_data_buffer.is_valid() )
  {
    return ( ErrCode::INVALID_PARAMS );
  }

  return ( _impl->encrypt( endpoint, rcz_data_buffer ) );
}

/**
 * This function should be called when data is received in the secure socket.
 * It will decrypt the data and return the clear data.
 *
 * @param endpoint
 * @param pu8_data
 * @param u16_data_len
 * @return
 */
ErrCode SecureContext::decrypt( Endpoint &endpoint, data_buffer_t &rcz_data_buffer )
{
  if( rcz_data_buffer.is_valid() )
  {
    return ( ErrCode::INVALID_PARAMS );
  }

  return ( _impl->decrypt( endpoint, rcz_data_buffer ) );
}

ErrCode SecureContext::initialize()
{
  return ( _impl->initialize() );
}

ErrCode SecureContext::uninitialize()
{
  return ( _impl->uninitialize() );
}

void SecureContext::set_psk_credential_cb( get_psk_credential_cb_t pfn_psk_credential_cb )
{
  _impl->_cb_get_psk_credential = pfn_psk_credential_cb;
}

void SecureContext::set_credential_type_cb( get_credential_type_cb_t pfn_get_credential_type_cb )
{
  _impl->_cb_get_credential_type = pfn_get_credential_type_cb;
}

void SecureContext::set_certificate_common_name_verify_cb( certificate_common_name_verify_cb_t pfn_certificate_common_name_verify_cb )
{
#ifdef _ENABLE_CERT_
  _impl->_cb_certificate_common_name_verify = pfn_certificate_common_name_verify_cb;
#endif
}

void SecureContext::set_cipher_suite( SecureContextCipherType e_cipher_suite )
{
  ScopedMutex lock{ _impl->_access_mutex };

  _impl->set_cipher_suite( e_cipher_suite );
}
}
}

void ja_iot::network::SecureContext::set_event_handler( uint16_t u16_adapter_type, ISecureContextEventHandler *event_handler )
{
  _impl->_event_handler = event_handler;
}