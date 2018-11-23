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
#include "cbor/CborEncoder.h"

namespace ja_iot {
namespace resources {
using namespace base;
using namespace stack;

class DoxmObject
{
  public:
    void    encode_to_cbor( CborEncoder &cz_cbor_encoder );
    uint8_t decode_from_cbor( ResRepresentation &cz_res_rep, uint8_t u8_device_state );
    void    set_to_default();
    void    copy_updatable_props( DoxmObject &src );

    void set_owned( bool is_owned ) { _is_owned = is_owned; }
    bool is_owned() { return ( _is_owned ); }

    Uuid & get_device_uuid() { return ( _device_uuid ); }
    void set_device_uuid( const Uuid &uuid ) { _device_uuid = uuid; }

    Uuid & get_device_owner_uuid() { return ( _device_owner_uuid ); }
    void set_device_owner_uuid( const Uuid &uuid ) { _device_owner_uuid = uuid; }

    Uuid & get_resource_owner_uuid() { return ( _resource_owner_uuid ); }
    void set_resource_owner_uuid( const Uuid &uuid ) { _resource_owner_uuid = uuid; }

  private:
    std::vector<uint8_t>   _owner_xfer_method_array;
    uint8_t                _selected_owner_xfer_method = 0xFF;
    uint8_t                _supported_credential_type  = 0xFF;
    bool                   _is_owned                   = false;
    Uuid _device_uuid{};
    Uuid _device_owner_uuid{};
    Uuid _resource_owner_uuid{};
};

class DeviceOwnerXferMethodRes : public BaseResource
{
  public:
    DeviceOwnerXferMethodRes ();

    bool is_method_supported( uint8_t method ) override;

    void set_owned( bool is_owned ) { _doxm_obj.set_owned( is_owned ); }
    bool is_owned() { return ( _doxm_obj.is_owned() ); }

    Uuid & get_device_uuid() { return ( _doxm_obj.get_device_uuid() ); }
    void set_device_uuid( const Uuid &uuid ) { _doxm_obj.set_device_uuid( uuid ); }

    Uuid & get_device_owner_uuid() { return ( _doxm_obj.get_device_owner_uuid() ); }
    void set_device_owner_uuid( const Uuid &uuid ) { _doxm_obj.set_device_owner_uuid( uuid ); }

    Uuid & get_resource_owner_uuid() { return ( _doxm_obj.get_resource_owner_uuid() ); }
    void set_resource_owner_uuid( const Uuid &uuid ) { _doxm_obj.set_resource_owner_uuid( uuid ); }

  private:
    uint8_t handle_get( QueryContainer &query_container, Interaction *interaction ) override;
    uint8_t handle_post( QueryContainer &query_container, Interaction *interaction ) override;
    uint8_t handle_post_intl( QueryContainer &query_container, Interaction *interaction );
    void    init();

    DoxmObject   _doxm_obj;
};
}
}