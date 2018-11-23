/*
 * ProvisioningStatusRes.h
 *
 *  Created on: Nov 18, 2018
 *      Author: psammand
 */

#pragma once
#include "BaseResource.h"
#include "Uuid.h"
#include "DeviceOperationState.h"
#include "cbor/CborEncoder.h"

namespace ja_iot {
namespace resources {
using namespace stack;
using namespace base;

class PstatObject
{
  public:
    PstatObject () {}
    void    encode_to_cbor( CborEncoder &cz_cbor_encoder );
    uint8_t decode_from_cbor( ResRepresentation &res_rep );

    uint8_t get_state() { return ( _state ); }
    void    set_state( uint8_t new_state ) { _state = new_state; }
    bool    is_device_operational() { return ( _is_device_operational ); }
    Uuid    & get_resource_owner_id() { return ( _resource_owner_uuid ); }
    void    set_resource_owner_id( const Uuid &uuid ) { _resource_owner_uuid = uuid; }

    void    set_cur_provisioning_state( uint8_t new_state ) { _current_provisioning_state = new_state; }
    uint8_t get_cur_provisioning_state() { return ( _current_provisioning_state ); }

    void    set_target_provisioning_state( uint8_t new_state ) { _target_provisioning_state = new_state; }
    uint8_t get_target_provisioning_state() { return ( _target_provisioning_state ); }

    void    set_cur_provisioning_mode( uint8_t new_mode ) { _current_provisioning_mode = new_mode; }
    uint8_t get_cur_provisioning_mode() { return ( _current_provisioning_mode ); }

    void set_to_reset_state();
    void set_to_owner_transfer_method_state();
    void set_to_provisioning_state();
    void set_to_normal_operation_state();
    void set_to_soft_reset_state();
    bool is_reset_state() { return ( _state == DEVICE_OPERATION_STATE_RESET ); }
    bool is_owner_transfer_state() { return ( _state == DEVICE_OPERATION_STATE_OWNER_TRANSFER_METHOD ); }
    bool is_provisioning_state() { return ( _state == DEVICE_OPERATION_STATE_PROVISIONING_STATE ); }
    bool is_normal_operation_state() { return ( _state == DEVICE_OPERATION_STATE_NORMAL_OPERATION ); }
    bool is_soft_reset_state() { return ( _state == DEVICE_OPERATION_STATE_SOFT_RESET ); }

    bool is_pending() { return ( _is_pending ); }
    void set_pending( bool is_pending ) { _is_pending = is_pending; }
    bool is_same_state( uint8_t u8_new_state ) { return ( _state == u8_new_state ); }
    bool is_valid_transition( uint8_t u8_new_state );
    void set_to_default();

  private:
    uint8_t   _state                       = 0xFF;
    bool      _is_pending                  = false;
    bool      _is_device_operational       = false;           // read
    uint8_t   _current_provisioning_state  = 0xFF;                // read
    uint8_t   _target_provisioning_state   = 0xFF;               // read (RESET), RW (OTHER STATES)
    uint8_t   _current_provisioning_mode   = 0xFF;               // read (RESET), RW (OTHER STATES)
    uint8_t   _supported_provisioning_mode = 0xFF;                 // read
    Uuid      _resource_owner_uuid;
};

class ProvisioningStatusRes : public BaseResource
{
  public:
    ProvisioningStatusRes ();

    bool    is_method_supported( uint8_t method ) override;
    uint8_t get_state() { return ( _pstat_obj.get_state() ); }
    void    set_state( uint8_t new_state ) { _pstat_obj.set_state( new_state ); }

    bool is_reset_state() { return ( _pstat_obj.is_reset_state() ); }
    bool is_owner_transfer_state() { return ( _pstat_obj.is_owner_transfer_state() ); }
    bool is_provisioning_state() { return ( _pstat_obj.is_provisioning_state() ); }
    bool is_normal_operation_state() { return ( _pstat_obj.is_normal_operation_state() ); }
    bool is_soft_reset_state() { return ( _pstat_obj.is_soft_reset_state() ); }

  private:
    uint8_t handle_get( QueryContainer &query_container, Interaction *interaction ) override;
    uint8_t handle_post( QueryContainer &query_container, Interaction *interaction ) override;
    uint8_t handle_post_intl( QueryContainer &query_container, Interaction *interaction );
    void    init();

    uint8_t handle_state_transition( uint8_t u8_new_state );

    PstatObject   _pstat_obj;
};
}
}