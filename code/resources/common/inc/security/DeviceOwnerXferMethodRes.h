/*
 * DeviceOwnershipXferMethodRes.h
 *
 *  Created on: Nov 17, 2018
 *      Author: psammand
 */

#pragma once

#include <vector>
#include "BaseResource.h"
#include "Uuid.h"
#include "SecurityDataTypes.h"

namespace ja_iot::resources {
using namespace base;
using namespace stack;

class DeviceOwnerXferMethodRes : public BaseResource
{
  public:
    DeviceOwnerXferMethodRes ();

    bool    is_method_supported( uint8_t method ) override;
    uint8_t get_discovery_representation( ResRepresentation &representation ) override;
    uint8_t get_representation( ResInterfaceType interface_type, ResRepresentation &representation ) override;

  private:
    uint8_t handle_get( QueryContainer &query_container, Interaction *interaction ) override;
    uint8_t handle_post( QueryContainer &query_container, Interaction *interaction ) override;
    void    init();

    std::vector<uint8_t>   _owner_xfer_method_array;
    uint8_t                _selected_owner_xfer_method = (uint8_t)DeviceOwnerXferMethodType::RANDOM_PIN;
    uint8_t                _supported_credential_type  = CREDENTIAL_TYPE_SYMMETRIC_PAIR_WISE_KEY | CREDENTIAL_TYPE_ASYMMETRIC_SIGNING_KEY_WITH_CERT;
    bool                   _is_owned                   = false;
    Uuid _device_uuid{};
    Uuid _device_owner_uuid{};
    Uuid _resource_owner_uuid{};
};
}
