/*
 * SecureContext.h
 *
 *  Created on: Nov 26, 2018
 *      Author: psammand
 */

#pragma once
#include <AdapterEvent.h>
#include <SecureContextTypes.h>
#include <vector>
#include "ErrCode.h"
#include "end_point.h"
#include "Mutex.h"
#include "data_types.h"

namespace ja_iot {
using namespace base;
using namespace osal;
namespace network {
class SecureContextImpl;

class SecureContext
{
  public:
    SecureContext ();
    ErrCode initialize();
    ErrCode uninitialize();
    ErrCode initiate_handshake( Endpoint &endpoint );
    ErrCode set_cipher_suite( const uint32_t u32_cipher );
    ErrCode decrypt( Endpoint &endpoint, data_buffer_t& rcz_data_buffer );
    ErrCode encrypt( Endpoint &endpoint, data_buffer_t& rcz_data_buffer );
    ErrCode close_connection( Endpoint &endpoint );
    ErrCode close_all_connection( uint16_t u16_adapter_type );

    void set_cipher_suite(SecureContextCipherType e_cipher_suite);
    void set_handshake_cb();
    void set_credential_type_cb( get_credential_type_cb_t pfn_get_credential_type_cb );
    void set_pkix_info_cb();
    void set_psk_credential_cb( get_psk_credential_cb_t pfn_psk_credential_cb );
    void set_certificate_common_name_verify_cb( certificate_common_name_verify_cb_t pfn_certificate_common_name_verify_cb );
    void set_event_handler(uint16_t u16_adapter_type, ISecureContextEventHandler* event_handler);

  private:
    SecureContextImpl * _impl           = nullptr;
    bool                _is_initialized = false;
};
}
}
