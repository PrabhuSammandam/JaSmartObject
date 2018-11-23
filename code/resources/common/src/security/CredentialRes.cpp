/*
 * CredentialRes.cpp
 *
 *  Created on: Nov 19, 2018
 *      Author: psammand
 */

#include <util/ResourceUtil.h>
#include "StackConsts.h"
#include "security/CredentialRes.h"
#include "security/ProvisioningStatusRes.h"
#include "ResourceMgr.h"
#include "CborCodec.h"
#include "base_utils.h"

namespace ja_iot {
namespace resources {
using namespace base;
CredentialRes::CredentialRes () : BaseResource( "/oic/sec/cred" )
{
  init();
}

bool CredentialRes::is_method_supported( uint8_t method )
{
  return ( method == COAP_MSG_CODE_GET || method == COAP_MSG_CODE_POST || method == COAP_MSG_CODE_DEL );
}

uint8_t CredentialRes::handle_get( QueryContainer &query_container, Interaction *interaction )
{
  std::vector<int> query_cred_id_list;

  for( auto &q : query_container.get_any_of_map() )
  {
    if( q.first == "credid" )
    {
      for( auto &credid_str : q.second )
      {
        query_cred_id_list.push_back( atoi( credid_str.c_str() ) );
      }
    }
  }

  for( auto &q : query_container.get_all_of_map() )
  {
    if( q.first == "credid" )
    {
      query_cred_id_list.push_back( atoi( q.second.c_str() ) );
      break;
    }
  }

  CborEncoder cz_encoder{ 256 };
  cz_encoder.start_map();

  if( !query_cred_id_list.empty() )
  {
    _cred_obj.encode_to_cbor( cz_encoder, query_cred_id_list );
  }
  else
  {
    _cred_obj.encode_to_cbor( cz_encoder );
  }

  cz_encoder.write_map_entry( "rt", get_types() );
  cz_encoder.write_map_entry( "if", get_interfaces() );
  cz_encoder.end_map();

  return ( BaseResource::set_cbor_response( interaction, cz_encoder.get_buf(), cz_encoder.get_buf_len() ) );
}

uint8_t CredentialRes::handle_post( QueryContainer &query_container, Interaction *interaction )
{
  auto ret_status = handle_post_intl( query_container, interaction );
  auto response   = new ServerResponse{};

  response->set_code( ret_status );
  interaction->set_server_response( response );

  return ( STACK_STATUS_OK );
}

uint8_t CredentialRes::handle_post_intl( QueryContainer &query_container, Interaction *interaction )
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
    if( !pstat_res->is_owner_transfer_state()
      || !pstat_res->is_soft_reset_state() )
    {
      return ( COAP_MSG_CODE_NOT_ACCEPTABLE_406 );
    }

    rep.get_prop( "rowneruuid", uuid );
    _cred_obj.get_resource_owner_uuid() << uuid;
  }

  /*
   * Rules for updating
   * 1. Passed aceid matches with existing aceid will replace the entire ace.
   * 2. Passed aceid does not match with existing aceid the new aceid will be added to the list.
   * 3. Data without aceid will be created and added with new unique id. Old deleted aceid should not be used.
   */
  bool is_created = false;

  std::vector<Credential *> temp_cred_list{};

  RepObjectArray cbor_creds_obj_list;

  if( rep.get_prop( "creds", cbor_creds_obj_list ) )
  {
    temp_cred_list.reserve( cbor_creds_obj_list.size() );

    for( auto &cbor_cred_obj : cbor_creds_obj_list )
    {
      uint16_t cred_id = 0;

      if( ( cred_id = (uint16_t) cbor_cred_obj.get_prop<long>( "credid", 0xFFFF ) ) != 0xFFFF )
      {
        _cred_obj.remove_credential( cred_id );
      }
      else
      {
        cred_id    = 90;                          // TODO assign new ace_id
        is_created = true;
      }

      auto new_cred = new Credential{ cred_id };

      if( new_cred->decode_from_cbor( cbor_cred_obj ) == COAP_MSG_CODE_BAD_REQUEST_400 )
      {
        delete new_cred;
        delete_list( temp_cred_list );
        return ( COAP_MSG_CODE_BAD_REQUEST_400 );
      }

      temp_cred_list.push_back( new_cred );
    }
  }

  for( auto ace : temp_cred_list )
  {
    _cred_obj.get_credentials_array().push_back( ace );
  }

  return ( is_created ? COAP_MSG_CODE_CREATED_201 : COAP_MSG_CODE_CHANGED_204 );
}

uint8_t CredentialRes::handle_delete( QueryContainer &query_container, Interaction *interaction )
{
  auto ret_status = handle_delete_intl( query_container, interaction );
  auto response   = new ServerResponse{};

  response->set_code( ret_status );
  interaction->set_server_response( response );

  return ( STACK_STATUS_OK );
}

