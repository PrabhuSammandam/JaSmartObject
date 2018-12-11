/*
 * ResourceUtil.cpp
 *
 *  Created on: Nov 21, 2018
 *      Author: psammand
 */
#include <util/ResourceUtil.h>
#include "coap/coap_consts.h"
#include "security/SecurityDataTypes.h"

namespace ja_iot {
namespace resources {
void ResourceUtil::encode_uuid( const char *key, Uuid &uuid, CborEncoder &cz_cbor_encoder )
{
  std::string uuid_str;
  uuid >> uuid_str;
  cz_cbor_encoder.write_map_entry( key, uuid_str );
}
uint8_t ResourceUtil::method_to_permission(uint8_t method)
{
	switch (method)
	{
	case COAP_MSG_CODE_GET:
	{
		return (CRUDN_PERMISSION_RETRIEVE);
	}
	case COAP_MSG_CODE_POST:
	{
		return (CRUDN_PERMISSION_UPDATE);
	}
	case COAP_MSG_CODE_PUT:
	{
		return (CRUDN_PERMISSION_CREATE);
	}
	case COAP_MSG_CODE_DEL:
	{
		return (CRUDN_PERMISSION_DELETE);
	}
	default:
		return (CRUDN_PERMISSION_FULL_CONTROL);
	}
}
}
}
