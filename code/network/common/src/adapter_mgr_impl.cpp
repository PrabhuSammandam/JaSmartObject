/*
 * AdapterMgrImpl.cpp
 *
 *  Created on: Jun 29, 2017
 *      Author: psammand
 */

#include <vector>
#include <algorithm>
#include <adapter_mgr.h>
#include <base_consts.h>
#include <common/inc/common_utils.h>
#include <common/inc/logging_network.h>
#include <i_adapter.h>
#include "config_network.h"
#include "base_utils.h"

#define __FILE_NAME__ "AdapMgrImpl"

using namespace ja_iot::network;
using namespace ja_iot::base;

namespace ja_iot {
namespace network {
void adapter_event_cb( AdapterEvent *pcz_adapter_event, void *pv_user_data );
class AdapterEventHandler : public IAdapterEventHandler
{
  public:
    void handle_event( AdapterEvent &pcz_adapter_event ) override
    {
      _adapter_mgr->handle_adapter_event( &pcz_adapter_event );
    }

    AdapterManager * _adapter_mgr = nullptr;
};

class SecureContextEventHandler : public ISecureContextEventHandler
{
  public:
    void handle_event( SecureContextEvent &rcz_secure_context_event ) override
    {
      _adapter_mgr->handle_secure_context_event( rcz_secure_context_event );
    }

    AdapterManager * _adapter_mgr = nullptr;
};

/**
 * adaptor manager
 * The adaptor manager is a singleton instance which can manage all the adaptors that are configured in the system.
 * During system initialization it is required to set all the adaptors that are available
 * in the system to the adaptor manager.
 *
 * Adaptor manager is the primary interface for all the adaptor related operations for example
 * to start the adaptor, to stop the adaptor, to start the server, to stop the server,
 * to send the unicast messages and multicast messages.
 *
 * It is required to set the call back to get the notification when a packet is arrived
 * or any error happened in the system.
 */

/* singleton instance of AdapterManager */
AdapterEventHandler       _gs_adapter_event_handler;
SecureContextEventHandler _gs_secure_context_event_handler;
AdapterManager *AdapterManager::_pcz_instance{};

AdapterManager            & AdapterManager::Inst()
{
  if( _pcz_instance == nullptr )
  {
    static AdapterManager _instance{};
    _pcz_instance = &_instance;
  }

  return ( *_pcz_instance );
}
AdapterManager::AdapterManager ()
{
  _gs_adapter_event_handler._adapter_mgr        = this;
  _gs_secure_context_event_handler._adapter_mgr = this;
  this->_adapters_list.reserve( 6 );
}
AdapterManager::~AdapterManager ()
{
}

ErrCode AdapterManager::initialize_adapters( const uint16_t u16_configured_adapter_types_bitmask )
{
  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "AdapterManager::initialize_adapters:%d# ENTER adapter_types[%x]", __LINE__, u16_configured_adapter_types_bitmask );

  if( INetworkPlatformFactory::GetCurrFactory() == nullptr )
  {
    DBG_ERROR( "AdapterManager::initialize_adapters:%d# NetworkPlatformFactory is NULL", __LINE__ );
    ret_status = ErrCode::ERR;
    goto exit_label_;
  }

  if( ( u16_configured_adapter_types_bitmask == k_adapter_type_all )
    || ( is_bit_set( u16_configured_adapter_types_bitmask, k_adapter_type_ip ) ) )
  {
    ret_status = init_adapter( u16_configured_adapter_types_bitmask, k_adapter_type_ip );

    if( ret_status != ErrCode::OK )
    {
      DBG_ERROR( "AdapterManager::initialize_adapters:%d# init_adapter FAILED for type[%x]", __LINE__, k_adapter_type_ip );
      goto exit_label_;
    }
  }

