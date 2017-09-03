/*
 * AdapterMgrImpl.cpp
 *
 *  Created on: Jun 29, 2017
 *      Author: psammand
 */

#include <common/inc/common_utils.h>
#include "AdapterMgr.h"
#include "PtrArray.h"
#include "IAdapter.h"
#include "AdapterProxyStore.h"

using namespace ja_iot::network;

namespace ja_iot {
namespace network {

/**
 * adaptor manager
 * The adaptor manager is a singleton instance which can manage all the  adaptors that are configured in the system.
 * During system initialization it is required to  set all the adaptors that are available
 * in the system to the adaptor manager.
 *
 * Adaptor manager is the primary interface for all the adaptor related operations for example
 * to start the adaptor, to stop the adaptor, to start the server, to stop the server,
 * to send the unicast messages and multicast messages.
 *
 * It is required to set the call back  to get the notification when a packet is arrived
 * or any error happened in the system.
 */
AdapterManager *AdapterManager::p_instance_{ nullptr };

AdapterManager & AdapterManager::Inst()
{
  if( p_instance_ == nullptr )
  {
    static AdapterManager _instance{};
    p_instance_ = &_instance;
  }

  return ( *p_instance_ );
}

AdapterManager::AdapterManager ()
{
}

AdapterManager::~AdapterManager ()
{
}

/**
 * Initialize the adapters.
 * @param adapter_types	- bitmask of adapter types to initialize.
 * @return
 */
ErrCode AdapterManager::InitializeAdapters( uint16_t adapter_types_bitmask )
{
  if( platform_factory_ == nullptr )
  {
    return ( ErrCode::ERR );
  }

  InitAdapter( (AdapterType) adapter_types_bitmask, AdapterType::IP );
  InitAdapter( (AdapterType) adapter_types_bitmask, AdapterType::TCP );

  return ( ErrCode::OK );
}

ErrCode AdapterManager::TerminateAdapters()
{
  if( adapters_list_.Count() > 0 )
  {
    for( auto &adapter : adapters_list_ )
    {
      if( adapter != nullptr )
      {
        adapter->Terminate();
      }
    }
  }

  adapters_list_.Clear();
  data_handler_ = nullptr;
  network_handlers_list_.Clear();

  return ( ErrCode::OK );
}

/**
 * Start the adapter. If the adapter type is already initialized then return doing nothing.
 * @param adapter_type	-	the adapter type to start.
 * @return
 */
ErrCode AdapterManager::StartAdapter( AdapterType adapter_type )
{
  /*check for the passed adapter is already initialized*/
  if( ( selected_adapters_ & (uint16_t) adapter_type ) != 0 )
  {
    return ( ErrCode::OK );
  }

  selected_adapters_ |= (uint16_t) adapter_type;

  auto adapter_to_start = GetAdapterForType( adapter_type );

  if( adapter_to_start != nullptr )
  {
    return ( adapter_to_start->StartAdapter() );
  }

  return ( ErrCode::ERR );
}

ErrCode AdapterManager::StopAdapter( AdapterType adapter_type )
{
  // be very specific to stop the adapter type
  if( ( adapter_type == AdapterType::DEFAULT ) || ( adapter_type == AdapterType::ALL ) )
  {
    return ( ErrCode::OK );
  }

  /*clear from the selected adapter list*/
  selected_adapters_ &= ( ~( (uint16_t) adapter_type ) );

  auto adapter = GetAdapterForType( adapter_type );

  if( adapter != nullptr )
  {
    return ( adapter->StopAdapter() );
  }

  return ( ErrCode::ERR );
}

ErrCode AdapterManager::StartServers()
{
  ErrCode ret_status = ErrCode::ERR;

  if( ( selected_adapters_ & 0xFF ) == (uint16_t) AdapterType::DEFAULT )
  {
    // no specific adapters selected
    return ( ret_status );
  }

  for( uint16_t i = 0; i < MAX_NO_OF_ADAPTER_TYPES; i++ )
  {
    auto adapter = GetAdapterForType( (AdapterType) ( 1 << i ) );

    if( ( adapter != nullptr ) && ( adapter->StartServer() == ErrCode::OK ) )
    {
      ret_status = ErrCode::OK;
    }
  }

  return ( ret_status );
}

ErrCode AdapterManager::StopServers()
{
  if( ( selected_adapters_ & 0xFF ) == (uint16_t) AdapterType::DEFAULT )
  {
    // no specific adapters selected
    return ( ErrCode::ERR );
  }

  for( uint16_t i = 0; i < MAX_NO_OF_ADAPTER_TYPES; i++ )
  {
    auto adapter = GetAdapterForType( (AdapterType) ( 1 << i ) );

    if( adapter != nullptr )
    {
      adapter->StopServer();
    }
  }

  return ( ErrCode::OK );
}

ErrCode AdapterManager::ReadData()
{
  if( ( selected_adapters_ & 0xFF ) == (uint16_t) AdapterType::DEFAULT )
  {
    // no specific adapters selected
    return ( ErrCode::ERR );
  }

  for( uint16_t i = 0; i < MAX_NO_OF_ADAPTER_TYPES; i++ )
  {
    auto adapter = GetAdapterForType( (AdapterType) ( 1 << i ) );

    if( adapter != nullptr )
    {
      adapter->ReadData();
    }
  }

  return ( ErrCode::OK );
}

ErrCode AdapterManager::SendUnicastData( const Endpoint &endpoint, const uint8_t *data, uint16_t data_length )
{
  if( ( selected_adapters_ & 0xFF ) == (uint16_t) AdapterType::DEFAULT )
  {
    // no specific adapters selected
    return ( ErrCode::SEND_DATA_FAILED );
  }

  auto requested_adapters = (uint16_t) AdapterType::ALL;

  for( uint16_t i = 0; i < MAX_NO_OF_ADAPTER_TYPES; i++ )
  {
    if( ( requested_adapters & ( 1 << i ) ) == 0 )
    {
      continue; // adapter not enabled skip it
    }

    /*get the required adaptor type*/
    auto adapter = GetAdapterForType( (AdapterType) ( 1 << i ) );

    if( adapter != nullptr )
    {
      int32_t sent_data_length = adapter->SendUnicastData( endpoint, data, data_length );

      if( ( 0 > sent_data_length ) || ( (uint16_t) sent_data_length != data_length ) )
      {
        return ( ErrCode::SEND_DATA_FAILED );
      }
    }
  }

  return ( ErrCode::OK );
}

ErrCode AdapterManager::SendMulticastData( const Endpoint &endpoint, const uint8_t *data, uint16_t data_length )
{
  if( ( selected_adapters_ & 0xFF ) == (uint16_t) AdapterType::DEFAULT )
  {
    return ( ErrCode::SEND_DATA_FAILED );
  }

  auto requested_adapters = (uint16_t) AdapterType::ALL;

  for( uint16_t i = 0; i < MAX_NO_OF_ADAPTER_TYPES; i++ )
  {
    if( ( requested_adapters & ( 1 << i ) ) == 0 )
    {
      continue;
    }

    auto adapter = GetAdapterForType( (AdapterType) ( 1 << i ) );

    if( adapter != nullptr )
    {
      int32_t sent_data_length = adapter->SendMulticastData( endpoint, data, data_length );

      if( ( 0 > sent_data_length ) || ( (uint16_t) sent_data_length != data_length ) )
      {
        return ( ErrCode::SEND_DATA_FAILED );
      }
    }
  }

  return ( ErrCode::OK );
}

void AdapterManager::handle_adapter_event( AdapterEvent* p_adapter_event) {
	if(p_adapter_event != nullptr)
	{
		switch(p_adapter_event->get_adapter_event_type())
		{
		case AdapterEventType::kPacketReceived:
		{
			  if( data_handler_ != nullptr )
			  {
			    data_handler_->HandlePacketReceived( *p_adapter_event->get_end_point(), p_adapter_event->get_data(), p_adapter_event->get_data_length() );
			  }
		}
		break;
		case AdapterEventType::kErrorOccured:
		{
			  if( data_handler_ != nullptr )
			  {
			    data_handler_->HandleError( *p_adapter_event->get_end_point(), p_adapter_event->get_data(), p_adapter_event->get_data_length(),  p_adapter_event->get_error_code());
			  }
		}
		break;
		case AdapterEventType::kAdapterChanged:
		{
			  if( network_handlers_list_.Count() > 0 )
			  {
			    for( auto &network_handler : network_handlers_list_ )
			    {
			      if( network_handler != nullptr )
			      {
			        network_handler->HandlerAdapterChanged( p_adapter_event->get_adapter_type(), p_adapter_event->is_enabled() );
			      }
			    }
			  }
		}
		break;
		case AdapterEventType::kConnectionChanged:
		{
			  if( network_handlers_list_.Count() > 0 )
			  {
			    for( auto &network_handler : network_handlers_list_ )
			    {
			      if( network_handler != nullptr )
			      {
			        network_handler->HandleConnChanged( *p_adapter_event->get_end_point(), p_adapter_event->is_connected() );
			      }
			    }
			  }
		}
		break;
		}
	}
}

IAdapter * AdapterManager::GetAdapterForType( AdapterType adapter_type )
{
  if( adapters_list_.Count() > 0 )
  {
    for( auto &adapter : adapters_list_ )
    {
      if( ( adapter != nullptr ) && ( adapter->GetType() == adapter_type ) )
      {
        return ( adapter );
      }
    }
  }

  return ( nullptr );
}

ErrCode AdapterManager::InitAdapter( AdapterType req_adapter_type, AdapterType to_init_adapter_type )
{
  if( ( req_adapter_type == AdapterType::DEFAULT )
    || IsBitSet( (uint16_t) req_adapter_type, (uint16_t) AdapterType::ALL )
    || IsBitSet( (uint16_t) req_adapter_type, (uint16_t) to_init_adapter_type ) )
  {
    auto ip_adapter = platform_factory_->GetAdapter( to_init_adapter_type );

    if( ip_adapter == nullptr )
    {
      return ( ErrCode::INVALID_PARAMS );
    }

    ip_adapter->SetAdapterHandler( &adapter_handler_ );
    ip_adapter->Initialize();
    adapters_list_.Add( ip_adapter );
  }

  return ( ErrCode::OK );
}
}
}

