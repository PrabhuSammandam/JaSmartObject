#include <MsgStack.h>
#include <string.h>
#include "coap/coap_msg.h"
#include "common/inc/WellKnownResource.h"
#include "QueryContainer.h"
#include "ResourceMgr.h"
#include "StackConsts.h"
#include "OsalTimer.h"
#include "StackEvents.h"
#include "CborCodec.h"
#include "ErrCode.h"

using namespace ja_iot::osal;
using namespace ja_iot::network;
using namespace ja_iot::base;

namespace ja_iot {
namespace stack {
ErrCode get_linked_list_representation( QueryContainer &query_container, std::vector<ResRepresentation> &object_array );
WellKnownResource::WellKnownResource () : BaseResource{ "/oic/res" }
{
  init();
}

uint8_t WellKnownResource::handle_request( Interaction *interaction )
{
  ErrCode ret_status = ErrCode::OK;
  auto    request    = interaction->get_server_request();

  if( ( request->get_code() == COAP_MSG_CODE_PUT )
    || ( request->get_code() == COAP_MSG_CODE_POST )
    || ( request->get_code() == COAP_MSG_CODE_DEL ) )
  {
    return ( STACK_STATUS_INVALID_METHOD );
  }

  QueryContainer query_container{};

  query_container.parse( request->get_option_set().get_uri_querys_list() );

	if (check_interface_query(query_container) != STACK_STATUS_OK)
	{
		return (STACK_STATUS_INVALID_INTERFACE_QUERY);
	}

  auto requested_interface = ResInterfaceType::LinksList;

  if( query_container.get_interface_count() > 0 )
  {
		requested_interface = QueryContainer::get_interface_enum(query_container.get_first_if_name());
	}

  std::vector<ResRepresentation> objects{};

  if( get_linked_list_representation( query_container, objects ) != ErrCode::OK )
  {
    return ( STACK_STATUS_INVALID_METHOD );
  }

  ResRepresentation representation{};

  if( requested_interface == ResInterfaceType::BaseLine )
  {
		std::vector<ResRepresentation> baseline_rep_array{};

		ResRepresentation base_rep{};

		base_rep.add( "rt", get_types() );
		base_rep.add( "if", get_interfaces() );
		base_rep.add( "links", std::move(objects));

		baseline_rep_array.push_back(std::move(base_rep));
		representation.add("", std::move(baseline_rep_array));
  }
  else
  {
    representation.add( "", std::move(objects) );
  }

	representation.print();

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

ErrCode get_linked_list_representation( QueryContainer &query_container, std::vector<ResRepresentation> &object_array )
{
  auto &res_list = ResourceMgr::inst().get_resources_list();

  for( auto &res : res_list )
  {
	  /* check whether the resource is discoverable, if it is not then don't include in response */
    if( !static_cast<BaseResource *>( res )->is_discoverable() )
    {
      continue;
    }

#if 0
    bool if_found = false;

    for( auto &if_name : res->get_interfaces() )
    {
      if( if_name == QueryContainer::get_interface_string( ResInterfaceType::BaseLine ) )
      {
        if_found = true;
        break;
      }
    }

    if( !if_found )
    {
      continue;
    }
#endif
    /* check whether any specific type of resource is requested */
    if( query_container.get_type_count() > 0 )
    {
      auto type_found = false;

      for( auto &res_type : res->get_types() )
      {
        if( query_container.is_res_type_available( res_type ) )
        {
          type_found = true;
          break;
        }
      }

      if( type_found == false )
      {
        continue;
      }
    }

    ResRepresentation temp_representation{};

    res->get_discovery_representation( temp_representation );

    object_array.push_back( std::move( temp_representation ) );
  }

  return ( ErrCode::OK );
}

uint8_t WellKnownResource::get_representation( const ResInterfaceType interface_type, ResRepresentation &representation )
{
	return 0;
}

uint8_t WellKnownResource::get_discovery_representation( ResRepresentation &representation )
{
  return ( 0 );
}

void WellKnownResource::init()
{
  add_type( "oic.wk.res" );
  add_interface( "oic.if.ll" );
  add_interface( "oic.if.baseline" );
  set_property( OCF_RESOURCE_PROP_DISCOVERABLE );
}

}
}