  if( ( u16_configured_adapter_types_bitmask == k_adapter_type_all )
    || ( is_bit_set( u16_configured_adapter_types_bitmask, k_adapter_type_tcp ) ) )
  {
    ret_status = init_adapter( u16_configured_adapter_types_bitmask, k_adapter_type_tcp );

    if( ret_status != ErrCode::OK )
    {
      DBG_ERROR( "AdapterManager::initialize_adapters:%d# init_adapter FAILED for type[%x]", __LINE__, k_adapter_type_tcp );
      goto exit_label_;
    }
  }

#ifdef _ENABLE_SECURE_
  ret_status = _secure_context.initialize();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR2( "Failed initializing SecureContext" );
  }

#endif

exit_label_:
  DBG_INFO( "AdapterManager::initialize_adapters:%d# EXIT", __LINE__ );

  return ( ret_status );
}

ErrCode AdapterManager::terminate_adapters()
{
  for( auto &pcz_adapter : this->_adapters_list )
  {
    pcz_adapter->terminate();
  }

#ifdef _ENABLE_SECURE_
  _secure_context.uninitialize();
#endif

  _adapters_list.clear();
  _network_handlers_list.clear();
  _event_handler = nullptr;

  // _cb_error             = nullptr;
  // _pv_error_cb_data     = nullptr;
  // _cb_packet_received   = nullptr;
  // _pv_pkt_recvd_cb_data = nullptr;

  return ( ErrCode::OK );
}

/**
 * Start the adapter. If the adapter type is already initialized then return doing nothing.
 * @param adapter_type	-	the adapter type to start. Need to pass only single adapter.
 * @return
 */
ErrCode AdapterManager::start_adapter( const uint16_t adapter_type )
{
  DBG_INFO( "AdapterManager::StartAdapter:%d# ENTER adapter_type[%x]", __LINE__, int(adapter_type) );

  /*check if passed adapter is already initialized*/
  if( ( _u16_selected_adapters & adapter_type ) != 0 )
  {
    return ( ErrCode::INVALID_PARAMS );
  }

  _u16_selected_adapters |= adapter_type;

  auto adapter_to_start = get_adapter_for_type( adapter_type );

  auto ret_status = ErrCode::OK;

  if( adapter_to_start != nullptr )
  {
    ret_status = adapter_to_start->start_adapter();

    if( ret_status != ErrCode::OK )
    {
      DBG_ERROR( "AdapterManager::StartAdapter:%d# StartAdapter FAILED", __LINE__ );
    }
  }
  else
  {
    DBG_ERROR( "AdapterManager::StartAdapter:%d# get_adapter_for_type FAILED", __LINE__ );
  }

  DBG_INFO( "AdapterManager::StartAdapter:%d# EXIT", __LINE__ );

  return ( ret_status );
}

ErrCode AdapterManager::stop_adapter( const uint16_t adapter_type )
{
  DBG_INFO( "AdapterManager::stop_adapter:%d# ENTER adapter_type[%x]", __LINE__, int(adapter_type) );

  // be very specific to stop the adapter type
  if( ( adapter_type == k_adapter_type_default ) || ( adapter_type == k_adapter_type_all ) )
  {
    DBG_ERROR( "AdapterManager::stop_adapter:%d# Not called for specific adapter", __LINE__ );
    return ( ErrCode::OK );
  }

  auto ret_status = ErrCode::OK;

  /*clear from the selected adapter list*/
  _u16_selected_adapters &= ~adapter_type;

  IAdapter *adapter_to_stop = get_adapter_for_type( adapter_type );

  if( adapter_to_stop != nullptr )
  {
    ret_status = adapter_to_stop->stop_adapter();

    if( ret_status != ErrCode::OK )
    {
      DBG_ERROR( "AdapterManager::stop_adapter:%d# stop_adapter FAILED", __LINE__ );
    }
  }
  else
  {
    DBG_ERROR( "AdapterManager::stop_adapter:%d# get_adapter_for_type FAILED", __LINE__ );
  }

  DBG_INFO( "AdapterManager::stop_adapter:%d# EXIT", __LINE__ );

  return ( ret_status );
}

