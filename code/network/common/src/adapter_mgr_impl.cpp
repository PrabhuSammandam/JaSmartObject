/*
 * AdapterMgrImpl.cpp
 *
 *  Created on: Jun 29, 2017
 *      Author: psammand
 */

#include <adapter_mgr.h>
#include <common/inc/common_utils.h>
#include "PtrArray.h"
#include <common/inc/logging_network.h>
#include <i_adapter.h>

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
  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "AdapterManager::InitializeAdapters:%d# ENTER adapter_types[%x]", __LINE__, adapter_types_bitmask );

  if( platform_factory_ == nullptr )
  {
    DBG_ERROR( "AdapterManager::InitializeAdapters:%d# NetworkPlatformFactory is NULL", __LINE__ );
    ret_status = ErrCode::ERR; goto exit_label_;
  }

  ret_status = init_adapter( (AdapterType) adapter_types_bitmask, AdapterType::IP );

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR( "AdapterManager::InitializeAdapters:%d# init_adapter FAILED for type[%x]", __LINE__, (int) AdapterType::IP );
    goto exit_label_;
  }

  ret_status = init_adapter( (AdapterType) adapter_types_bitmask, AdapterType::TCP );

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR( "AdapterManager::InitializeAdapters:%d# init_adapter FAILED for type[%x]", __LINE__, (int) AdapterType::TCP );
    goto exit_label_;
  }

exit_label_:
  DBG_INFO( "AdapterManager::InitializeAdapters:%d# EXIT", __LINE__ );

  return ( ret_status );
}

ErrCode AdapterManager::TerminateAdapters()
{
  if( adapters_list_.Count() > 0 )
  {
    for( int i = 0; i < adapters_list_.Count(); i++ )
    {
      auto adapter = adapters_list_.GetItem( i );

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
  ErrCode   ret_status       = ErrCode::OK;
  IAdapter *adapter_to_start = nullptr;

  DBG_INFO( "AdapterManager::StartAdapter:%d# ENTER adapter_type[%x]", __LINE__, (int) adapter_type );

  /*check if passed adapter is already initialized*/
  if( ( selected_adapters_ & (uint16_t) adapter_type ) != 0 )
  {
    goto exit_label_;
  }

  selected_adapters_ |= (uint16_t) adapter_type;

  adapter_to_start = get_adapter_for_type( adapter_type );

  if( adapter_to_start != nullptr )
  {
    ret_status = adapter_to_start->StartAdapter();

    if( ret_status != ErrCode::OK )
    {
      DBG_ERROR( "AdapterManager::StartAdapter:%d# StartAdapter FAILED", __LINE__ );
      goto exit_label_;
    }
  }
  else
  {
    DBG_ERROR( "AdapterManager::StartAdapter:%d# get_adapter_for_type FAILED", __LINE__ );
  }

exit_label_:
  DBG_INFO( "AdapterManager::StartAdapter:%d# EXIT", __LINE__ );

  return ( ret_status );
}

ErrCode AdapterManager::StopAdapter( AdapterType adapter_type )
{
  ErrCode   ret_status      = ErrCode::OK;
  IAdapter *adapter_to_stop = nullptr;

  DBG_INFO( "AdapterManager::StopAdapter:%d# ENTER adapter_type[%x]", __LINE__, (int) adapter_type );

  // be very specific to stop the adapter type
  if( ( adapter_type == AdapterType::TYPE_DEFAULT ) || ( adapter_type == AdapterType::ALL ) )
  {
    DBG_ERROR( "AdapterManager::StopAdapter:%d# Not called for specific adapter", __LINE__ );
    goto exit_label_;
  }

  /*clear from the selected adapter list*/
  selected_adapters_ &= ( ~( (uint16_t) adapter_type ) );

  adapter_to_stop = get_adapter_for_type( adapter_type );

  if( adapter_to_stop != nullptr )
  {
    ret_status = adapter_to_stop->StopAdapter();

    if( ret_status != ErrCode::OK )
    {
      DBG_ERROR( "AdapterManager::StopAdapter:%d# StopAdapter FAILED", __LINE__ );
      goto exit_label_;
    }
  }
  else
  {
    DBG_ERROR( "AdapterManager::StopAdapter:%d# get_adapter_for_type FAILED", __LINE__ );
  }

exit_label_:
  DBG_INFO( "AdapterManager::StopAdapter:%d# EXIT", __LINE__ );

  return ( ret_status );
}

ErrCode AdapterManager::StartServers()
{
  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "AdapterManager::StartServers:%d# ENTER", __LINE__ );

  if( ( selected_adapters_ & 0xFF ) == (uint16_t) AdapterType::TYPE_DEFAULT )
  {
    DBG_ERROR( "AdapterManager::StartServers:%d# Not called for specific adapter", __LINE__ );
    goto exit_label_;
  }

  for( uint16_t i = 0; i < MAX_NO_OF_ADAPTER_TYPES; i++ )
  {
    auto adapter = get_adapter_for_type( (AdapterType) ( 1 << i ) );

    if( adapter != nullptr )
    {
      ret_status = adapter->StartServer();

      if( ret_status != ErrCode::OK )
      {
        DBG_ERROR( "AdapterManager::StartServers:%d# StartServer FAILED for adapter_type[%x]", __LINE__, (int) ( 1 << i ) );
      }
    }
  }

exit_label_:
  DBG_INFO( "AdapterManager::StartServers:%d# EXIT", __LINE__ );

  return ( ret_status );
}

ErrCode AdapterManager::StopServers()
{
  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "AdapterManager::StopServers:%d# ENTER", __LINE__ );

  if( ( selected_adapters_ & 0xFF ) == (uint16_t) AdapterType::TYPE_DEFAULT )
  {
    DBG_ERROR( "AdapterManager::StopServers:%d# Not called for specific adapter", __LINE__ );
    goto exit_label_;
  }

  for( uint16_t i = 0; i < MAX_NO_OF_ADAPTER_TYPES; i++ )
  {
    auto adapter = get_adapter_for_type( (AdapterType) ( 1 << i ) );

    if( adapter != nullptr )
    {
      ret_status = adapter->StopServer();

      if( ret_status != ErrCode::OK )
      {
        DBG_ERROR( "AdapterManager::StopServers:%d# StopServer FAILED for adapter_type[%x]", __LINE__, (int) ( 1 << i ) );
      }
    }
  }

exit_label_:
  DBG_INFO( "AdapterManager::StopServers:%d# EXIT", __LINE__ );

  return ( ret_status );
}

