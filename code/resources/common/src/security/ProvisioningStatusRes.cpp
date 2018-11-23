/*
 * ProvisioningStatusRes.cpp
 *
 *  Created on: Nov 18, 2018
 *      Author: psammand
 */
#include "security/ProvisioningStatusRes.h"
#include "security/SecurityDataTypes.h"
#include "security/DeviceOwnerXferMethodRes.h"
#include "security/AccessControlListRes.h"
#include "security/CredentialRes.h"

#include "StackConsts.h"
#include "CborCodec.h"
#include "cbor/CborDecoder.h"
#include "cbor/CborEncoder.h"
#include "base_utils.h"
#include "ResourceMgr.h"


namespace ja_iot {
namespace resources {
using namespace base;
constexpr const char *PSTAT_PROP_RT                  = "rt";
constexpr const char *PSTAT_PROP_IF                  = "if";
constexpr const char *PSTAT_PROP_DOS                 = "dos";
constexpr const char *PSTAT_PROP_DOS_S               = "s";
constexpr const char *PSTAT_PROP_DOS_P               = "p";
constexpr const char *PSTAT_PROP_IS_OPERATIONAL      = "isop";
constexpr const char *PSTAT_PROP_CURRENT_PROV_STATE  = "cm";
constexpr const char *PSTAT_PROP_TARGET_PROV_STATE   = "tm";
constexpr const char *PSTAT_PROP_CURRENT_OPER_MODE   = "om";
constexpr const char *PSTAT_PROP_SUPPORTED_OPER_MODE = "sm";
constexpr const char *PSTAT_PROP_ROWNERUUID          = "rowneruuid";

void update_pstat_object_by_post_data( PstatObject &dst, PstatObject &src );
ProvisioningStatusRes::ProvisioningStatusRes () : BaseResource( "/oic/sec/pstat" )
{
  init();
}

bool ProvisioningStatusRes::is_method_supported( uint8_t method )
{
  return ( method == COAP_MSG_CODE_GET || method == COAP_MSG_CODE_POST );
}

uint8_t ProvisioningStatusRes::handle_get( QueryContainer &query_container, Interaction *interaction )
{
  (void) query_container;
  CborEncoder cz_encoder{ 256 };

  cz_encoder.start_map();
  _pstat_obj.encode_to_cbor( cz_encoder );
  cz_encoder.write_map_entry( PSTAT_PROP_RT, get_types() );
  cz_encoder.write_map_entry( PSTAT_PROP_IF, get_interfaces() );
  cz_encoder.end_map();

  return ( BaseResource::set_cbor_response( interaction, cz_encoder.get_buf(), cz_encoder.get_buf_len() ) );
}

uint8_t ProvisioningStatusRes::handle_post( QueryContainer &query_container, Interaction *interaction )
{
  auto ret_status = handle_post_intl( query_container, interaction );

  auto response = new ServerResponse{};

  response->set_code( ret_status );
  interaction->set_server_response( response );

  return ( STACK_STATUS_OK );
}
uint8_t ProvisioningStatusRes::handle_post_intl( QueryContainer &query_container, Interaction *interaction )
{
  (void) query_container;
  auto request = interaction->get_server_request();

  if( !request->has_payload() )
  {
    return ( COAP_MSG_CODE_BAD_REQUEST_400 );
  }

  ResRepresentation rep;

  if( CborCodec::decode( request->get_payload(), request->get_payload_len(), rep ) != ErrCode::OK )
  {
    return ( COAP_MSG_CODE_INTERNAL_SERVER_ERROR_500 );
  }

  PstatObject pstat_obj{};
  uint8_t ret_status = pstat_obj.decode_from_cbor( rep );

  if( ret_status != COAP_MSG_CODE_CHANGED_204 )
  {
    return ( ret_status );
  }

  if( ( pstat_obj.get_state() != 0xFF )
    && ( !_pstat_obj.is_valid_transition( pstat_obj.get_state() )
       || _pstat_obj.is_pending() ) )
  {
    return ( COAP_MSG_CODE_NOT_ACCEPTABLE_406 );
  }

  if( ( pstat_obj.get_state() != 0xFF ) && !_pstat_obj.is_same_state( pstat_obj.get_state() ) )
  {
    if( handle_state_transition( pstat_obj.get_state() ) == COAP_MSG_CODE_NOT_ACCEPTABLE_406 )
    {
      return ( COAP_MSG_CODE_NOT_ACCEPTABLE_406 );
    }
  }

  update_pstat_object_by_post_data( _pstat_obj, pstat_obj );

  return ( COAP_MSG_CODE_CHANGED_204 );
}

uint8_t ProvisioningStatusRes::handle_state_transition( uint8_t u8_new_state )
{
  _pstat_obj.set_pending( true );

  auto &res_mgr                     = ResourceMgr::inst();
  auto device_owner_xfer_method_res = (DeviceOwnerXferMethodRes *) res_mgr.get_device_owner_xfer_method_res();

  switch( u8_new_state )
  {
    case DEVICE_OPERATION_STATE_RESET:
    {
      device_owner_xfer_method_res->set_owned( false );
      device_owner_xfer_method_res->get_device_owner_uuid().clear();
      device_owner_xfer_method_res->get_resource_owner_uuid().clear();

      auto acl_res = (AccessControlListRes *) res_mgr.get_access_control_list_res();

      if( acl_res != nullptr )
      {
        acl_res->get_resource_owner_uuid().clear();
      }

      auto cred_res = (CredentialRes *) res_mgr.get_credential_res();

      if( cred_res != nullptr )
      {
        cred_res->get_resource_owner_uuid().clear();
      }

      _pstat_obj.set_to_reset_state();
    }
    break;
    case DEVICE_OPERATION_STATE_OWNER_TRANSFER_METHOD:
    {
      if( device_owner_xfer_method_res->is_owned()
        || !device_owner_xfer_method_res->get_device_owner_uuid().is_nil() )
      {
        return ( COAP_MSG_CODE_NOT_ACCEPTABLE_406 );
      }

      _pstat_obj.set_to_owner_transfer_method_state();
    }
    break;
    case DEVICE_OPERATION_STATE_PROVISIONING_STATE:
    {
      if( !device_owner_xfer_method_res->is_owned()
        || device_owner_xfer_method_res->get_device_owner_uuid().is_nil()
        || device_owner_xfer_method_res->get_device_uuid().is_nil()
        || device_owner_xfer_method_res->get_resource_owner_uuid().is_nil()
        || _pstat_obj.get_resource_owner_id().is_nil() )
      {
        return ( COAP_MSG_CODE_NOT_ACCEPTABLE_406 );
      }

      _pstat_obj.set_to_provisioning_state();
    }
    break;
    case DEVICE_OPERATION_STATE_NORMAL_OPERATION:
    {
      if( !device_owner_xfer_method_res->is_owned()
        || device_owner_xfer_method_res->get_device_owner_uuid().is_nil()
        || device_owner_xfer_method_res->get_device_uuid().is_nil()
        || device_owner_xfer_method_res->get_resource_owner_uuid().is_nil()
        || _pstat_obj.get_resource_owner_id().is_nil()
        || _pstat_obj.is_device_operational() )
      {
        return ( COAP_MSG_CODE_NOT_ACCEPTABLE_406 );
      }

      _pstat_obj.set_to_normal_operation_state();
    }
    break;
    case DEVICE_OPERATION_STATE_SOFT_RESET:
    {
      if( !device_owner_xfer_method_res->is_owned()
        || device_owner_xfer_method_res->get_device_owner_uuid().is_nil()
        || device_owner_xfer_method_res->get_device_uuid().is_nil() )
      {
        return ( COAP_MSG_CODE_NOT_ACCEPTABLE_406 );
      }

      _pstat_obj.set_to_soft_reset_state();
    }
    break;
  }

  _pstat_obj.set_pending( false );

  return ( COAP_MSG_CODE_CHANGED_204 );
}

void ProvisioningStatusRes::init()
{
  _pstat_obj.set_to_default();
  add_type( "oic.r.pstat" );
  add_interface( "oic.if.baseline" );
  set_property( OCF_RESOURCE_PROP_DISCOVERABLE );
}

void update_pstat_object_by_post_data( PstatObject &dst, PstatObject &src )
{
  if( src.get_state() != 0xFF )
  {
    dst.set_state( src.get_state() );
  }

  if( src.get_cur_provisioning_mode() != 0xFF )
  {
    dst.set_cur_provisioning_mode( src.get_cur_provisioning_mode() );
  }

  if( src.get_target_provisioning_state() != 0xFF )
  {
    dst.set_target_provisioning_state( src.get_target_provisioning_state() );
  }

  if( !src.get_resource_owner_id().is_nil() )
  {
    dst.set_resource_owner_id( src.get_resource_owner_id() );
  }
}

/*****************************************************************************************************************/
/**********************************   PstatObject            *****************************************************/
/*****************************************************************************************************************/

void PstatObject::encode_to_cbor( CborEncoder &cz_cbor_encoder )
{
  cz_cbor_encoder.write( PSTAT_PROP_DOS );
  cz_cbor_encoder.write_map( 2 );
  cz_cbor_encoder.write_map_entry( PSTAT_PROP_DOS_S, get_state() );
  cz_cbor_encoder.write_map_entry( PSTAT_PROP_DOS_P, is_pending() );

  cz_cbor_encoder.write_map_entry( PSTAT_PROP_IS_OPERATIONAL, _is_device_operational );
  cz_cbor_encoder.write_map_entry( PSTAT_PROP_CURRENT_PROV_STATE, _current_provisioning_state );
  cz_cbor_encoder.write_map_entry( PSTAT_PROP_TARGET_PROV_STATE, _target_provisioning_state );
  cz_cbor_encoder.write_map_entry( PSTAT_PROP_CURRENT_OPER_MODE, _current_provisioning_mode );
  cz_cbor_encoder.write_map_entry( PSTAT_PROP_SUPPORTED_OPER_MODE, _supported_provisioning_mode );

  cz_cbor_encoder.write_map_entry( PSTAT_PROP_ROWNERUUID, _resource_owner_uuid.to_string() );
}

#define CHECK_NULL( _PTR_, _RET_VALUE_ ) if( _PTR_ == nullptr ){ return _RET_VALUE_; }
#define CHECK_NULL_CONTINUE( _PTR_ ) if( _PTR_ == nullptr ){ continue; }

uint8_t PstatObject::decode_from_cbor( ResRepresentation &res_rep )
{
  for( auto &prop_key : res_rep.get_props() )
  {
    CHECK_NULL( prop_key.second, COAP_MSG_CODE_NOT_ACCEPTABLE_406 );

    if( prop_key.first == PSTAT_PROP_DOS )
    {
      auto v        = prop_key.second;
      auto &dos_obj = v->get<ResRepresentation>();

      for( auto &dos_obj_prop : dos_obj.get_props() )
      {
        CHECK_NULL( dos_obj_prop.second, COAP_MSG_CODE_NOT_ACCEPTABLE_406 );

        if( dos_obj_prop.first == PSTAT_PROP_DOS_S )
        {
          _state = (uint8_t) dos_obj_prop.second->get<long>();
        }
        else if( dos_obj_prop.first == PSTAT_PROP_DOS_P )
        {
          return ( COAP_MSG_CODE_NOT_ACCEPTABLE_406 );
        }
      }
    }
    else if( ( prop_key.first == PSTAT_PROP_IS_OPERATIONAL )
           || ( prop_key.first == PSTAT_PROP_CURRENT_PROV_STATE )
           || ( prop_key.first == PSTAT_PROP_SUPPORTED_OPER_MODE ) )
    {
      return ( COAP_MSG_CODE_NOT_ACCEPTABLE_406 );
    }
    else if( prop_key.first == PSTAT_PROP_TARGET_PROV_STATE )
    {
      if( is_reset_state() )
      {
        return ( COAP_MSG_CODE_NOT_ACCEPTABLE_406 );
      }

      _target_provisioning_state = (uint8_t) prop_key.second->get<long>();
    }
    else if( prop_key.first == PSTAT_PROP_CURRENT_OPER_MODE )
    {
      if( is_reset_state() )
      {
        return ( COAP_MSG_CODE_NOT_ACCEPTABLE_406 );
      }

      _current_provisioning_mode = (uint8_t) prop_key.second->get<long>();
    }
    else if( prop_key.first == PSTAT_PROP_ROWNERUUID )
    {
      if( is_reset_state()
        || is_provisioning_state()
        || is_normal_operation_state() )
      {
        return ( COAP_MSG_CODE_NOT_ACCEPTABLE_406 );
      }

      _resource_owner_uuid << prop_key.second->get<std::string>();
    }
  }

  return ( COAP_MSG_CODE_CHANGED_204 );
}

void PstatObject::set_to_reset_state()
{
  _resource_owner_uuid.clear();
  _is_device_operational      = false;
  _current_provisioning_state = DEVICE_PROVISIONING_STATE_DEVICE_PAIRING_OWNER_TRANSFER;
  _target_provisioning_state  = 0;
  _state                      = DEVICE_OPERATION_STATE_OWNER_TRANSFER_METHOD;
}

void PstatObject::set_to_owner_transfer_method_state()
{
  _is_device_operational      = false;
  _current_provisioning_state = DEVICE_PROVISIONING_STATE_DEVICE_PAIRING_OWNER_TRANSFER;
  _target_provisioning_state  = 0;
  _state                      = DEVICE_OPERATION_STATE_OWNER_TRANSFER_METHOD;
}

void PstatObject::set_to_provisioning_state()
{
  _is_device_operational      = false;
  _current_provisioning_state = 0;
  _target_provisioning_state  = 0;
  _state                      = DEVICE_OPERATION_STATE_PROVISIONING_STATE;
}

void PstatObject::set_to_normal_operation_state()
{
  _is_device_operational      = true;
  _current_provisioning_state = 0;
  _target_provisioning_state  = 0;
  _state                      = DEVICE_OPERATION_STATE_NORMAL_OPERATION;
}

void PstatObject::set_to_soft_reset_state()
{
  _is_device_operational      = false;
  _current_provisioning_state = DEVICE_PROVISIONING_STATE_MANU_RESET;
  _target_provisioning_state  = 0;
  _state                      = DEVICE_OPERATION_STATE_SOFT_RESET;
}

inline bool PstatObject::is_valid_transition( uint8_t u8_new_state )
{
  switch( u8_new_state )
  {
    case DEVICE_OPERATION_STATE_RESET:
    {
      return ( true );
    }
    case DEVICE_OPERATION_STATE_OWNER_TRANSFER_METHOD:
    {
      return ( is_reset_state() );
    }
    case DEVICE_OPERATION_STATE_NORMAL_OPERATION:
    {
      return ( is_provisioning_state() );
    }
    case DEVICE_OPERATION_STATE_PROVISIONING_STATE:
    {
      return ( is_normal_operation_state() || is_owner_transfer_state() || is_soft_reset_state() );
    }
    case DEVICE_OPERATION_STATE_SOFT_RESET:
    {
      return ( is_normal_operation_state() || is_provisioning_state() );
    }
    default:
    {
    }
    break;
  }

  return ( false );
}

void PstatObject::set_to_default()
{
  _state                       = DEVICE_OPERATION_STATE_OWNER_TRANSFER_METHOD;
  _is_pending                  = false;
  _is_device_operational       = false;
  _current_provisioning_state  = DEVICE_PROVISIONING_STATE_DEVICE_PAIRING_OWNER_TRANSFER;
  _supported_provisioning_mode = DEVICE_PROVISIONING_MODE_CLIENT;
  _target_provisioning_state   = 0;
  _current_provisioning_mode   = DEVICE_PROVISIONING_MODE_CLIENT;
}
}
}