ErrCode AdapterManager::start_servers() const
{
  DBG_INFO( "AdapterManager::start_servers:%d# ENTER", __LINE__ );

  if( ( _u16_selected_adapters & 0xFF ) == k_adapter_type_default )
  {
    DBG_ERROR( "AdapterManager::start_servers:%d# Not called for specific adapter", __LINE__ );
    return ( ErrCode::OK );
  }

  auto ret_status = ErrCode::OK;

  for( uint16_t i = 0; i < MAX_NO_OF_ADAPTER_TYPES; i++ )
  {
    auto adapter = get_adapter_for_type( 1 << i );

    if( adapter != nullptr )
    {
      ret_status = adapter->start_server();

      if( ret_status != ErrCode::OK )
      {
        DBG_ERROR( "AdapterManager::start_servers:%d# FAILED for adapter_type[%x]", __LINE__, int(1 << i) );
      }
    }
  }

  DBG_INFO( "AdapterManager::start_servers:%d# EXIT", __LINE__ );

  return ( ret_status );
}

/* Stop all the servers */
ErrCode AdapterManager::stop_servers() const
{
  DBG_INFO( "AdapterManager::stop_servers:%d# ENTER", __LINE__ );

  if( ( _u16_selected_adapters & 0xFF ) == k_adapter_type_default )
  {
    DBG_ERROR( "AdapterManager::stop_servers:%d# Not called for specific adapter", __LINE__ );
    return ( ErrCode::OK );
  }

  auto ret_status = ErrCode::OK;

  for( uint16_t i = 0; i < MAX_NO_OF_ADAPTER_TYPES; i++ )
  {
    auto adapter = get_adapter_for_type( 1 << i );

    if( adapter != nullptr )
    {
      ret_status = adapter->stop_server();

      if( ret_status != ErrCode::OK )
      {
        DBG_ERROR( "AdapterManager::stop_servers:%d# StopServer FAILED for adapter_type[%x]", __LINE__, int(1 << i) );
      }
    }
  }

  DBG_INFO( "AdapterManager::stop_servers:%d# EXIT", __LINE__ );

  return ( ret_status );
}

/* this api will get called only for SINGLE_THREAD */
ErrCode AdapterManager::read_data() const
{
  // DBG_INFO( "AdapterManager::ReadData:%d# ENTER", __LINE__ );

  if( ( _u16_selected_adapters & 0xFF ) == k_adapter_type_default )
  {
    DBG_ERROR( "AdapterManager::read_data:%d# Not called for specific adapter", __LINE__ );
    return ( ErrCode::OK );
  }

  for( uint16_t i = 0; i < MAX_NO_OF_ADAPTER_TYPES; i++ )
  {
    auto adapter = get_adapter_for_type( 1 << i );

    if( adapter != nullptr )
    {
      adapter->read_data();
    }
  }

  // DBG_INFO( "AdapterManager::ReadData:%d# EXIT", __LINE__ );

  return ( ErrCode::OK );
}

ErrCode AdapterManager::send_unicast_data( Endpoint &endpoint, const uint8_t *data, const uint16_t data_length )
{
  const auto adapter_type = endpoint.get_adapter_type();

  if( ( _u16_selected_adapters == k_adapter_type_default )
    || ( _u16_selected_adapters == k_adapter_type_all )
    || !is_bit_set( _u16_selected_adapters, adapter_type ) )
  {
    // no specific adapters selected
    return ( ErrCode::SEND_DATA_FAILED );
  }

#ifdef _ENABLE_SECURE_

  if( endpoint.is_secure() )
  {
    data_buffer_t data_buffer{ (uint8_t *) data, (uint16_t) data_length };

    if( _secure_context.encrypt( endpoint, data_buffer ) != ErrCode::OK )
    {
      return ( ErrCode::SEND_DATA_FAILED );
    }

    return ( ErrCode::OK );
  }

#endif

  for( uint8_t i = 0; i < _adapters_list.size(); i++ )
  {
    if( _adapters_list[i]->get_type() == adapter_type )
    {
      const auto sent_data_length = _adapters_list[i]->send_unicast_data( endpoint, data, data_length );

      if( ( 0 > sent_data_length ) || ( uint16_t( sent_data_length ) != data_length ) )
      {
        return ( ErrCode::SEND_DATA_FAILED );
      }

      break;
    }
  }

  return ( ErrCode::OK );
}

