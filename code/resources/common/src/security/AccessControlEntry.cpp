/*
 * AccessControlEntry.cpp
 *
 *  Created on: Nov 20, 2018
 *      Author: psammand
 */

#include <util/ResourceUtil.h>
#include <algorithm>
#include "security/AccessControlEntry.h"
#include "base_utils.h"
#include "coap/coap_consts.h"

namespace ja_iot {
namespace resources {
using namespace ja_iot::stack;
void AceResource::encode_to_cbor( CborEncoder &cz_cbor_encoder )
{
  cz_cbor_encoder.start_map();

  cz_cbor_encoder.write_map_entry( "href", get_href() );
  cz_cbor_encoder.write_map_entry( "rt", get_types() );
  cz_cbor_encoder.write_map_entry( "if", get_interfaces() );

  if( get_wildcard() != ACL_WILDCARD_NONE )
  {
    if( get_wildcard() == ACL_WILDCARD_ALL_DISCOVERABLE )
    {
      cz_cbor_encoder.write_map_entry( "wc", std::string( "+" ) );
    }
    else if( get_wildcard() == ACL_WILDCARD_ALL_NON_DISCOVERABLE )
    {
      cz_cbor_encoder.write_map_entry( "wc", std::string( "-" ) );
    }
    else if( get_wildcard() == ACL_WILDCARD_ALL_RESOURCES )
    {
      cz_cbor_encoder.write_map_entry( "wc", std::string( "*" ) );
    }
  }

  cz_cbor_encoder.end_map();
}

void AceSubjectBase::encode_to_cbor( CborEncoder &cz_cbor_encoder )
{
  cz_cbor_encoder.write_string( "subject" );
  cz_cbor_encoder.start_map();
  do_encode_to_cbor( cz_cbor_encoder );
  cz_cbor_encoder.end_map();
}
void AceSubjectUuid::do_encode_to_cbor( CborEncoder &cz_cbor_encoder )
{
  std::string uuid_str;
  _uuid >> uuid_str;
  cz_cbor_encoder.write_map_entry( "uuid", uuid_str );
}
void AceSubjectRole::do_encode_to_cbor( CborEncoder &cz_cbor_encoder )
{
  cz_cbor_encoder.write_map_entry( "authority", _authority );
  cz_cbor_encoder.write_map_entry( "role", _role );
}
void AceSubjectConnection::do_encode_to_cbor( CborEncoder &cz_cbor_encoder )
{
  if( _connection == ACE_SUBJECT_CONNECTION_TYPE_ANONYMOUS_UNENCRYPTED )
  {
    cz_cbor_encoder.write_map_entry( "conntype", std::string( "anon-clear" ) );
  }
  else
  {
    cz_cbor_encoder.write_map_entry( "conntype", std::string( "auth-crypt" ) );
  }
}
AccessControlEntry::AccessControlEntry( uint16_t id ) : _id{ id }
{
}
AccessControlEntry::~AccessControlEntry ()
{
  delete_and_clear( _subject );
  delete_list( _resources_array );
  delete_list( _validities_array );
}

bool AccessControlEntry::operator == ( const AccessControlEntry &other )
{
  return ( _id == other._id && is_same( other ) );
}
bool AccessControlEntry::is_same( const AccessControlEntry &other )
{
  return ( _resources_array == other._resources_array
         && _permission == other._permission
         /* TODO && *_subject == *other._subject*/
         && _validities_array == other._validities_array );
}

void AccessControlEntry::encode_to_cbor( CborEncoder &cz_cbor_encoder )
{
  cz_cbor_encoder.start_map();
  cz_cbor_encoder.write_map_entry( "aceid", get_id() );

  if( !get_resources_array().empty() )
  {
    cz_cbor_encoder.write_string( "resources" );
    cz_cbor_encoder.start_array();

    for( auto res : get_resources_array() )
    {
      res->encode_to_cbor( cz_cbor_encoder );
    }

    cz_cbor_encoder.end_array();
  }

  cz_cbor_encoder.write_map_entry( "permission", get_permission() );

  if( get_subject() != nullptr )
  {
    get_subject()->encode_to_cbor( cz_cbor_encoder );
  }

  cz_cbor_encoder.end_map();
}

uint8_t AccessControlEntry::decode_from_cbor(ResRepresentation & cbor_ace_obj)
{
	/**************************** resources *****************************/
	RepObjectArray cbor_ace_res_obj_array;

	if (cbor_ace_obj.get_prop("resources", cbor_ace_res_obj_array))
	{
		_resources_array.reserve(cbor_ace_res_obj_array.size());
	}

	for (auto &cbor_ace_res_obj : cbor_ace_res_obj_array)
	{
		auto new_ace_resource = new AceResource{};

		cbor_ace_res_obj.get_prop("href", new_ace_resource->get_href());
		cbor_ace_res_obj.get_prop("rt", new_ace_resource->get_types());
		cbor_ace_res_obj.get_prop("if", new_ace_resource->get_interfaces());

		std::string str_prop;

		if (cbor_ace_res_obj.get_prop("wc", str_prop))
		{
			if (str_prop[0] == '*')
			{
				new_ace_resource->set_wildcard(ACL_WILDCARD_ALL_RESOURCES);
			}
			else if (str_prop[0] == '+')
			{
				new_ace_resource->set_wildcard(ACL_WILDCARD_ALL_DISCOVERABLE);
			}
			else if (str_prop[0] == '-')
			{
				new_ace_resource->set_wildcard(ACL_WILDCARD_ALL_NON_DISCOVERABLE);
			}
		}

		_resources_array.push_back(new_ace_resource);
	}

	_permission = (uint8_t)cbor_ace_obj.get_prop<long>("permission");

	ResRepresentation cbor_ace_subject_obj{};
	if (cbor_ace_obj.get_prop("subject", cbor_ace_subject_obj))
	{
		if (cbor_ace_subject_obj.has_prop("uuid"))
		{
			_subject = new AceSubjectUuid{};
			std::string str_prop;
			cbor_ace_subject_obj.get_prop("uuid", str_prop);

			((AceSubjectUuid *)_subject)->get_uuid() << str_prop;
		}
		else if (cbor_ace_subject_obj.has_prop("conntype"))
		{
			std::string str_prop;
			cbor_ace_subject_obj.get_prop("conntype", str_prop);
			uint8_t connection_type{};

			if (str_prop == "auth-crypt")
			{
				connection_type = ACE_SUBJECT_CONNECTION_TYPE_AUTHENTICATED_ENCRYPTED;
			}
			else if (str_prop == "anon-clear")
			{
				connection_type = ACE_SUBJECT_CONNECTION_TYPE_ANONYMOUS_UNENCRYPTED;
			}

			_subject = new AceSubjectConnection{ connection_type };
		}
		else if (cbor_ace_subject_obj.has_prop("role"))
		{
			_subject = new AceSubjectRole{};
			cbor_ace_subject_obj.get_prop("role", ((AceSubjectRole *)_subject)->get_role());
			cbor_ace_subject_obj.get_prop("authority", ((AceSubjectRole *)_subject)->get_authority());
		}
	}

	return COAP_MSG_CODE_CHANGED_204;
}
}
}
