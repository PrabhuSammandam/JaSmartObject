/*
 * ConnectivityManager.cpp
 *
 *  Created on: Sep 15, 2017
 *      Author: psammand
 */

#include <adapter_mgr.h>
#include <stdio.h>
#include <common/inc/common_utils.h>
#include <common/inc/logging_network.h>
#include <connectivity_mgr.h>

using namespace ja_iot::network;

namespace ja_iot {
namespace network {
ConnectivityManager *ConnectivityManager::p_instance_{ nullptr };

ConnectivityManager & ConnectivityManager::Inst()
{
  if( p_instance_ == nullptr )
  {
    static ConnectivityManager _instance{};
    p_instance_ = &_instance;
  }

  return ( *p_instance_ );
}

ConnectivityManager::ConnectivityManager ()
{
}

ConnectivityManager::~ConnectivityManager ()
{
}


ErrCode ConnectivityManager::initialize( AdapterType adapter_types )
{
  if( is_initialized_ )
  {
    DBG_ERROR( "ConnectivityManager::initialize:%d# Already Initialized", __LINE__ );
    return ( ErrCode::OK );
  }

  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "ConnectivityManager::initialize:%d# ENTER adapter_types[%x]", __LINE__, (int) adapter_types );

  is_initialized_ = true;
  DBG_INFO( "ConnectivityManager::initialize:%d# EXIT", __LINE__ );

  return ( ret_status );
}

ErrCode ConnectivityManager::terminate()
{
  if( !is_initialized_ )
  {
    DBG_ERROR( "ConnectivityManager::terminate:%d# Not initialized", __LINE__ );
    return ( ErrCode::NOT_INITIALIZED );
  }

  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "ConnectivityManager::terminate:%d# ENTER", __LINE__ );

  DBG_INFO( "ConnectivityManager::terminate:%d# EXIT", __LINE__ );

  return ( ret_status );
}

ErrCode ConnectivityManager::start_listening_server()
{
  return ( ErrCode::OK );
}

ErrCode ConnectivityManager::stop_listening_server()
{
  return ( ErrCode::OK );
}

ErrCode ConnectivityManager::start_discovery_server()
{
  return ( ErrCode::OK );
}

ErrCode ConnectivityManager::select_network( AdapterType adapter_type )
{
  if( !is_initialized_ )
  {
    DBG_ERROR( "ConnectivityManager::select_network:%d# Not initialized", __LINE__ );
    return ( ErrCode::NOT_INITIALIZED );
  }

  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "ConnectivityManager::select_network:%d# ENTER adapter_type[%x]", __LINE__, (int) adapter_type );

  if( IsBitSet( adapter_type, AdapterType::IP ) )
  {
    ret_status = AdapterManager::Inst().StartAdapter( AdapterType::IP );

    if( ret_status != ErrCode::OK )
    {
      DBG_ERROR( "ConnectivityManager::select_network:%d# Failed StartAdapter for IP", __LINE__ );
      goto exit_label_;
    }
  }

exit_label_:
  DBG_INFO( "ConnectivityManager::select_network:%d# EXIT", __LINE__ );

  return ( ret_status );
}

ErrCode ConnectivityManager::unselect_network( AdapterType adapter_type )
{
  if( !is_initialized_ )
  {
    DBG_ERROR( "ConnectivityManager::unselect_network:%d# Not initialized", __LINE__ );
    return ( ErrCode::NOT_INITIALIZED );
  }

  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "ConnectivityManager::unselect_network:%d# ENTER adapter_type[%x]", __LINE__, (int) adapter_type );

  if( IsBitSet( adapter_type, AdapterType::IP ) )
  {
    ret_status = AdapterManager::Inst().StopAdapter( AdapterType::IP );

    if( ret_status != ErrCode::OK )
    {
      DBG_ERROR( "ConnectivityManager::unselect_network:%d# Failed StopAdapter for IP", __LINE__ );
      goto exit_label_;
    }
  }

exit_label_:
  DBG_INFO( "ConnectivityManager::unselect_network:%d# EXIT", __LINE__ );

  return ( ret_status );
}

ErrCode ConnectivityManager::send_request( Endpoint *end_point )
{
  return ( ErrCode::OK );
}

ErrCode ConnectivityManager::send_response( Endpoint *end_point )
{
  return ( ErrCode::OK );
}

ErrCode ConnectivityManager::handle_request_response()
{
  if( is_initialized_ )
  {
    AdapterManager::Inst().ReadData();
  }

  return ( ErrCode::OK );
}

ErrCode ConnectivityManager::set_event_handler( IConnectivityManagerEventHandler *event_handler )
{
  return ( ErrCode::OK );
}

/**************************ConnectivityManagerEvent*****************************/

ConnectivityManagerEvent::ConnectivityManagerEvent( ConnectivityManagerEventType event_type )
{
}
}
}
