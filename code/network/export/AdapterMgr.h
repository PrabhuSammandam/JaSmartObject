/*
 * AdapterMgr.h
 *
 *  Created on: Jun 29, 2017
 *      Author: psammand
 */

#ifndef NETWORK_EXPORT_ADAPTERMGR_H_
#define NETWORK_EXPORT_ADAPTERMGR_H_

#include <INetworkPlatformFactory.h>
#include <cstdint>
#include "SingletonHolder.h"
#include "ErrCode.h"
#include "EndPoint.h"
#include "TransportType.h"
#include "IAdapter.h"
#include "PtrArray.h"

constexpr uint16_t MAX_NO_OF_NETWORK_HANDLERS = 5;
constexpr uint8_t  MAX_NO_ADAPTERS            = 5;

using ErrCode = ja_iot::base::ErrCode;

namespace ja_iot {
namespace network {
class AdapterManagerImplData;

class IAdapterManagerDataHandler
{
  public:

    virtual ~IAdapterManagerDataHandler () {}

    virtual void HandlePacketReceived( Endpoint const &end_point, const uint8_t *data, uint16_t data_len )       = 0;
    virtual void HandleError( Endpoint const &end_point, const uint8_t *data, uint16_t data_len, ErrCode error ) = 0;
};

class IAdapterManagerNetworkHandler
{
  public:

    virtual ~IAdapterManagerNetworkHandler () {}

    virtual void HandlerAdapterChanged( AdapterType adapter_type, bool is_enabled ) = 0;
    virtual void HandleConnChanged( Endpoint const &end_point, bool is_connected )  = 0;
};

class AdapterManager
{
  public:
    static AdapterManager& Inst();

  public:

    ErrCode InitializeAdapters( uint16_t adapter_types );
    ErrCode TerminateAdapters();

    ErrCode StartAdapter( AdapterType adapter_type );
    ErrCode StopAdapter( AdapterType adapter_type );

    ErrCode StartServers();
    ErrCode StopServers();

    ErrCode ReadData();
    ErrCode SendUnicastData( Endpoint const &end_point, const uint8_t *data, uint16_t data_length );
    ErrCode SendMulticastData( Endpoint const &endPoint, const uint8_t *data, uint16_t data_length );

    void SetAdapterDataHandler( IAdapterManagerDataHandler *adapter_mgr_data_handler ) { data_handler_ = adapter_mgr_data_handler; }

    void AddAdapterNetworkHandler( IAdapterManagerNetworkHandler *adapter_mgr_network_handler ) { network_handlers_list_.Add( adapter_mgr_network_handler ); }
    void RemoveAdapterNetworkHandler( IAdapterManagerNetworkHandler *adapter_mgr_network_handler ) { network_handlers_list_.Remove( adapter_mgr_network_handler ); }

    void                     SetPlatformFactory( INetworkPlatformFactory *platform_factory ) { platform_factory_ = platform_factory; }
    INetworkPlatformFactory* GetPlatformFactory() { return ( platform_factory_ ); }

  private:

    AdapterManager ();

    ~AdapterManager ();

  private:

    AdapterManager( const AdapterManager & )               = delete;
    AdapterManager & operator = ( const AdapterManager & ) = delete;

    ErrCode InitAdapter( AdapterType req_adapter_type, AdapterType to_init_adapter_type );

    void handle_adapter_event( AdapterEvent *p_adapter_event );

    IAdapter* GetAdapterForType( AdapterType adapterType );

    class AdapterEventHandler : public IAdapterEventHandler
    {
      public:

        AdapterEventHandler( AdapterManager *adapter_mgr ) : adapter_mgr_{ adapter_mgr } {}

        void handle_adapter_event( AdapterEvent *p_adapter_event ) override { adapter_mgr_->handle_adapter_event( p_adapter_event ); }

      private:

        AdapterManager * adapter_mgr_;
    };

    AdapterEventHandler adapter_handler_{ this };
    IAdapterManagerDataHandler *data_handler_{ nullptr };
    ja_iot::base::StaticPtrArray<IAdapterManagerNetworkHandler *, MAX_NO_OF_NETWORK_HANDLERS> network_handlers_list_{};
    ja_iot::base::StaticPtrArray<ja_iot::network::IAdapter *, MAX_NO_ADAPTERS> adapters_list_{};
    uint16_t selected_adapters_{ (uint16_t) AdapterType::DEFAULT };

    INetworkPlatformFactory *platform_factory_{ nullptr };

    static AdapterManager * p_instance_;
};
}
}

#endif /* NETWORK_EXPORT_ADAPTERMGR_H_ */
