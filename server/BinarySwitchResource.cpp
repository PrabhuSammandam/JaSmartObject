#include "BinarySwitchResource.h"
#include "StackConsts.h"
#include "CborCodec.h"
BinarySwitchResource::BinarySwitchResource () : BaseResource{ "/switch/1" }
{
  init();
}

uint8_t BinarySwitchResource::handle_request( Interaction *interaction )
{
  auto request = interaction->get_server_request();

  switch( request->get_code() )
  {
    case COAP_MSG_CODE_GET:
      return ( handle_get( interaction ) );
    case COAP_MSG_CODE_POST:
      return ( handle_post( interaction ) );
    case COAP_MSG_CODE_PUT:
    case COAP_MSG_CODE_DEL:
    default:
      return ( STACK_STATUS_INVALID_METHOD );
  }

  return ( STACK_STATUS_INVALID_METHOD );
}

uint8_t BinarySwitchResource::get_representation( ResInterfaceType interface_type, ResRepresentation &representation )
{
  return ( uint8_t() );
}

uint8_t BinarySwitchResource::get_discovery_representation( ResRepresentation &representation )
{
  BaseResource::get_discovery_representation( representation );

  return ( STACK_STATUS_OK );
}

uint8_t BinarySwitchResource::handle_get( Interaction *interaction )
{
  auto pcz_server_request  = interaction->get_server_request();
  auto interface_requested = ResInterfaceType::Actuator;

  if( pcz_server_request->get_option_set().get_uri_querys_count() > 0 )
  {
    QueryContainer query_container {};

    query_container.parse( pcz_server_request->get_option_set().get_uri_querys_list() );

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
  }

  ResRepresentation rep;

  if( interface_requested == ResInterfaceType::BaseLine )
  {
    rep.add( "if", get_interfaces() );
    rep.add( "rt", get_types() );
  }

  rep.add < bool > ( "value", _value );

  ResRepresentation representation {};
  representation.add( "", std::move( rep ) );

  uint8_t *         buffer;
  uint16_t          buffer_length;

  if( CborCodec::encode( representation, buffer, buffer_length ) == ErrCode::OK )
  {
    auto response = new ServerResponse {};
    response->set_code( COAP_MSG_CODE_CONTENT_205 );
    response->get_option_set().set_content_format( COAP_CONTENT_FORMAT_CBOR );
    response->set_payload( buffer, buffer_length );

    interaction->set_server_response( response );
  }

  return ( STACK_STATUS_OK );
}

uint8_t BinarySwitchResource::handle_post( Interaction *interaction )
{
  return ( uint8_t() );
}

void BinarySwitchResource::init()
{
  add_type( "oic.r.switch.binary" );
  add_interface( "oic.if.a" );
  add_interface( "oic.if.baseline" );
  set_property( OCF_RESOURCE_PROP_DISCOVERABLE );
}
