/*
 * BinarySwitchResourceBase.cpp
 *
 *  Created on: Nov 8, 2018
 *      Author: psammand
 */
#include <string>
#include "ErrCode.h"
#include "StackConsts.h"
#include "BinarySwitchResourceBase.h"
#include "CborCodec.h"

/* Consolidated properties
 * oic.core.json
 * 1. rt[] : string
 * 2. if[] : string
 * 3. n : string
 * 4. id : string
 *
 * oic.r.switch.binary
 * 5. value : bool
 * */

namespace ja_iot::resources {
using namespace base;
BinarySwitchResourceBase::BinarySwitchResourceBase( std::string uri ) : BaseResource{ uri }
{
  init();
}

bool BinarySwitchResourceBase::is_method_supported( uint8_t method )
{
  return ( ( method == COAP_MSG_CODE_GET ) || ( method == COAP_MSG_CODE_POST ) );
}

uint8_t BinarySwitchResourceBase::get_representation( ResInterfaceType interface_type, ResRepresentation &representation )
{
  if( interface_type == ResInterfaceType::BaseLine )
  {
    representation.add<std::vector<std::string> >( "if", get_interfaces() );
    representation.add<std::vector<std::string> >( "rt", get_types() );
  }

  representation.add<bool>( "value", _value );

  return ( STACK_STATUS_OK );
}

uint8_t BinarySwitchResourceBase::handle_get( QueryContainer &query_container, Interaction *interaction )
{
  if( !query_container.check_valid_query( get_property_name_list() ) )
  {
    return ( STACK_STATUS_INVALID_TYPE_QUERY );
  }

  auto interface_requested = ResInterfaceType::Actuator;

  if( query_container.get_interface_count() > 0 )
  {
    interface_requested = QueryContainer::get_interface_enum( query_container.get_first_if_name() );
  }

  ResRepresentation rep;
  get_representation( interface_requested, rep );

  ResRepresentation representation{};
  representation.add( "", std::move( rep ) );

  uint8_t *buffer;
  uint16_t buffer_length;

  if( CborCodec::encode( representation, buffer, buffer_length ) == ErrCode::OK )
  {
    auto response = new ServerResponse{};
    response->set_code( COAP_MSG_CODE_CONTENT_205 );
    response->get_option_set().set_content_format( COAP_CONTENT_FORMAT_CBOR );
    response->set_payload( buffer, buffer_length );

    interaction->set_server_response( response );
  }

  return ( STACK_STATUS_OK );
}

uint8_t BinarySwitchResourceBase::handle_post( QueryContainer &query_container, Interaction *interaction )
{
  auto request = interaction->get_server_request();

  if( !request->has_payload() )
  {
    return ( STACK_STATUS_INVALID_METHOD );
  }

  if( !query_container.check_valid_query( get_property_name_list() ) )
  {
    return ( STACK_STATUS_INVALID_TYPE_QUERY );
  }

  auto interface_requested = ResInterfaceType::Actuator;

  if( query_container.get_interface_count() > 0 )
  {
    interface_requested = QueryContainer::get_interface_enum( query_container.get_first_if_name() );
  }

  ResRepresentation rep;

  if( CborCodec::decode( request->get_payload(), request->get_payload_len(), rep ) != ErrCode::OK )
  {
    return ( STACK_STATUS_INVALID_METHOD );
  }

  if( rep.has_prop( "value" ) )
  {
    auto old_value = _value;
    _value = rep.get_prop<bool>( "value" );

    if( old_value != _value )
    {
      handle_property_change();
    }
  }

  rep.~ResRepresentation ();
  get_representation( interface_requested, rep );

  ResRepresentation representation{};
  representation.add( "", std::move( rep ) );

  return ( set_response( interaction, representation ) );
}

uint8_t BinarySwitchResourceBase::get_discovery_representation( ResRepresentation &representation )
{
  BaseResource::get_discovery_representation( representation );

  return ( STACK_STATUS_OK );
}
void BinarySwitchResourceBase::init()
{
  add_type( "oic.r.switch.binary" );
  add_interface( "oic.if.a" );
  add_interface( "oic.if.baseline" );
  set_property( OCF_RESOURCE_PROP_DISCOVERABLE | OCF_RESOURCE_PROP_OBSERVABLE );

  add_property_name( "value" );
}
}
