#include "core/DeviceResource.h"
#include "QueryContainer.h"
#include "DeviceInfo.h"
#include "StackConsts.h"
#include "ResourceMgr.h"
#include "cbor/CborEncoder.h"

namespace ja_iot {
namespace resources {
using namespace base;
using namespace stack;
/*
 * WellKnownURI		/oic/d
 * Resource Type	oic.wk.d
 * Methods			GET[oic.if.r, oic.if.baseline]
 * Interfaces		oic.if.r, oic.if.baseline
 */
DeviceResource::DeviceResource () : BaseResource{ "/oic/d" }
{
  init();
}

bool DeviceResource::is_method_supported( uint8_t method )
{
  return ( method == COAP_MSG_CODE_GET );
}

uint8_t DeviceResource::handle_get( QueryContainer &query_container, Interaction *interaction )
{
  if( check_type_query( query_container ) != STACK_STATUS_OK )
  {
    return ( STACK_STATUS_INVALID_TYPE_QUERY );
  }

  if( check_interface_query( query_container ) != STACK_STATUS_OK )
  {
    return ( STACK_STATUS_INVALID_INTERFACE_QUERY );
  }

  // auto interface_requested = query_container.get_first_interface( ResInterfaceType::ReadOnly );
  auto &device_info = ResourceMgr::inst().get_device_info();

  CborEncoder cz_encoder{ 1024 };

  cz_encoder.start_map();
  cz_encoder.write_map_entry( "rt", get_types() );
  cz_encoder.write_map_entry( "if", get_interfaces() );
  cz_encoder.write_map_entry( "n", device_info.get_name() );
  cz_encoder.write_map_entry( "di", device_info.get_device_id() );
  cz_encoder.write_map_entry( "icv", device_info.get_server_version() );
  cz_encoder.write_map_entry( "dmv", device_info.get_data_model_version() );
  cz_encoder.end_map();

  return ( set_cbor_response( interaction, cz_encoder.get_buf(), cz_encoder.get_buf_len() ) );
}

void DeviceResource::init()
{
  add_type( "oic.wk.d" );
  add_interface( "oic.if.r" );
  add_interface( "oic.if.baseline" );
  set_property( OCF_RESOURCE_PROP_DISCOVERABLE );
}
}
}