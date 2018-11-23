/*
 * AccessControlListRes.cpp
 *
 *  Created on: Nov 20, 2018
 *      Author: psammand
 */
#include <stdlib.h>
#include <util/ResourceUtil.h>
#include "StackConsts.h"
#include "security/AccessControlListRes.h"
#include "security/DeviceOwnerXferMethodRes.h"
#include "security/ProvisioningStatusRes.h"
#include "ResourceMgr.h"
#include "CborCodec.h"
#include "base_utils.h"
#include "cbor/CborEncoder.h"

namespace ja_iot {
namespace resources {
using namespace stack;

static uint8_t ACL_INTL_initialize_default_instance( AccessControlListRes &acl );
AccessControlListRes::AccessControlListRes () : BaseResource( "/oic/sec/acl2" )
{
  init();

  ACL_INTL_initialize_default_instance( *this );
}

bool AccessControlListRes::is_method_supported( uint8_t method )
{
  return ( method == COAP_MSG_CODE_GET || method == COAP_MSG_CODE_POST || method == COAP_MSG_CODE_DEL );
}

uint8_t AccessControlListRes::handle_get( QueryContainer &query_container, Interaction *interaction )
{
  std::vector<int> query_ace_id_list;

  for( auto &q : query_container.get_any_of_map() )
  {
    if( q.first == "aceid" )
    {
      for( auto &aceid_str : q.second )
      {
        query_ace_id_list.push_back( atoi( aceid_str.c_str() ) );
      }
    }
  }

  for( auto &q : query_container.get_all_of_map() )
  {
    if( q.first == "aceid" )
    {
      query_ace_id_list.push_back( atoi( q.second.c_str() ) );
      break;
    }
  }

  CborEncoder cz_cbor_encoder{ 1024 };
  cz_cbor_encoder.start_map();

  if( !query_ace_id_list.empty() )
  {
    _acl_obj.encode_to_cbor( cz_cbor_encoder, query_ace_id_list );
  }
  else
  {
    _acl_obj.encode_to_cbor( cz_cbor_encoder );
  }

  cz_cbor_encoder.write_map_entry( "rt", get_types() );
  cz_cbor_encoder.write_map_entry( "if", get_interfaces() );
  cz_cbor_encoder.end_map();

  return ( set_cbor_response( interaction, cz_cbor_encoder.get_buf(), cz_cbor_encoder.get_buf_len() ) );
}
uint8_t AccessControlListRes::handle_post( QueryContainer &query_container, Interaction *interaction )
{
  auto ret_status = handle_post_intl( query_container, interaction );

  auto response = new ServerResponse{};

  response->set_code( ret_status );
  interaction->set_server_response( response );

  return ( STACK_STATUS_OK );
}

uint8_t AccessControlListRes::handle_post_intl( QueryContainer &query_container, Interaction *interaction )
{
  (void) query_container;
  auto request = interaction->get_server_request();

  if( !request->has_payload() )
  {
    return ( COAP_MSG_CODE_BAD_REQUEST_400 );
  }

  auto pstat_res = (ProvisioningStatusRes *) ResourceMgr::inst().get_provisioning_status_res();

  if( ( pstat_res == nullptr )
    || pstat_res->is_reset_state()
    || pstat_res->is_normal_operation_state() )
  {
    return ( COAP_MSG_CODE_NOT_ACCEPTABLE_406 );
  }

  ResRepresentation rep;

  if( CborCodec::decode( request->get_payload(), request->get_payload_len(), rep ) != ErrCode::OK )
  {
    return ( COAP_MSG_CODE_INTERNAL_SERVER_ERROR_500 );
  }

  std::string uuid;

  if( rep.has_prop( "rowneruuid" ) )
  {
    if( pstat_res->is_provisioning_state() )
    {
      return ( COAP_MSG_CODE_NOT_ACCEPTABLE_406 );
    }

    rep.get_prop( "rowneruuid", uuid );
    _acl_obj.get_resource_owner_id() << uuid;
  }

  /*
   * Rules for updating
   * 1. Passed aceid matches with existing aceid will replace the entire ace.
   * 2. Passed aceid does not match with existing aceid the new aceid will be added to the list.
   * 3. Data without aceid will be created and added with new unique id. Old deleted aceid should not be used.
   */
  bool is_created = false;

  AccessControlList temp_ace_list{};

  if( rep.has_prop( "aclist2" ) )
  {
    RepObjectArray cbor_ace_obj_list;
    rep.get_prop( "aclist2", cbor_ace_obj_list );

    temp_ace_list.reserve( cbor_ace_obj_list.size() );

    for( auto &cbor_ace_obj : cbor_ace_obj_list )
    {
      uint16_t ace_id = 0;

      if( ( ace_id = (uint16_t) cbor_ace_obj.get_prop<long>( "aceid", 0xFFFF ) ) != 0xFFFF )
      {
        _acl_obj.remove_ace( ace_id );
      }
      else
      {
        ace_id     = 90;     // TODO assign new ace_id
        is_created = true;
      }

      auto new_ace = new AccessControlEntry{ ace_id };

      if( new_ace->decode_from_cbor( cbor_ace_obj ) != COAP_MSG_CODE_CHANGED_204 )
      {
        delete new_ace;
        delete_list( temp_ace_list );
        return ( COAP_MSG_CODE_BAD_REQUEST_400 );
      }

      temp_ace_list.push_back( new_ace );
    }
  }

  for( auto ace : temp_ace_list )
  {
    _acl_obj.get_ace_list().push_back( ace );
  }

  return ( is_created ? COAP_MSG_CODE_CREATED_201 : COAP_MSG_CODE_CHANGED_204 );
}

uint8_t AccessControlListRes::handle_delete( QueryContainer &query_container, Interaction *interaction )
{
  auto ret_status = handle_delete_intl( query_container, interaction );
  auto response   = new ServerResponse{};

  response->set_code( ret_status );
  interaction->set_server_response( response );

  return ( STACK_STATUS_OK );
}

uint8_t AccessControlListRes::handle_delete_intl( QueryContainer &query_container, Interaction *interaction )
{
  (void) interaction;

  auto pstat_res = (ProvisioningStatusRes *) ResourceMgr::inst().get_provisioning_status_res();

  if( ( pstat_res == nullptr )
    || pstat_res->is_reset_state()
    || pstat_res->is_normal_operation_state() )
  {
    return ( COAP_MSG_CODE_NOT_ACCEPTABLE_406 );
  }

  std::vector<int> query_ace_id_list;

  for( auto &q : query_container.get_any_of_map() )
  {
    if( q.first == "aceid" )
    {
      for( auto &aceid_str : q.second )
      {
        query_ace_id_list.push_back( atoi( aceid_str.c_str() ) );
      }
    }
  }

  for( auto &q : query_container.get_all_of_map() )
  {
    if( q.first == "aceid" )
    {
      query_ace_id_list.push_back( atoi( q.second.c_str() ) );
      break;
    }
  }

  uint8_t ret_status = COAP_MSG_CODE_DELETED_202;

  if( !query_ace_id_list.empty() )
  {
    for( auto ace_id : query_ace_id_list )
    {
      _acl_obj.remove_ace( ace_id );
    }

    ret_status = COAP_MSG_CODE_DELETED_202;
  }
  else
  {
    /* not called with aceid, so delete all the ace entries */
    ret_status = _acl_obj.remove_all_ace();
  }

  return ( ret_status );
}

void AccessControlListRes::set_resource_owner_uuid( const Uuid &uuid )
{
  _acl_obj.set_resource_owner_id( uuid );
  /* update storage */
}

AccessControlList AccessControlListRes::find_ace_by_connection( uint8_t connection_type )
{
  AccessControlList temp_ace_list;

  if( ( connection_type != ACE_SUBJECT_CONNECTION_TYPE_AUTHENTICATED_ENCRYPTED )
    || ( connection_type != ACE_SUBJECT_CONNECTION_TYPE_ANONYMOUS_UNENCRYPTED ) )
  {
    return ( temp_ace_list );
  }

  for( auto ace : _acl_obj.get_ace_list() )
  {
    if( ace->get_subject()->getType() == ACE_SUBJECT_TYPE_CONNECTION )
    {
      temp_ace_list.push_back( ace );
    }
  }

  return ( temp_ace_list );
}

void AccessControlListRes::init()
{
  add_type( "oic.r.acl2" );
  add_interface( "oic.if.baseline" );
  set_property( OCF_RESOURCE_PROP_DISCOVERABLE );
}

static uint8_t ACL_INTL_initialize_default_instance( AccessControlListRes &acl )
{
  auto temp_ace = new AccessControlEntry{ 1 };

  temp_ace->set_permission( CRUDN_PERMISSION_RETRIEVE );
  temp_ace->set_subject( new AceSubjectConnection{ ACE_SUBJECT_CONNECTION_TYPE_ANONYMOUS_UNENCRYPTED } );
  temp_ace->get_resources_array().push_back( new AceResource{ "/oic/res" } );
  temp_ace->get_resources_array().push_back( new AceResource{ "/oic/d" } );
  temp_ace->get_resources_array().push_back( new AceResource{ "/oic/p" } );
  acl.get_access_control_list().push_back( temp_ace );

  temp_ace = new AccessControlEntry{ 2 };
  temp_ace->set_permission( CRUDN_PERMISSION_RETRIEVE );
  temp_ace->set_subject( new AceSubjectConnection{ ACE_SUBJECT_CONNECTION_TYPE_AUTHENTICATED_ENCRYPTED } );
  temp_ace->get_resources_array().push_back( new AceResource{ "/oic/res" } );
  temp_ace->get_resources_array().push_back( new AceResource{ "/oic/d" } );
  temp_ace->get_resources_array().push_back( new AceResource{ "/oic/p" } );
  acl.get_access_control_list().push_back( temp_ace );

  temp_ace = new AccessControlEntry{ 3 };
  temp_ace->set_permission( CRUDN_PERMISSION_RETRIEVE |
    CRUDN_PERMISSION_UPDATE |
    CRUDN_PERMISSION_DELETE );
  temp_ace->set_subject( new AceSubjectConnection{ ACE_SUBJECT_CONNECTION_TYPE_ANONYMOUS_UNENCRYPTED } );
  temp_ace->get_resources_array().push_back( new AceResource{ "/oic/sec/doxm" } );
  acl.get_access_control_list().push_back( temp_ace );

  temp_ace = new AccessControlEntry{ 4 };
  temp_ace->set_permission( CRUDN_PERMISSION_RETRIEVE |
    CRUDN_PERMISSION_UPDATE |
    CRUDN_PERMISSION_DELETE );
  temp_ace->set_subject( new AceSubjectConnection{ ACE_SUBJECT_CONNECTION_TYPE_AUTHENTICATED_ENCRYPTED } );
  temp_ace->get_resources_array().push_back( new AceResource{ "/oic/sec/doxm" } );
  temp_ace->get_resources_array().push_back( new AceResource{ "/oic/sec/roles" } );
  acl.get_access_control_list().push_back( temp_ace );

  auto doxm_res = (DeviceOwnerXferMethodRes *) ResourceMgr::inst().get_device_owner_xfer_method_res();

  if( doxm_res != nullptr )
  {
    acl.set_resource_owner_uuid( doxm_res->get_resource_owner_uuid() );
  }
  else
  {
    /* TODO generate uuid */
  }

  return ( STACK_STATUS_OK );
}

/*****************************************************************************************************************/
/**********************************   Acl2Object             *****************************************************/
/*****************************************************************************************************************/

Acl2Object::~Acl2Object()
{
	for (auto ace : _ace_list)
	{
		delete ace;
	}
	_ace_list.clear();
}

void Acl2Object::encode_to_cbor( CborEncoder &cz_cbor_encoder )
{
  if( !_ace_list.empty() )
  {
    cz_cbor_encoder.write_string( "aclist2" );
    cz_cbor_encoder.start_array();

    for( auto ace : _ace_list )
    {
      ace->encode_to_cbor( cz_cbor_encoder );
    }

    cz_cbor_encoder.end_array();
  }

  ResourceUtil::encode_uuid( "rowneruuid", _resource_owner_uuid, cz_cbor_encoder );
}
void Acl2Object::encode_to_cbor( CborEncoder &cz_cbor_encoder, std::vector<int> &aceid_list )
{
  if( !_ace_list.empty() )
  {
    cz_cbor_encoder.write_string( "aclist2" );
    cz_cbor_encoder.start_array();

    for( auto ace : _ace_list )
    {
      for( auto id : aceid_list )
      {
        if( ace->get_id() == id )
        {
          ace->encode_to_cbor( cz_cbor_encoder );
        }
      }
    }

    cz_cbor_encoder.end_array();
  }

  ResourceUtil::encode_uuid( "rowneruuid", _resource_owner_uuid, cz_cbor_encoder );
}

uint8_t Acl2Object::remove_ace( int ace_id )
{
  int idx = -1;

  for( auto ace : _ace_list )
  {
    idx++;

    if( ace->get_id() == ace_id )
    {
      delete ace;
      break;
    }
  }

  if( idx != -1 )
  {
    _ace_list.erase( _ace_list.cbegin() + idx );
    return ( COAP_MSG_CODE_DELETED_202 );
  }

  return ( COAP_MSG_CODE_NOT_ACCEPTABLE_406 );
}

uint8_t Acl2Object::remove_all_ace()
{
  for( auto ace : _ace_list )
  {
    delete ace;
  }

  _ace_list.clear();

  return ( COAP_MSG_CODE_NOT_ACCEPTABLE_406 );
}
}
}