ErrCode AdapterManager::send_multicast_data( Endpoint &endpoint, const uint8_t *pu8_data, const uint16_t u16_data_length )
{
  const auto adapter_type = endpoint.get_adapter_type();

  if( ( _u16_selected_adapters == k_adapter_type_default )
    || ( _u16_selected_adapters == k_adapter_type_all )
    || !is_bit_set( _u16_selected_adapters, adapter_type ) )
  {
    // no specific adapters selected
    return ( ErrCode::SEND_DATA_FAILED );
  }

  for( uint8_t i = 0; i < _adapters_list.size(); i++ )
  {
    if( _adapters_list[i]->get_type() == adapter_type )
    {
      const auto sent_data_length = _adapters_list[i]->send_multicast_data( endpoint, pu8_data, u16_data_length );

      if( ( 0 > sent_data_length ) || ( uint16_t( sent_data_length ) != u16_data_length ) )
      {
        return ( ErrCode::SEND_DATA_FAILED );
      }

      break;
    }
  }

  return ( ErrCode::OK );
}

void AdapterManager::handle_secure_context_event( SecureContextEvent &secure_context_event )
{
  DBG_INFO2( "event[%d]", (uint8_t) secure_context_event._type );

  switch( secure_context_event._type )
  {
    case SecureContextEventType::RECEIVE:
    {
      if( _event_handler )
      {
        _event_handler->handle_packet_received( *secure_context_event._endpoint, secure_context_event._data );
      }
    }
    break;
    case SecureContextEventType::SEND:
    {
      for( uint8_t i = 0; i < _adapters_list.size(); i++ )
      {
        if( _adapters_list[i]->get_type() == secure_context_event._endpoint->get_adapter_type() )
        {
          _adapters_list[i]->send_unicast_data( *secure_context_event._endpoint,
            secure_context_event._data._pu8_data,
            secure_context_event._data._u16_data_len );
          break;
        }
      }
    }
    break;
    case SecureContextEventType::ERROR:
    {
    }
    break;
  }
}

void AdapterManager::handle_adapter_event( AdapterEvent *p_adapter_event )
{
  // DBG_INFO( "AdapterManager::handle_adapter_event:%d# ENTER event[0x%p]", __LINE__, p_adapter_event );

  if( p_adapter_event == nullptr )
  {
    return;
  }

  // DBG_INFO( "AdapterManager::handle_adapter_event:%d# event_type[%d]", __LINE__, int(p_adapter_event->get_adapter_event_type() ) );

  switch( p_adapter_event->get_adapter_event_type() )
  {
    case ADAPTER_EVENT_TYPE_PACKET_RECEIVED:
    {
      auto end_point = *p_adapter_event->get_end_point();

      if( end_point.is_secure() )
      {
#ifdef _ENABLE_SECURE_
        if( _secure_context.decrypt( end_point, p_adapter_event->get_data_buffer() ) == ErrCode::OK )
        {
        }
#endif
      }
      else
      {
        if( _event_handler )
        {
          _event_handler->handle_packet_received( end_point, p_adapter_event->get_data_buffer() );
        }
      }
    }
    break;
    case ADAPTER_EVENT_TYPE_ERROR:
    {
      // if( _cb_error )
      // {
      // auto end_point = *p_adapter_event->get_end_point();
      //
      // DBG_INFO( "AdapterManager::handle_adapter_event:%d# Notifying error, port[%d], error[%d]", __LINE__, end_point.get_port(), int(p_adapter_event->get_error_code() ) );
      // _cb_error( this->_pv_error_cb_data, *p_adapter_event->get_end_point(), p_adapter_event->get_data(), p_adapter_event->get_data_length(), p_adapter_event->get_error_code() );
      // }
      if( _event_handler )
      {
        auto end_point = *p_adapter_event->get_end_point();
        _event_handler->handle_error( end_point, p_adapter_event->get_data_buffer(), p_adapter_event->get_error_code() );
      }
    }
    break;
    case ADAPTER_EVENT_TYPE_ADAPTER_CHANGED:
    {
      std::for_each( _network_handlers_list.cbegin(), _network_handlers_list.cend(),
        [&] ( IAdapterMgrNetworkHandler *item )
          {
            item->handle_adapter_changed( p_adapter_event->get_adapter_type(), p_adapter_event->is_enabled() );
          } );
    }
    break;
    case ADAPTER_EVENT_TYPE_CONNECTION_CHANGED:
    {
      std::for_each( _network_handlers_list.cbegin(), _network_handlers_list.cend(),
        [&] ( IAdapterMgrNetworkHandler *item )
          {
            item->handle_conn_changed( *p_adapter_event->get_end_point(), p_adapter_event->is_connected() );
          } );
    }
    break;
    default:
    {
    }
    break;
  }
}

