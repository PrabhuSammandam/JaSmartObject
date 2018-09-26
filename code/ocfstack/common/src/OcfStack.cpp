/*
 * ReqResEngine.cpp
 *
 *  Created on: Oct 10, 2017
 *      Author: psammand
 */

#include <OcfStack.h>
#include "i_packet_event_handler.h"
#include "ip_adapter_config.h"
#include "config_mgr.h"
#include "connectivity_mgr.h"
#include "OcfStackImpl.h"
#include "OcfResourceMgr.h"
#include "OsalMgr.h"
#include "ScopedMutex.h"
#include "OcfDeviceResource.h"

namespace ja_iot {
namespace ocfstack {
using namespace network;
using namespace base;

OcfStack *OcfStack::_p_instance{ nullptr };

/**********************************************************************************************************************/
/**************                       StackImpl DEFINITIONS            **************************************/
/**********************************************************************************************************************/
OcfStackImpl::OcfStackImpl () : _stack_req_handler{ this }
{
}

ErrCode OcfStackImpl::initialise( const OcfPlatformConfig platform_cfg )
{
  if( _is_initialised )
  {
    return ( ErrCode::OK );
  }

  if( _access_mutex == nullptr )
  {
    _access_mutex = osal::OsalMgr::Inst()->AllocMutex();

    if( _access_mutex == nullptr )
    {
      return ( ErrCode::ERR );
    }
  }

  _platform_cfg = platform_cfg;
  auto ret_status = ErrCode::OK;

  auto ip_adapter_config = ConfigManager::Inst().get_ip_adapter_config();

  if( _platform_cfg.is_server_enabled() )
  {
    ip_adapter_config->set_flag( IP_ADAPTER_CONFIG_DEVICE_TYPE_SERVER, true );
  }

  if( _platform_cfg.is_client_enabled() )
  {
    ip_adapter_config->set_flag( IP_ADAPTER_CONFIG_DEVICE_TYPE_CLIENT, true );
  }

  if( _platform_cfg.get_server_network_flags() & k_network_flag_ipv4 )
  {
    ip_adapter_config->set_flag( IP_ADAPTER_CONFIG_IPV4_UCAST, true );
    ip_adapter_config->set_flag( IP_ADAPTER_CONFIG_IPV4_MCAST, true );
  }

  if( _platform_cfg.get_server_network_flags() & k_network_flag_ipv6 )
  {
    ip_adapter_config->set_flag( IP_ADAPTER_CONFIG_IPV6_UCAST, true );
    ip_adapter_config->set_flag( IP_ADAPTER_CONFIG_IPV6_MCAST, true );
  }

  if( _platform_cfg.get_client_network_flags() & k_network_flag_ipv4 )
  {
    ip_adapter_config->set_flag( IP_ADAPTER_CONFIG_IPV4_UCAST, true );
  }

  if( _platform_cfg.get_client_network_flags() & k_network_flag_ipv6 )
  {
    ip_adapter_config->set_flag( IP_ADAPTER_CONFIG_IPV6_UCAST, true );
  }

  ret_status = ConnectivityMgr::inst().initialize( _platform_cfg.get_transport_flags() );

  if( ret_status != ErrCode::OK )
  {
    return ( ret_status );
  }

  ret_status = ConnectivityMgr::inst().select_network( _platform_cfg.get_transport_flags() );

  if( ret_status != ErrCode::OK )
  {
    return ( ret_status );
  }

  ret_status = ConnectivityMgr::inst().set_packet_callback([this] ( Packet *packet ) {
        handle_packet_event_cb( packet );
      } );

  if( _platform_cfg.is_client_enabled() )
  {
    ret_status = ConnectivityMgr::inst().start_discovery_server();

    if( ret_status != ErrCode::OK )
    {
      return ( ret_status );
    }
  }

  if( _platform_cfg.is_server_enabled() )
  {
    ret_status = ConnectivityMgr::inst().start_listening_server();

    if( ret_status != ErrCode::OK )
    {
      return ( ret_status );
    }
  }

  _is_initialised = true;

  if( _platform_cfg.is_server_enabled() )
  {
    ret_status = ResourceManager::inst().init_default_resources();

    if( ret_status != ErrCode::OK )
    {
      return ( ret_status );
    }
  }

  return ( ret_status );
}

void OcfStackImpl::handle_packet_event_cb( Packet *pcz_received_packet )
{
  if( pcz_received_packet == nullptr )
  {
    return;
  }

  ErrCode ret_status = ErrCode::OK;

  if( pcz_received_packet->get_packet_type() == ePacketType::REQ )
  {
    ret_status = _stack_req_handler.process_request( static_cast<RequestPacket *>( pcz_received_packet ) );
  }
  else if( pcz_received_packet->get_packet_type() == ePacketType::RES )
  {
    ret_status = handle_response( static_cast<ResponsePacket *>( pcz_received_packet ) );
  }
  else if( pcz_received_packet->get_packet_type() == ePacketType::ERR )
  {
    ret_status = handle_errror( static_cast<ErrorPacket *>( pcz_received_packet ) );
  }

  if( ret_status != ErrCode::OK )
  {
  }
}

ErrCode OcfStackImpl::send_direct_response( ResponsePacket *pcz_response_packet )
{
  return ( ErrCode::OK );
}

ErrCode OcfStackImpl::check_and_process_received_data() const
{
  auto ret_status = ErrCode::ERR;

  osal::ScopedMutex lock{ _access_mutex };

  ret_status = ConnectivityMgr::inst().handle_request_response();

  return ( ret_status );
}

ErrCode OcfStackImpl::set_device_info( OcfDeviceInfo &device_info ) const
{
  auto ret_status = ErrCode::ERR;

  osal::ScopedMutex lock{ _access_mutex };

  const auto device_resource = ResourceManager::inst().find_resource_by_uri( "/oic/d" );

  if( device_resource )
  {
    static_cast<OcfDeviceResource *>( device_resource )->set_device_info( device_info );
    ret_status = ErrCode::OK;
  }

  return ( ret_status );
}

ErrCode OcfStackImpl::set_platform_info( OcfPlatformInfo &platform_info ) const
{
  auto ret_status = ErrCode::ERR;

  osal::ScopedMutex lock{ _access_mutex };

  const auto platform_resource = ResourceManager::inst().find_resource_by_uri( "/oic/p" );

  if( platform_resource )
  {
    // static_cast<OcfPlatformResource*>(device_resource)->set_pla(platform_info);
    ret_status = ErrCode::OK;
  }

  return ( ret_status );
}

ErrCode OcfStackImpl::handle_response( ResponsePacket *pcz_response_packet )
{
  return ( ErrCode::OK );
}

ErrCode OcfStackImpl::handle_errror( ErrorPacket *pcz_error_packet )
{
  return ( ErrCode::OK );
}



/**********************************************************************************************************************/
/**************                       Stack DEFINITIONS            **************************************/
/**********************************************************************************************************************/
OcfStack & OcfStack::inst()
{
  if( _p_instance == nullptr )
  {
    static OcfStack instance{};
    _p_instance = &instance;
  }

  return ( *_p_instance );
}

ErrCode OcfStack::initialise( const OcfPlatformConfig platform_cfg ) const
{
  return ( _pcz_impl->initialise( platform_cfg ) );
}

ErrCode OcfStack::set_device_info( OcfDeviceInfo &device_info ) const
{
  return ( _pcz_impl->set_device_info( device_info ) );
}

ErrCode OcfStack::set_platform_info( OcfPlatformInfo &platform_info ) const
{
  return ( _pcz_impl->set_platform_info( platform_info ) );
}

ErrCode OcfStack::check_and_process_received_data() const
{
  return ( _pcz_impl->check_and_process_received_data() );
}
/*****************************************private implementation ******************************************************/
OcfStack::OcfStack ()
{
  _pcz_impl = std::make_unique<OcfStackImpl>();
}
OcfStack::~OcfStack ()
{
}

ErrCode send_direct_response( Packet *pcz_response_packet )
{
  return ( ErrCode::OK );
}
}
}