#include "OcfWellKnownResource.h"
#include "OcfResourceMgr.h"

namespace ja_iot {
namespace ocfstack {
OcfWellKnownResource::OcfWellKnownResource () : OcfResource{ "/oic/res" }
{
  init();
}

uint8_t OcfWellKnownResource::handle_request( OcfExchange &exchange )
{
  auto &request = exchange.get_request();

  if( ( request.get_code() == OcfMsgCode::PUT )
    || ( request.get_code() == OcfMsgCode::POST )
    || ( request.get_code() == OcfMsgCode::DEL ) )
  {
    return ( 1 );
  }

  auto &query_container = request.get_query_container();

  /* if rt query passed and if it not matched then return */
  if( ( query_container.get_type_count() > 0 ) && !query_container.is_res_type_available( get_types()[0] ) )
  {
    return ( 1 );
  }

  if( !query_container.is_interface_matched( get_interfaces() ) )
  {
    return ( 1 );
  }

  auto &representation = exchange.get_response().get_representation();

  get_representation( query_container.is_interface_available( OcfResInterfaceType::LinksList ) ? OcfResInterfaceType::LinksList : OcfResInterfaceType::BaseLine, representation );

  return ( 0 );
}

uint8_t OcfWellKnownResource::get_representation( const OcfResInterfaceType interface_type, ResRepresentation &representation )
{
  if( interface_type == OcfResInterfaceType::LinksList )
  {
    return ( get_links_list_representation( representation ) );
  }
  else
  {
    return ( get_baseline_representation( representation ) );
  }
}

uint8_t OcfWellKnownResource::get_discovery_representation( ResRepresentation &representation )
{
  return ( 0 );
}

void OcfWellKnownResource::init()
{
  add_type( "oic.wk.res" );
  add_interface( "oic.if.baseline" );
  add_interface( "oic.if.ll" );
  set_property( OCF_RESOURCE_PROP_DISCOVERABLE );
}

uint8_t OcfWellKnownResource::get_baseline_representation( ResRepresentation &representation )
{
  representation.add<std::vector<std::string> >( "rt", get_types() );
  representation.add<std::vector<std::string> >( "if", get_interfaces() );

  auto &res_list = ResourceManager::inst().get_resources_list();

  for (auto& res : res_list)
  {
    res->get_discovery_representation(representation);
  }

  return ( 0 );
}

uint8_t OcfWellKnownResource::get_links_list_representation( ResRepresentation &representation ) const
{
  auto &res_list = ResourceManager::inst().get_resources_list();

  for (auto& res : res_list)
  {
    res->get_discovery_representation(representation);
  }

  return ( 0 );
}
}
}