ErrCode AdapterManager::ReadData()
{
  ErrCode ret_status = ErrCode::OK;

//  DBG_INFO( "AdapterManager::ReadData:%d# ENTER", __LINE__ );

  if( ( selected_adapters_ & 0xFF ) == (uint16_t) AdapterType::TYPE_DEFAULT )
  {
    DBG_ERROR( "AdapterManager::StopServers:%d# Not called for specific adapter", __LINE__ );
    goto exit_label_;
  }

  for( uint16_t i = 0; i < MAX_NO_OF_ADAPTER_TYPES; i++ )
  {
    auto adapter = get_adapter_for_type( (AdapterType) ( 1 << i ) );

    if( adapter != nullptr )
    {
      adapter->ReadData();
    }
  }

exit_label_:
//  DBG_INFO( "AdapterManager::ReadData:%d# EXIT", __LINE__ );

  return ( ret_status );
}

ErrCode AdapterManager::SendUnicastData( Endpoint &endpoint, const uint8_t *data, uint16_t data_length )
{
  if( ( selected_adapters_ & 0xFF ) == (uint16_t) AdapterType::TYPE_DEFAULT )
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
    auto adapter = get_adapter_for_type( (AdapterType) ( 1 << i ) );

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

ErrCode AdapterManager::SendMulticastData( Endpoint &endpoint, const uint8_t *data, uint16_t data_length )
{
  if( ( selected_adapters_ & 0xFF ) == (uint16_t) AdapterType::TYPE_DEFAULT )
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

    auto adapter = get_adapter_for_type( (AdapterType) ( 1 << i ) );

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

void AdapterManager::handle_adapter_event( AdapterEvent *p_adapter_event )
{
  DBG_INFO( "AdapterManager::handle_adapter_event:%d# ENTER event[0x%p]", __LINE__, p_adapter_event );

  if( p_adapter_event != nullptr )
  {
    DBG_INFO( "AdapterManager::handle_adapter_event:%d# event_type[%d]", __LINE__, (int)p_adapter_event->get_adapter_event_type() );

    switch( p_adapter_event->get_adapter_event_type() )
    {
      case AdapterEventType::kPacketReceived:
      {
        if( data_handler_ != nullptr )
        {
          auto end_point = *p_adapter_event->get_end_point();

          DBG_INFO( "AdapterManager::handle_adapter_event:%d# Notifying packet received, port[%d], data_length[%d]", __LINE__, end_point.getPort(), p_adapter_event->get_data_length() );

          data_handler_->HandlePacketReceived( end_point, p_adapter_event->get_data(), p_adapter_event->get_data_length() );
        }
      }
      break;
      case AdapterEventType::kErrorOccured:
      {
        if( data_handler_ != nullptr )
        {
          auto end_point = *p_adapter_event->get_end_point();

          DBG_INFO( "AdapterManager::handle_adapter_event:%d# Notifying error, port[%d], error[%d]", __LINE__, end_point.getPort(), (int)p_adapter_event->get_error_code() );

          data_handler_->HandleError( *p_adapter_event->get_end_point(), p_adapter_event->get_data(), p_adapter_event->get_data_length(), p_adapter_event->get_error_code() );
        }
      }
      break;
      case AdapterEventType::kAdapterChanged:
      {
        if( network_handlers_list_.Count() > 0 )
        {
          for( int i = 0; i < network_handlers_list_.Count(); ++i )
          {
            auto network_handler = network_handlers_list_.GetItem( i );

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
          for( int i = 0; i < network_handlers_list_.Count(); ++i )
          {
            auto network_handler = network_handlers_list_.GetItem( i );

            if( network_handler != nullptr )
            {
              network_handler->HandleConnChanged( *p_adapter_event->get_end_point(), p_adapter_event->is_connected() );
            }
          }
        }
      }
      break;
      default:
      {
      }
      break;
    }
  }
}

IAdapter * AdapterManager::get_adapter_for_type( AdapterType adapter_type )
{
  if( adapters_list_.Count() > 0 )
  {
    for( int i = 0; i < adapters_list_.Count(); ++i )
    {
      auto adapter = adapters_list_.GetItem( i );

      if( ( adapter != nullptr ) && ( adapter->GetType() == adapter_type ) )
      {
        return ( adapter );
      }
    }
  }

  return ( nullptr );
}

ErrCode AdapterManager::init_adapter( AdapterType req_adapter_type, AdapterType to_init_adapter_type )
{
  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "AdapterManager::init_adapter:%d# ENTER req_adapter_type[%x], to_init_adapter_type[%x]", __LINE__, (int) req_adapter_type, (int) to_init_adapter_type );

  if( ( req_adapter_type == AdapterType::TYPE_DEFAULT )
    || IsBitSet( (uint16_t) req_adapter_type, (uint16_t) AdapterType::ALL )
    || IsBitSet( (uint16_t) req_adapter_type, (uint16_t) to_init_adapter_type ) )
  {
    auto ip_adapter = platform_factory_->GetAdapter( to_init_adapter_type );

    if( ip_adapter == nullptr )
    {
      DBG_ERROR( "AdapterManager::init_adapter:%d# No Adapter defined for adapter_type[%x]", __LINE__, (int) to_init_adapter_type );
      ret_status = ErrCode::INVALID_PARAMS; goto exit_label_;
    }

    ip_adapter->SetAdapterHandler( &adapter_handler_ );

    ret_status = ip_adapter->Initialize();

    if( ret_status != ErrCode::OK )
    {
      DBG_ERROR( "AdapterManager::init_adapter:%d# Adapter Initialize FAILED", __LINE__ );
      goto exit_label_;
    }

    adapters_list_.Add( ip_adapter );
  }

exit_label_:
  DBG_INFO( "AdapterManager::init_adapter:%d# EXIT", __LINE__ );

  return ( ret_status );
}
}
}
