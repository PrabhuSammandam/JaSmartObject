#include "BaseResource.h"
#include "ObserverMgr.h"
#include "base_utils.h"
#include "StackConsts.h"
#include "CborCodec.h"
#include        "ErrCode.h"

namespace ja_iot {
namespace stack {
using namespace base;
BaseResource::BaseResource( std::string uri ) : _uri{ uri }
{
}

uint8_t BaseResource::handle_request( Interaction *interaction )
{
  auto request = interaction->get_server_request();

  if( !is_method_supported( request->get_code() ) )
  {
    return ( STACK_STATUS_INVALID_METHOD );
  }

  QueryContainer query_container{};

  if( request->get_option_set().get_uri_querys_count() > 0 )
  {
    query_container.parse( request->get_option_set().get_uri_querys_list() );
  }

  if( request->get_code() == COAP_MSG_CODE_GET )
  {
    return ( handle_get( query_container, interaction ) );
  }
  else if( request->get_code() == COAP_MSG_CODE_POST )
  {
    return ( handle_post( query_container, interaction ) );
  }
  else if( request->get_code() == COAP_MSG_CODE_PUT )
  {
    return ( handle_put( query_container, interaction ) );
  }
  else if( request->get_code() == COAP_MSG_CODE_DEL )
  {
    return ( handle_delete( query_container, interaction ) );
  }

  return ( STACK_STATUS_OK );

#if 0
  auto server_request = interaction->get_server_request();

  if( !server_request->get_option_set().has_observe() || !base::is_bit_set( get_property(), OCF_RESOURCE_PROP_OBSERVABLE ) )
  {
    return ( 0 );
  }

  auto &option_set     = server_request->get_option_set();
  auto observer_option = option_set.get_observe();

  if( observer_option == 0 )
  {
    /* register option */
    auto observe_info = ObserverMgr::inst().find_observer( this, server_request->get_token(), server_request->get_endpoint() );

    if( observe_info != nullptr )
    {
      /* already registered */
      return ( 0 );
    }

    observe_info = ObserverMgr::inst().register_observer( this, server_request->get_token(), server_request->get_endpoint() );

    // observe_info = new ObserveInfo{this, server_request->get_token(), server_request->get_endpoint()};
  }
  else if( observer_option == 1 )
  {
    /* unregister option */
    ObserverMgr::inst().unregister_observer( this, server_request->get_token(), server_request->get_endpoint() );
  }

#endif

  return ( 0 );
}

std::vector<std::string> & BaseResource::get_types()
{
  return ( _types );
}

void BaseResource::add_type( const std::string &types )
{
  _types.push_back( types );
}

std::vector<std::string> & BaseResource::get_interfaces()
{
  return ( _interfaces );
}

void BaseResource::add_interface( const std::string &interfaces )
{
  _interfaces.push_back( interfaces );
}

uint8_t BaseResource::get_property()
{
  return ( _property );
}

void BaseResource::set_property( const uint8_t property )
{
  _property = property;
}

bool BaseResource::is_collection()
{
  return ( false );
}

std::string & BaseResource::get_uri()
{
  return ( _uri );
}

void BaseResource::set_uri( const std::string &uri )
{
  _uri = uri;
}

std::string & BaseResource::get_unique_id()
{
  return ( _unique_id );
}

void BaseResource::set_unique_id( const std::string &unique_id )
{
  _unique_id = unique_id;
}

uint8_t BaseResource::get_representation( ResInterfaceType interface_type, ResRepresentation &representation )
{
  return ( 1 );
}

uint8_t BaseResource::get_discovery_representation( ResRepresentation &representation )
{
  representation.add( "href", get_uri() );
  representation.add( "rt", get_types() );
  representation.add( "if", get_interfaces() );

  ResRepresentation policy;
  policy.add( "bm", (long) get_property() );

  representation.add( "p", std::move( policy ) );

  return ( STACK_STATUS_OK );
}
void BaseResource::add_interfaces( std::vector<std::string> &interfaces )
{
  for( auto &if_name : interfaces )
  {
    _interfaces.push_back( if_name );
  }
}

uint8_t BaseResource::handle_get( QueryContainer &query_container, Interaction *interaction )
{
  return ( STACK_STATUS_INVALID_METHOD );
}

uint8_t BaseResource::handle_post( QueryContainer &query_container, Interaction *interaction )
{
  return ( STACK_STATUS_INVALID_METHOD );
}

uint8_t BaseResource::handle_put( QueryContainer &query_container, Interaction *interaction )
{
  return ( STACK_STATUS_INVALID_METHOD );
}

uint8_t BaseResource::handle_delete( QueryContainer &query_container, Interaction *interaction )
{
  return ( STACK_STATUS_INVALID_METHOD );
}

uint8_t BaseResource::set_response( Interaction *interaction, ResRepresentation &representation )
{
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
  else
  {
    return ( STACK_STATUS_OUT_OF_MEMORY );
  }

  return ( STACK_STATUS_OK );
}

uint8_t BaseResource::check_interface_query( QueryContainer &query_container )
{
  auto query_if_count = query_container.get_interface_count();

  if( query_if_count == 0 )
  {
    return ( STACK_STATUS_OK );
  }

  auto res_if_count = get_interfaces().size();

  if( query_if_count > res_if_count )
  {
	  /* query contains more no of interfaces than the resource supports */
    return ( STACK_STATUS_INVALID_INTERFACE_QUERY );
  }
  else
  {
    std::string if_string{ "if" };

    for( auto &query_if : query_container.get_query_map() )
    {
      if( query_if.first != if_string )
      {
        continue;
      }

      auto is_matched = false;

      for( auto &res_if_name : get_interfaces() )
      {
        if( res_if_name == query_if.second )
        {
          is_matched = true;
          break;
        }
      }

      if( !is_matched )
      {
        return ( STACK_STATUS_INVALID_INTERFACE_QUERY );
      }
    }
  }

  return ( STACK_STATUS_OK );
}

uint8_t BaseResource::check_type_query( QueryContainer &query_container )
{
  auto query_type_count = query_container.get_type_count();

  if( query_type_count == 0 )
  {
    return ( STACK_STATUS_OK );
  }

  /* get the number of resource type supported in this resource */
  auto res_type_count = get_types().size();

  if( query_type_count > res_type_count )
  {
	  /* query contains more number of resource types the resource supported */
    return ( STACK_STATUS_INVALID_TYPE_QUERY );
  }
  else
  {
    std::string rt_string{ "rt" };

    for( auto &query_rt : query_container.get_query_map() )
    {
    	/* check for only "rt" query */
      if( query_rt.first != rt_string )
      {
        continue;
      }

      /* found the "rt" query, now check whether it is supported in the resource */
      auto is_matched = false;

      for( auto &res_rt_name : get_types() )
      {
        if( res_rt_name == query_rt.second )
        {
          is_matched = true;
          break;
        }
      }

      if( !is_matched )
      {
        return ( STACK_STATUS_INVALID_TYPE_QUERY );
      }
    }
  }

  return ( STACK_STATUS_OK );
}
}
}
