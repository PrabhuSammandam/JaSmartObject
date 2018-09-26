#include "BinarySwitchResource.h"
#include "StackConsts.h"
#include "cbor/CborCodec.h"

using namespace ja_iot::stack;
using namespace ja_iot::base;
BinarySwitchResource::BinarySwitchResource () : BaseResource{ "/switch/1" }
{
  init();
}

BinarySwitchResource::BinarySwitchResource(std::string uri) : BaseResource{ uri }
{
	init();
}

bool BinarySwitchResource::is_method_supported( uint8_t method )
{
  return ( method == COAP_MSG_CODE_GET || method == COAP_MSG_CODE_POST );
}

uint8_t BinarySwitchResource::get_representation( ResInterfaceType interface_type, ResRepresentation &representation )
{
  if( interface_type == ResInterfaceType::BaseLine )
  {
    representation.add( "if", get_interfaces() );
    representation.add( "rt", get_types() );
  }

  representation.add<bool>( "value", _value );

  return ( STACK_STATUS_OK );
}

uint8_t BinarySwitchResource::handle_get( QueryContainer &query_container, Interaction *interaction )
{
  auto request             = interaction->get_server_request();
  auto interface_requested = ResInterfaceType::Actuator;

  if( check_type_query( query_container ) != STACK_STATUS_OK )
  {
    return ( STACK_STATUS_INVALID_TYPE_QUERY );
  }

  if( check_interface_query( query_container ) != STACK_STATUS_OK )
  {
    return ( STACK_STATUS_INVALID_INTERFACE_QUERY );
  }

  if( query_container.get_interface_count() > 0 )
  {
    interface_requested = QueryContainer::get_interface_enum( query_container.get_first_if_name() );
  }

  ResRepresentation rep;
  get_representation( interface_requested, rep );

  ResRepresentation representation{};
  representation.add( "", std::move( rep ) );

  return ( set_response( interaction, representation ) );
}

uint8_t BinarySwitchResource::handle_post( QueryContainer &query_container, Interaction *interaction )
{
	auto request = interaction->get_server_request();

	if (!request->has_payload())
	{
		return STACK_STATUS_INVALID_METHOD;
	}

	auto interface_requested = ResInterfaceType::Actuator;

	if (check_type_query(query_container) != STACK_STATUS_OK)
	{
		return (STACK_STATUS_INVALID_TYPE_QUERY);
	}

	if (check_interface_query(query_container) != STACK_STATUS_OK)
	{
		return (STACK_STATUS_INVALID_INTERFACE_QUERY);
	}

	if (query_container.get_interface_count() > 0)
	{
		interface_requested = QueryContainer::get_interface_enum(query_container.get_first_if_name());
	}

	ResRepresentation rep;

	if (CborCodec::decode(request->get_payload(), request->get_payload_len(), rep) != ErrCode::OK)
	{
		return STACK_STATUS_INVALID_METHOD;
	}

	if (rep.has_prop("value"))
	{
		_value = rep.get_prop<bool>("value");
	}

	get_representation(interface_requested, rep);

	ResRepresentation representation{};
	representation.add("", std::move(rep));

	return (set_response(interaction, representation));
}

void BinarySwitchResource::init()
{
  add_type( "oic.r.switch.binary" );
  add_interface( "oic.if.a" );
  add_interface( "oic.if.baseline" );
  set_property( OCF_RESOURCE_PROP_DISCOVERABLE );
}