/*
 * DeviceOwnershipXferMethodRes.cpp
 *
 *  Created on: Nov 17, 2018
 *      Author: psammand
 */
#include <algorithm>
#include <security/DeviceOwnerXferMethodRes.h>
#include "security/SecurityDataTypes.h"
#include "security/ProvisioningStatusRes.h"
#include "StackConsts.h"
#include "CborCodec.h"
#include "cbor/CborDecoder.h"
#include "cbor/CborEncoder.h"
#include "base_utils.h"
#include "ResourceMgr.h"

namespace ja_iot {
namespace resources {
using namespace base;

constexpr const char *DOXM_PROP_RT           = "rt";
constexpr const char *DOXM_PROP_IF           = "if";
constexpr const char *DOXM_PROP_OXMS         = "oxms";
constexpr const char *DOXM_PROP_OXMSEL       = "oxmsel";
constexpr const char *DOXM_PROP_SCT          = "sct";
constexpr const char *DOXM_PROP_OWNED        = "owned";
constexpr const char *DOXM_PROP_DEVICEUUID   = "deviceuuid";
constexpr const char *DOXM_PROP_DEVOWNERUUID = "devowneruuid";
constexpr const char *DOXM_PROP_ROWNERUUID   = "rowneruuid";
DeviceOwnerXferMethodRes::DeviceOwnerXferMethodRes () : BaseResource( "/oic/sec/doxm" )
{
  init();
}

bool DeviceOwnerXferMethodRes::is_method_supported( uint8_t method )
{
  return ( method == COAP_MSG_CODE_GET || method == COAP_MSG_CODE_POST );
}

void to_lower(std::string& str)
{
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}

uint8_t DeviceOwnerXferMethodRes::handle_get( QueryContainer &query_container, Interaction *interaction )
{
	for(auto& q : query_container.get_all_of_map())
	{
		auto key = q.first;
		to_lower(key);

		if(key == "owned")
		{
			to_lower(q.second);

			if ((q.second == "true" && !_doxm_obj.is_owned())
				|| (q.second == "false" && _doxm_obj.is_owned()))
			{
				return STACK_STATUS_OK;
			}
		}
	}

	CborEncoder cz_encoder{ 256 };

  cz_encoder.start_map();
  _doxm_obj.encode_to_cbor( cz_encoder );
  cz_encoder.write_map_entry( DOXM_PROP_RT, get_types() );
  cz_encoder.write_map_entry( DOXM_PROP_IF, get_interfaces() );
  cz_encoder.end_map();

  return ( BaseResource::set_cbor_response( interaction, cz_encoder.get_buf(), cz_encoder.get_buf_len() ) );
}

uint8_t DeviceOwnerXferMethodRes::handle_post( QueryContainer &query_container, Interaction *interaction )
{
  auto ret_status = handle_post_intl( query_container, interaction );

  auto response = new ServerResponse{};

  response->set_code( ret_status );
  interaction->set_server_response( response );

  return ( STACK_STATUS_OK );
}

uint8_t DeviceOwnerXferMethodRes::handle_post_intl( QueryContainer &query_container, Interaction *interaction )
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

  auto provisioning_status_res = (ProvisioningStatusRes *) ResourceMgr::inst().get_provisioning_status_res();

  if( provisioning_status_res == nullptr )
  {
    return ( COAP_MSG_CODE_INTERNAL_SERVER_ERROR_500 );
  }

  DoxmObject doxm_obj{};
  doxm_obj.copy_updatable_props( _doxm_obj );
  uint8_t ret_status = doxm_obj.decode_from_cbor( rep, provisioning_status_res->get_state() );

  if( ret_status != COAP_MSG_CODE_CHANGED_204 )
  {
    return ( ret_status );
  }

  _doxm_obj.copy_updatable_props( doxm_obj );

