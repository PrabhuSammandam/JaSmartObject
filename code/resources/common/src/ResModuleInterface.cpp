/*
 * ResModuleInterface.cpp
 *
 *  Created on: Nov 23, 2018
 *      Author: psammand
 */
#include "ResModuleInterface.h"
#include "security/DeviceOwnerXferMethodRes.h"
#include "security/ProvisioningStatusRes.h"
#include "security/CredentialRes.h"
#include "security/AccessControlListRes.h"
#include "core/DeviceResource.h"
#include "core/WellKnownResource.h"
#include "ResourceMgr.h"

namespace ja_iot {
using namespace stack;
namespace resources {

ErrCode resource_module_init_core_resources()
{
	auto& res_mgr = ResourceMgr::inst();

	res_mgr.add_resource(new DeviceResource());
	res_mgr.add_resource(new WellKnownResource());

	return ErrCode::OK;
}

ErrCode resource_module_init_secure_virtual_resources()
{
	auto& res_mgr = ResourceMgr::inst();

	res_mgr.add_resource(new DeviceOwnerXferMethodRes());
	res_mgr.add_resource(new ProvisioningStatusRes());
	res_mgr.add_resource(new CredentialRes());
	res_mgr.add_resource(new AccessControlListRes());

	return ErrCode::OK;
}

}
}