std::vector<Endpoint *> AdapterManager::get_endpoints_list()
{
  std::vector<Endpoint *> cz_endpoint_list{};

  for( auto &pcz_adapter : _adapters_list )
  {
    pcz_adapter->get_endpoints_list( cz_endpoint_list );
  }

  return ( cz_endpoint_list );
}

void AdapterManager::remove_adapter_network_handler( IAdapterMgrNetworkHandler *adapter_mgr_network_handler )
{
  if( adapter_mgr_network_handler != nullptr )
  {
    _network_handlers_list.erase( find( _network_handlers_list.cbegin(), _network_handlers_list.cend(),
      adapter_mgr_network_handler ) );
  }
}

IAdapter * AdapterManager::get_adapter_for_type( const uint16_t u16_adapter_type ) const
{
  for( auto &pcz_adapter : this->_adapters_list )
  {
    if( pcz_adapter->get_type() == u16_adapter_type )
    {
      return ( pcz_adapter );
    }
  }

  return ( nullptr );
}

ErrCode AdapterManager::init_adapter( const uint16_t u16_configured_adapter_types_bitmask, const uint16_t to_init_adapter_type )
{
  auto ret_status = ErrCode::OK;
  IAdapter *pcz_ip_adapter{};

  DBG_INFO( "AdapterManager::init_adapter:%d# ENTER req_adapter_type[%x], to_init_adapter_type[%x]", __LINE__, int(u16_configured_adapter_types_bitmask), int(to_init_adapter_type) );

  auto platform_factory = INetworkPlatformFactory::GetCurrFactory();

  if( platform_factory == nullptr )
  {
    DBG_ERROR( "AdapterManager::init_adapter:%d# PlatformFactory NULL", __LINE__ );
    ret_status = ErrCode::INVALID_PARAMS;
    goto exit_label_;
  }

  pcz_ip_adapter = platform_factory->get_adapter( to_init_adapter_type );

  if( pcz_ip_adapter == nullptr )
  {
    DBG_ERROR( "AdapterManager::init_adapter:%d# No Adapter defined for adapter_type[%x]", __LINE__, int(to_init_adapter_type) );
    ret_status = ErrCode::INVALID_PARAMS;
    goto exit_label_;
  }

  pcz_ip_adapter->set_event_handler( &_gs_adapter_event_handler );
  // pcz_ip_adapter->set_adapter_event_cb( adapter_event_cb, this /* user data */ );

  ret_status = pcz_ip_adapter->initialize();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR( "AdapterManager::init_adapter:%d# Adapter Initialize FAILED", __LINE__ );
    goto exit_label_;
  }

  _adapters_list.push_back( pcz_ip_adapter );

exit_label_:
  DBG_INFO( "AdapterManager::init_adapter:%d# EXIT", __LINE__ );

  return ( ret_status );
}

void adapter_event_cb( AdapterEvent *pcz_adapter_event, void *pv_user_data )
{
  if( pv_user_data != nullptr )
  {
    static_cast<AdapterManager *>( pv_user_data )->handle_adapter_event( pcz_adapter_event );
  }
}
}
}
