/*
 * AdapterEvent.h
 *
 *  Created on: Nov 26, 2018
 *      Author: psammand
 */

#pragma once
#include <base_datatypes.h>
#include <cstdint>
#include <ErrCode.h>
#include "end_point.h"

namespace ja_iot {
using namespace base;
namespace network {
constexpr uint8_t ADAPTER_EVENT_TYPE_NONE               = 0;
constexpr uint8_t ADAPTER_EVENT_TYPE_PACKET_RECEIVED    = 1;
constexpr uint8_t ADAPTER_EVENT_TYPE_ERROR              = 2;
constexpr uint8_t ADAPTER_EVENT_TYPE_ADAPTER_CHANGED    = 3;
constexpr uint8_t ADAPTER_EVENT_TYPE_CONNECTION_CHANGED = 4;

class AdapterEvent
{
  public:
    AdapterEvent( const uint8_t adapter_event_type ) : _event_type{ adapter_event_type }, _is_enabled{ false }, _is_connected{ false }, _reserved{ 0 }
    {
    }
    AdapterEvent( const uint8_t event_type, Endpoint *endpoint, uint8_t *data, const uint16_t data_len, const uint16_t adapter_type )
      : _event_type{ event_type },
      _is_enabled{ false },
      _is_connected{ false },
      _reserved{ 0 },
      end_point_{ endpoint },
	  _data{data, data_len},
      adapter_type_{ adapter_type }
    {
    }

    uint16_t get_adapter_type() const { return ( adapter_type_ ); }
    void     set_adapter_type( const uint16_t adapter_type ) { adapter_type_ = adapter_type; }

    const Endpoint* get_end_point() const { return ( end_point_ ); }
    void            set_end_point( Endpoint *end_point ) { end_point_ = end_point; }

    base::ErrCode get_error_code() const { return ( error_code_ ); }
    void          set_error_code( const base::ErrCode error_code ) { error_code_ = error_code; }

    bool is_connected() const { return ( _is_connected ); }
    void set_connected( const bool is_connected ) { _is_connected = is_connected; }

    bool is_enabled() const { return ( _is_enabled ); }
    void set_enabled( const bool is_enabled ) { _is_enabled = is_enabled; }

    uint8_t get_adapter_event_type() const { return ( _event_type ); }
    void    set_adapter_event_type( const uint8_t adapter_event_type )
    {
      _event_type = adapter_event_type;
    }

    data_buffer_t& get_data_buffer() { return ( _data ); }

  private:
    uint8_t         _event_type   : 3;
    uint8_t         _is_enabled   : 1;
    uint8_t         _is_connected : 1;
    uint8_t         _reserved     : 3;
    Endpoint *      end_point_    = nullptr;
    data_buffer_t _data;
    uint16_t        adapter_type_ = base::k_adapter_type_ip;
    base::ErrCode   error_code_   = base::ErrCode::OK;
};

typedef void ( *pfn_adapter_event_cb ) ( AdapterEvent *pcz_adapter_event, void *pv_user_data );
}
}
