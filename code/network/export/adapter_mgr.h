/*
 * AdapterMgr.h
 *
 *  Created on: Jun 29, 2017
 *      Author: psammand
 */

#ifndef NETWORK_EXPORT_ADAPTER_MGR_H_
#define NETWORK_EXPORT_ADAPTER_MGR_H_

#include <end_point.h>
#include <i_adapter.h>
#include <i_nwk_platform_factory.h>
#include <ip_adapter_config.h>
#include <cstdint>
#include "SingletonHolder.h"
#include "ErrCode.h"
#include <data_types.h>
#include "PtrArray.h"

constexpr uint16_t ADAPTER_MGR_MAX_NO_OF_NETWORK_HANDLERS = 5;
constexpr uint8_t  ADAPTER_MGR_MAX_NO_ADAPTERS            = 5;

using ErrCode = ja_iot::base::ErrCode;

namespace ja_iot {
namespace network {
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
    ErrCode SendUnicastData( Endpoint &end_point, const uint8_t *data, uint16_t data_length );
    ErrCode SendMulticastData( Endpoint &endPoint, const uint8_t *data, uint16_t data_length );

    void SetAdapterDataHandler( IAdapterManagerDataHandler *adapter_mgr_data_handler ) { data_handler_ = adapter_mgr_data_handler; }

    void AddAdapterNetworkHandler( IAdapterManagerNetworkHandler *adapter_mgr_network_handler ) { network_handlers_list_.Add( adapter_mgr_network_handler ); }
    void RemoveAdapterNetworkHandler( IAdapterManagerNetworkHandler *adapter_mgr_network_handler ) { network_handlers_list_.Remove( adapter_mgr_network_handler ); }

    void                     SetPlatformFactory( INetworkPlatformFactory *platform_factory ) { platform_factory_ = platform_factory; }
    INetworkPlatformFactory* GetPlatformFactory() { return ( platform_factory_ ); }

    IpAdapterConfig* get_ip_adapter_config() { return ( &ip_adapter_config_ ); }

  private:

    AdapterManager ();

    ~AdapterManager ();

  private:

    AdapterManager( const AdapterManager & )               = delete;// prevent copy constructor
    AdapterManager & operator = ( const AdapterManager & ) = delete;// prevent assignment operator

    ErrCode   init_adapter( AdapterType req_adapter_type, AdapterType to_init_adapter_type );
    void      handle_adapter_event( AdapterEvent *p_adapter_event );
    IAdapter* get_adapter_for_type( AdapterType adapter_type );
    DECLARE_ADAPTER_EVENT_HANDLER_CLASS( AdapterEventHandler, AdapterManager, handle_adapter_event );

    IpAdapterConfig         ip_adapter_config_;
    static AdapterManager * p_instance_;
    AdapterEventHandler adapter_handler_{ this };
    IAdapterManagerDataHandler * data_handler_      = nullptr;
    INetworkPlatformFactory *    platform_factory_  = nullptr;
    uint16_t                     selected_adapters_ = (uint16_t) AdapterType::TYPE_DEFAULT;

    ja_iot::base::StaticPtrArray<IAdapterManagerNetworkHandler *, ADAPTER_MGR_MAX_NO_OF_NETWORK_HANDLERS> network_handlers_list_{};
    ja_iot::base::StaticPtrArray<ja_iot::network::IAdapter *, ADAPTER_MGR_MAX_NO_ADAPTERS> adapters_list_{};
};
}
}

#endif /* NETWORK_EXPORT_ADAPTER_MGR_H_ */
