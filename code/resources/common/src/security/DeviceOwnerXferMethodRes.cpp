/*
 * DeviceOwnershipXferMethodRes.cpp
 *
 *  Created on: Nov 17, 2018
 *      Author: psammand
 */

#include <common/inc/security/DeviceOwnerXferMethodRes.h>
#include "common/inc/security/SecurityDataTypes.h"
#include "StackConsts.h"
#include "CborCodec.h"

namespace ja_iot::resources {
DeviceOwnerXferMethodRes::DeviceOwnerXferMethodRes () : BaseResource( "/oic/sec/doxm" )
{
}

bool DeviceOwnerXferMethodRes::is_method_supported( uint8_t method )
{
  return ( method == COAP_MSG_CODE_GET || method == COAP_MSG_CODE_POST );
}

uint8_t DeviceOwnerXferMethodRes::handle_get( QueryContainer &query_container, Interaction *interaction )
{
  ResRepresentation rep{};
  auto stack_status = get_representation( ResInterfaceType::BaseLine, rep );

  if( stack_status == STACK_STATUS_OK )
  {
    ResRepresentation representation{};
    representation.add( "", std::move( rep ) );
    stack_status = set_response( interaction, representation );
  }

  return ( stack_status );
}
uint8_t DeviceOwnerXferMethodRes::handle_post( QueryContainer &query_container, Interaction *interaction )
{
  auto request = interaction->get_server_request();

  if( !request->has_payload() )
  {
    return ( STACK_STATUS_INVALID_METHOD );
  }

  ResRepresentation rep;

  if( CborCodec::decode( request->get_payload(), request->get_payload_len(), rep ) != ErrCode::OK )
  {
    return ( STACK_STATUS_INVALID_METHOD );
  }

  uint8_t selected_owner_xfer_method = 0xFF;
  uint8_t selected_credential_type = 0xFF;

  if( rep.has_prop( "oxmsel" ) )
  {
	  selected_owner_xfer_method = rep.get_prop<long>("oxmsel");
  }

  if( rep.has_prop( "sct" ) )
  {
	  selected_credential_type = rep.get_prop<long>("sct");
  }

  return ( STACK_STATUS_OK );
}
void DeviceOwnerXferMethodRes::init()
{
  add_type( "oic.r.doxm" );
  add_interface( "oic.if.baseline" );
  set_property( OCF_RESOURCE_PROP_DISCOVERABLE );
}

uint8_t DeviceOwnerXferMethodRes::get_representation( ResInterfaceType interface_type, ResRepresentation &representation )
{
  if( !_owner_xfer_method_array.empty() )
  {
    std::vector<long> la{};

    for( auto i = 0; i < (int) _owner_xfer_method_array.size(); i++ )
    {
      la[i] = _owner_xfer_method_array[i];
    }

    representation.add( "oxms", la );
  }

  representation.add( "oxmsel", (long) _selected_owner_xfer_method );
  representation.add( "sct", (long) _supported_credential_type );
  representation.add( "owned", _is_owned );
  representation.add( "deviceuuid", std::move( _device_uuid.to_string() ) );
  representation.add( "devowneruuid", std::move( _device_owner_uuid.to_string() ) );
  representation.add( "rowneruuid", std::move( _resource_owner_uuid.to_string() ) );
  representation.add( "rt", get_types() );
  representation.add( "if", get_interfaces() );

  return ( STACK_STATUS_OK );
}
}
