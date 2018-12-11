/*
 * ResourceUtil.h
 *
 *  Created on: Nov 21, 2018
 *      Author: psammand
 */

#pragma once

#include "cbor/CborEncoder.h"
#include "Uuid.h"

namespace ja_iot {
namespace resources {
using namespace ja_iot::base;
using namespace ja_iot::stack;

class ResourceUtil
{
  public:
    static void encode_uuid( const char *key, Uuid &uuid, CborEncoder &cz_cbor_encoder );
		static uint8_t method_to_permission(uint8_t method);
};
}
}