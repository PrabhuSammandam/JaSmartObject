/*
 * DeviceOperationState.h
 *
 *  Created on: Nov 18, 2018
 *      Author: psammand
 */

#pragma once

#include <cstdint>
#include "SecurityDataTypes.h"

namespace ja_iot {
namespace resources {
class DeviceOperationState
{
  public:
    bool is_reset() { return ( _state == DEVICE_OPERATION_STATE_RESET ); }
    bool is_transfer() { return ( _state == DEVICE_OPERATION_STATE_OWNER_TRANSFER_METHOD ); }
    bool is_normal_op() { return ( _state == DEVICE_OPERATION_STATE_NORMAL_OPERATION ); }
    bool is_provisioning() { return ( _state == DEVICE_OPERATION_STATE_PROVISIONING_STATE ); }
    bool is_soft_reset() { return ( _state == DEVICE_OPERATION_STATE_SOFT_RESET ); }
    bool is_pending() { return ( _pending ); }
    void set_pending( bool is_pending ) { _pending = is_pending; }

    bool is_same_state( uint8_t u8_new_state ) { return ( _state == u8_new_state ); }
    bool is_valid_transition( uint8_t u8_new_state )
    {
      switch( u8_new_state )
      {
        case DEVICE_OPERATION_STATE_RESET:
        {
          return ( true );
        }
        case DEVICE_OPERATION_STATE_OWNER_TRANSFER_METHOD:
        {
          return ( is_reset() );
        }
        case DEVICE_OPERATION_STATE_NORMAL_OPERATION:
        {
          return ( is_provisioning() );
        }
        case DEVICE_OPERATION_STATE_PROVISIONING_STATE:
        {
          return ( is_normal_op() || is_transfer() || is_soft_reset() );
        }
        case DEVICE_OPERATION_STATE_SOFT_RESET:
        {
          return ( is_normal_op() || is_provisioning() );
        }
        default:
        {
        }
        break;
      }

      return ( false );
    }

  public:
    uint8_t   _state = 0xFF;
    bool      _pending;
};
}
}