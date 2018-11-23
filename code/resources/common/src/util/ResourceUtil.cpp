/*
 * ResourceUtil.cpp
 *
 *  Created on: Nov 21, 2018
 *      Author: psammand
 */
#include <util/ResourceUtil.h>

namespace ja_iot {
namespace resources {
void ResourceUtil::encode_uuid( const char *key, Uuid &uuid, CborEncoder &cz_cbor_encoder )
{
  std::string uuid_str;
  uuid >> uuid_str;
  cz_cbor_encoder.write_map_entry( key, uuid_str );
}
}
}
