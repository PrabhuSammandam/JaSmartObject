#include "common/inc/DeviceResource.h"
#include "QueryContainer.h"
#include "DeviceInfo.h"
#include "StackConsts.h"
#include"ResourceMgr.h"

namespace ja_iot {
namespace stack {
using namespace base;
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

bool DeviceResource::is_method_supported(uint8_t method)
{
	return (method == COAP_MSG_CODE_GET );
}

uint8_t DeviceResource::handle_get(QueryContainer & query_container, Interaction * interaction)
{
	auto interface_requested = ResInterfaceType::ReadOnly;

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

	ResRepresentation rep{};
	auto stack_status = get_representation(interface_requested, rep);

	if (stack_status == STACK_STATUS_OK)
	{
		ResRepresentation representation{};
		representation.add("", std::move(rep));
		stack_status = set_response(interaction, representation);
	}

	return (stack_status);
}

uint8_t DeviceResource::get_representation( const ResInterfaceType interface_type, ResRepresentation &representation )
{
	if(interface_type == ResInterfaceType::BaseLine || interface_type == ResInterfaceType::ReadOnly)
	{
		auto& device_info = ResourceMgr::inst().get_device_info();
	  /* all are readonly properties */
	  representation.add( "rt", get_types() );
	  representation.add( "if", get_interfaces() );
	  representation.add( "n", device_info.get_name());
	  representation.add( "di", device_info.get_device_id());
	  representation.add( "icv", device_info.get_server_version());
	  representation.add( "dmv", device_info.get_data_model_version());

	  return ( STACK_STATUS_OK );
	}
	return STACK_STATUS_INVALID_INTERFACE_QUERY;
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
