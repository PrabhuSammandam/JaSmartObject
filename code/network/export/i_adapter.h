/*
 * 1 * IAdapter.h
 *
 *  Created on: Jun 30, 2017
 *      Author: psammand
 */

#ifndef NETWORK_EXPORT_I_ADAPTER_H_
#define NETWORK_EXPORT_I_ADAPTER_H_

#include <end_point.h>
#include "PtrArray.h"
#include <ErrCode.h>
#include <data_types.h>
#include <cstdint>

namespace ja_iot {
namespace network {
using ErrCode = ja_iot::base::ErrCode;

/***
 * Enum that defines the types of adapter event.
 */
enum class AdapterEventType
{
  kNone,
  kPacketReceived,
  kErrorOccured,
  kAdapterChanged,
  kConnectionChanged
};

class AdapterEvent
{
  public:

    AdapterEvent( AdapterEventType adapter_event_type )
      : adapter_event_type_{ adapter_event_type } {}

    AdapterType get_adapter_type() const { return ( adapter_type_ ); }

    void set_adapter_type( AdapterType adapter_type ) { adapter_type_ = adapter_type; }

    uint8_t* get_data() const { return ( data_ ); }
    void     set_data( uint8_t *data ) { data_ = data; }

    uint16_t get_data_length() const { return ( data_length_ ); }
    void     set_data_length( uint16_t data_length ) { data_length_ = data_length; }

    const Endpoint* get_end_point() const { return ( end_point_ ); }
    void            set_end_point( Endpoint *end_point ) { end_point_ = end_point; }

    ErrCode get_error_code() const { return ( error_code_ ); }
    void    set_error_code( ErrCode error_code ) { error_code_ = error_code; }

    bool is_connected() const { return ( is_connected_ ); }
    void set_is_connected( bool is_connected ) { is_connected_ = is_connected; }

    bool is_enabled() const { return ( is_enabled_ ); }
    void set_is_enabled( bool is_enabled ) { is_enabled_ = is_enabled; }

    AdapterEventType get_adapter_event_type() const { return ( adapter_event_type_ ); }

    void set_adapter_event_type( AdapterEventType adapter_event_type ) { adapter_event_type_ = adapter_event_type; }

  private:
    bool               is_enabled_         = false;
    bool               is_connected_       = false;
    Endpoint *         end_point_          = nullptr;
    uint8_t *          data_               = nullptr;
    uint16_t           data_length_        = 0;
    ErrCode            error_code_         = ErrCode::OK;
    AdapterType        adapter_type_       = AdapterType::IP;
    AdapterEventType   adapter_event_type_ = AdapterEventType::kNone;
};

/***
 *
 */
class IAdapterEventHandler
{
  public:

    virtual ~IAdapterEventHandler () {}

    virtual void handle_adapter_event( AdapterEvent *p_adapter_event ) = 0;
};

class IAdapter
{
  public:

    virtual ~IAdapter () {}

    virtual ErrCode Initialize() = 0;
    virtual ErrCode Terminate()  = 0;

    virtual ErrCode StartAdapter() = 0;
    virtual ErrCode StopAdapter()  = 0;

    virtual ErrCode StartServer() = 0;
    virtual ErrCode StopServer()  = 0;

    virtual ErrCode StartListening() = 0;
    virtual ErrCode StopListening()  = 0;

    virtual int32_t SendUnicastData( Endpoint &end_point, const uint8_t *data, uint16_t data_length ) = 0;

    virtual int32_t SendMulticastData( Endpoint &end_point, const uint8_t *data, uint16_t data_length ) = 0;

    virtual void ReadData() = 0;

    virtual AdapterType GetType() = 0;

    virtual void SetAdapterHandler( IAdapterEventHandler *adapter_event_handler ) = 0;
};

using IAdapterPtrArray = ja_iot::base::PtrArray<IAdapter *>;

#define DECLARE_ADAPTER_EVENT_HANDLER_CLASS( HNDLR_CLASS, HOST, HNDL_FUNC ) class HNDLR_CLASS : public ja_iot::network::IAdapterEventHandler \
{ \
    public: \
      HNDLR_CLASS( HOST * host ) : host_{ host } {} \
      void handle_adapter_event( AdapterEvent * p_adapter_event ) override{ host_->HNDL_FUNC( p_adapter_event ); } \
    private: \
      HOST * host_; \
}; \

}
}

#endif /* NETWORK_EXPORT_I_ADAPTER_H_ */
