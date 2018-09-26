/*
 * CborCodec.h
 *
 *  Created on: Feb 28, 2018
 *      Author: psammand
 */

#pragma once

#include <ErrCode.h>
#include <ResPropValue.h>
#include <vector>

namespace ja_iot {
namespace stack {
class CborCodec
{
  public:
    static ja_iot::base::ErrCode encode( ja_iot::stack::ResRepresentation &res_representation, uint8_t *&dst_buffer, uint16_t &dst_buffer_len );
    static ja_iot::base::ErrCode decode( uint8_t *src_buffer, uint16_t src_buffer_len, ja_iot::stack::ResRepresentation &res_representation );

		
};
}
}