uint8_t CredentialRes::handle_delete_intl( QueryContainer &query_container, Interaction *interaction )
{
  (void) interaction;
  auto pstat_res = (ProvisioningStatusRes *) ResourceMgr::inst().get_provisioning_status_res();

  if( ( pstat_res == nullptr )
    || pstat_res->is_reset_state()
    || pstat_res->is_normal_operation_state() )
  {
    return ( COAP_MSG_CODE_NOT_ACCEPTABLE_406 );
  }

  std::vector<int> query_cred_id_list;
  std::string      query_subject;
  bool             has_subject_query = false;

  for( auto &q : query_container.get_any_of_map() )
  {
    if( q.first == "credid" )
    {
      for( auto &aceid_str : q.second )
      {
        query_cred_id_list.push_back( atoi( aceid_str.c_str() ) );
      }
    }
  }

  for( auto &q : query_container.get_all_of_map() )
  {
    if( q.first == "credid" )
    {
      query_cred_id_list.push_back( atoi( q.second.c_str() ) );
      break;
    }
    else if( q.first == "subjectuuid" )
    {
      query_subject     = q.second;
      has_subject_query = true;
    }
  }

  uint8_t ret_status;

  if( !query_cred_id_list.empty() )
  {
    ret_status = _cred_obj.remove_credential( query_cred_id_list );
  }
  else if( has_subject_query )
  {
    ret_status = _cred_obj.remove_credential( query_subject );
  }
  else
  {
    ret_status = _cred_obj.remove_all_credential();
  }

  if( ret_status == COAP_MSG_CODE_DELETED_202 )
  {
    /* update storage */
  }

  return ( ret_status );
}

void CredentialRes::init()
{
  add_type( "oic.r.cred" );
  add_interface( "oic.if.baseline" );
  set_property( OCF_RESOURCE_PROP_DISCOVERABLE | OCF_RESOURCE_PROP_SECURE );
}

/*****************************************************************************************************************/
/**********************************   CredObject             *****************************************************/
/*****************************************************************************************************************/
CredObject::~CredObject ()
{
  for( auto cred : _credentials_array )
  {
    delete cred;
  }

  _credentials_array.clear();
}

void CredObject::encode_to_cbor( CborEncoder &cz_cbor_encoder )
{
  if( !_credentials_array.empty() )
  {
    cz_cbor_encoder.write_string( "creds" );
    cz_cbor_encoder.start_array();

    for( auto cred : _credentials_array )
    {
      cred->encode_to_cbor( cz_cbor_encoder );
    }

    cz_cbor_encoder.end_array();
  }

  ResourceUtil::encode_uuid( "rowneruuid", _resource_owner_uuid, cz_cbor_encoder );
}

void CredObject::encode_to_cbor( CborEncoder &cz_cbor_encoder, std::vector<int> &credid_list )
{
  if( !_credentials_array.empty() )
  {
    cz_cbor_encoder.write_string( "creds" );
    cz_cbor_encoder.start_array();

    for( auto cred : _credentials_array )
    {
      for( auto credid : credid_list )
      {
        if( cred->get_id() == credid )
        {
          cred->encode_to_cbor( cz_cbor_encoder );
        }
      }
    }

    cz_cbor_encoder.end_array();
  }

  ResourceUtil::encode_uuid( "rowneruuid", _resource_owner_uuid, cz_cbor_encoder );
}

uint8_t CredObject::remove_credential( std::vector<int> &cred_id_list )
{
  for( auto cred_id : cred_id_list )
  {
    remove_credential( cred_id );
  }

  return ( COAP_MSG_CODE_DELETED_202 );
}

uint8_t CredObject::remove_credential( const std::string &subject )
{
  int idx = -1;

  for( auto cred : _credentials_array )
  {
    idx++;

    if( cred->get_subject_uuid().to_string() == subject )
    {
      delete cred;
      break;
    }
  }

  if( idx != -1 )
  {
    _credentials_array.erase( _credentials_array.cbegin() + idx );
    return ( COAP_MSG_CODE_DELETED_202 );
  }

  return ( COAP_MSG_CODE_NOT_ACCEPTABLE_406 );
}

uint8_t CredObject::remove_credential( int credid )
{
  int idx = -1;

  for( auto cred : _credentials_array )
  {
    idx++;

    if( cred->get_id() == credid )
    {
      delete cred;
      break;
    }
  }

  if( idx != -1 )
  {
    _credentials_array.erase( _credentials_array.cbegin() + idx );
    return ( COAP_MSG_CODE_DELETED_202 );
  }

  return ( COAP_MSG_CODE_NOT_ACCEPTABLE_406 );
}

uint8_t CredObject::remove_all_credential()
{
  for( auto cred : _credentials_array )
  {
    delete cred;
  }

  _credentials_array.clear();

  return ( COAP_MSG_CODE_DELETED_202 );
}
}
}
