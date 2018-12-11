/*
 * base_datatypes.h
 *
 *  Created on: Dec 6, 2018
 *      Author: psammand
 */

#pragma once

#include <cstdint>

namespace ja_iot {
namespace base {
struct data_buffer_t
{
	data_buffer_t(){}
  data_buffer_t( uint8_t *pu8_data, uint16_t u16_data_len ) : _pu8_data{ pu8_data }, _u16_data_len{ u16_data_len } {}

  bool is_valid() { return ( _pu8_data != nullptr && _u16_data_len > 0 ); }

  uint8_t *  _pu8_data     = nullptr;
  uint16_t   _u16_data_len = 0;
};
}
}
