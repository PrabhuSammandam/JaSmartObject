/*
 * SecureContextUtils.h
 *
 *  Created on: Dec 6, 2018
 *      Author: psammand
 */
#include <SecureContextTypes.h>

namespace ja_iot {
namespace network {
SecureContextCipherType intl_convert_to_stack_cipher_suite( uint16_t u16_cipher_suite );
uint16_t                intl_convert_from_stack_cipher_suite( SecureContextCipherType e_cipher_suite );
uint8_t intl_get_event_cb_data__index_for_adapter( uint16_t u16_adapter_type );
}
}