  return ( COAP_MSG_CODE_CHANGED_204 );
}

void DeviceOwnerXferMethodRes::init()
{
  _doxm_obj.set_to_default();

  add_type( "oic.r.doxm" );
  add_interface( "oic.if.baseline" );
  set_property( OCF_RESOURCE_PROP_DISCOVERABLE );
}

/*****************************************************************************************************************/
/**********************************   DoxmObject             *****************************************************/
/*****************************************************************************************************************/

void DoxmObject::encode_to_cbor( CborEncoder &cz_encoder)
{
  cz_encoder.write_map_entry( DOXM_PROP_OXMS, _owner_xfer_method_array );
  cz_encoder.write_map_entry( DOXM_PROP_OXMSEL, _selected_owner_xfer_method );
  cz_encoder.write_map_entry( DOXM_PROP_SCT, _supported_credential_type );
  cz_encoder.write_map_entry( DOXM_PROP_OWNED, _is_owned );
  cz_encoder.write_map_entry( DOXM_PROP_DEVICEUUID, _device_uuid.to_string() );
  cz_encoder.write_map_entry( DOXM_PROP_DEVOWNERUUID, _device_owner_uuid.to_string() );
  cz_encoder.write_map_entry( DOXM_PROP_ROWNERUUID, _resource_owner_uuid.to_string() );
}

#define CHECK_NULL( _PTR_, _RET_VALUE_ ) if( _PTR_ == nullptr ){ return _RET_VALUE_; }
uint8_t DoxmObject::decode_from_cbor( ResRepresentation &cz_res_rep, uint8_t u8_device_state )
{
  for( auto &prop_key : cz_res_rep.get_props() )
  {
    CHECK_NULL( prop_key.second, COAP_MSG_CODE_NOT_ACCEPTABLE_406 );

    if( ( prop_key.first == DOXM_PROP_OXMS )
      || ( prop_key.first == DOXM_PROP_SCT ) )
    {
      return ( COAP_MSG_CODE_NOT_ACCEPTABLE_406 );
    }
    else if( prop_key.first == DOXM_PROP_OXMSEL )
    {
      if( ( u8_device_state != DEVICE_OPERATION_STATE_OWNER_TRANSFER_METHOD )
        || !find_in_list( _owner_xfer_method_array, (uint8_t) prop_key.second->get<long>() ) )
      {
        return ( COAP_MSG_CODE_NOT_ACCEPTABLE_406 );
      }

      _selected_owner_xfer_method = (uint8_t)prop_key.second->get<long>();
    }
    else if( prop_key.first == DOXM_PROP_OWNED )
    {
      if( u8_device_state != DEVICE_OPERATION_STATE_OWNER_TRANSFER_METHOD )
      {
        return ( COAP_MSG_CODE_NOT_ACCEPTABLE_406 );
      }

      _is_owned = prop_key.second->get<bool>();
    }
    else if( prop_key.first == DOXM_PROP_DEVICEUUID )
    {
      if( u8_device_state != DEVICE_OPERATION_STATE_OWNER_TRANSFER_METHOD )
      {
        return ( COAP_MSG_CODE_NOT_ACCEPTABLE_406 );
      }

      _device_uuid << prop_key.second->get<std::string>();
    }
    else if( prop_key.first == DOXM_PROP_DEVOWNERUUID )
    {
      if( u8_device_state != DEVICE_OPERATION_STATE_OWNER_TRANSFER_METHOD )
      {
        return ( COAP_MSG_CODE_NOT_ACCEPTABLE_406 );
      }

      _device_owner_uuid << prop_key.second->get<std::string>();
    }
    else if( prop_key.first == DOXM_PROP_ROWNERUUID )
    {
      if( ( u8_device_state != DEVICE_OPERATION_STATE_OWNER_TRANSFER_METHOD )
        || ( u8_device_state != DEVICE_OPERATION_STATE_SOFT_RESET ) )
      {
        return ( COAP_MSG_CODE_NOT_ACCEPTABLE_406 );
      }

      _resource_owner_uuid << prop_key.second->get<std::string>();
    }
  }

  return ( COAP_MSG_CODE_CHANGED_204 );
}

void DoxmObject::set_to_default()
{
  _owner_xfer_method_array.clear();
	_owner_xfer_method_array.push_back((uint8_t)DeviceOwnerXferMethodType::RANDOM_PIN);
  _selected_owner_xfer_method = (uint8_t) DeviceOwnerXferMethodType::RANDOM_PIN;
  _supported_credential_type  = CREDENTIAL_TYPE_SYMMETRIC_PAIR_WISE_KEY | CREDENTIAL_TYPE_ASYMMETRIC_SIGNING_KEY_WITH_CERT;
  _is_owned                   = false;
  _device_uuid.clear();
  _device_owner_uuid.clear();
  _resource_owner_uuid.clear();
}

void DoxmObject::copy_updatable_props( DoxmObject &src )
{
  _selected_owner_xfer_method = src._selected_owner_xfer_method;
  _is_owned                   = src._is_owned;
  _device_uuid                = src._device_uuid;
  _device_owner_uuid          = src._device_owner_uuid;
  _resource_owner_uuid        = src._resource_owner_uuid;
}
}
}
