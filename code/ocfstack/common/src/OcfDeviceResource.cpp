#include "OcfDeviceResource.h"
#include "OcfDeviceInfo.h"

namespace ja_iot {
namespace ocfstack {
OcfDeviceResource::OcfDeviceResource () : OcfResource{ "/oic/d" }
{
  init();
}

uint8_t OcfDeviceResource::handle_request( OcfExchange &exchange )
{
  auto &request = exchange.get_request();

  if( ( request.get_code() == OcfMsgCode::PUT )
    || ( request.get_code() == OcfMsgCode::POST )
    || ( request.get_code() == OcfMsgCode::DEL ) )
  {
    return ( 1 );
  }

  auto       &query_container = request.get_query_container();

  /* if rt query passed and if it not matched then return */
  if ((query_container.get_type_count() > 0) && !query_container.is_res_type_available(get_types()[0]))
  {
    return (1);
  }

  if( !query_container.is_interface_matched(get_interfaces()))
  {
    return ( 1 );
  }

  auto &representation = exchange.get_response().get_representation();

  get_representation( OcfResInterfaceType::BaseLine, representation );

  exchange.respond();

  return ( 0 );
}

void OcfDeviceResource::set_device_info( OcfDeviceInfo &device_info )
{
  _name               = device_info.get_name();
  _device_id          = device_info.get_device_id();
  _server_version     = device_info.get_server_version();
  _data_model_version = device_info.get_data_model_version();
}

uint8_t OcfDeviceResource::get_representation( const OcfResInterfaceType interface_type, ResRepresentation &representation )
{
  if( ( interface_type != OcfResInterfaceType::BaseLine )
    || ( interface_type != OcfResInterfaceType::ReadOnly ) )
  {
    return ( 1 );
  }

  representation.add<std::vector<std::string> >( "rt", get_types() );
  representation.add<std::vector<std::string> >( "if", get_interfaces() );
  representation.add<std::string>( "n", _name );
  representation.add<std::string>( "di", _device_id );
  representation.add<std::string>( "icv", _server_version );
  representation.add<std::string>( "dmv", _data_model_version );

  return ( 0 );
}

uint8_t OcfDeviceResource::get_discovery_representation(ResRepresentation &representation )
{
  return 0;
}

void OcfDeviceResource::init()
{
  add_type( "oic.wk.d" );
  add_interface( "oic.if.baseline" );
  add_interface( "oic.if.r" );
  set_property( OCF_RESOURCE_PROP_DISCOVERABLE );
}
}
}