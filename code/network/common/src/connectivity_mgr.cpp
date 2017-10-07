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
#include <common/inc/msg_processor.h>

using namespace ja_iot::network;
using namespace ja_iot::base;

namespace ja_iot {
namespace network {
ConnectivityManager *ConnectivityManager::p_instance_{ nullptr };

class ConnectivityManagerImpl
{
  public:

    ConnectivityManagerImpl( ConnectivityManager *pcz_host ) : _pcz_host{ pcz_host }, _cz_msg_processor{} {}

  public:
    ConnectivityManager * _pcz_host;
    bool                  _is_initialized = false;
    MsgProcessor          _cz_msg_processor;
};


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
  pcz_impl = new ConnectivityManagerImpl( this );
}

ConnectivityManager::~ConnectivityManager ()
{
}


ErrCode ConnectivityManager::initialize( uint16_t u16_adapter_types )
{
  if( pcz_impl->_is_initialized )
  {
    DBG_ERROR( "ConnectivityManager::initialize:%d# Already Initialized", __LINE__ );
    return ( ErrCode::OK );
  }

  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "ConnectivityManager::initialize:%d# ENTER adapter_types[0x%x]", __LINE__, u16_adapter_types );

  ret_status = pcz_impl->_cz_msg_processor.initialize( u16_adapter_types );

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR( "ConnectivityManager::initialize:%d# Failed to init msg_processor", __LINE__ );
    goto exit_label_;
  }

  pcz_impl->_is_initialized = true;
  DBG_INFO( "ConnectivityManager::initialize:%d# EXIT", __LINE__ );

  return ( ret_status );

exit_label_:
  pcz_impl->_cz_msg_processor.terminate();

  return ( ret_status );
}

ErrCode ConnectivityManager::terminate()
{
  if( !pcz_impl->_is_initialized )
  {
    DBG_ERROR( "ConnectivityManager::terminate:%d# Not initialized", __LINE__ );
    return ( ErrCode::NOT_INITIALIZED );
  }

  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "ConnectivityManager::terminate:%d# ENTER", __LINE__ );

  ret_status = pcz_impl->_cz_msg_processor.terminate();

  DBG_INFO( "ConnectivityManager::terminate:%d# EXIT", __LINE__ );

  return ( ret_status );
}

ErrCode ConnectivityManager::start_listening_server()
{
  if( !pcz_impl->_is_initialized )
  {
    DBG_ERROR( "ConnectivityManager::start_listening_server:%d# Not initialized", __LINE__ );
    return ( ErrCode::NOT_INITIALIZED );
  }

  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "ConnectivityManager::start_listening_server:%d# ENTER", __LINE__ );

  ret_status = AdapterManager::Inst().StartServers();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR( "ConnectivityManager::start_listening_server:%d# StartServer FAILED", __LINE__ );
    goto exit_label_;
  }

exit_label_:
  DBG_INFO( "ConnectivityManager::start_listening_server:%d# EXIT status %d", __LINE__, (int) ret_status );

  return ( ret_status );
}

ErrCode ConnectivityManager::stop_listening_server()
{
  if( !pcz_impl->_is_initialized )
  {
    DBG_ERROR( "ConnectivityManager::stop_listening_server:%d# Not initialized", __LINE__ );
    return ( ErrCode::NOT_INITIALIZED );
  }

  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "ConnectivityManager::stop_listening_server:%d# ENTER", __LINE__ );

  ret_status = AdapterManager::Inst().StopServers();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR( "ConnectivityManager::stop_listening_server:%d# StopServer FAILED", __LINE__ );
    goto exit_label_;
  }

exit_label_:
  DBG_INFO( "ConnectivityManager::stop_listening_server:%d# EXIT status %d", __LINE__, (int) ret_status );

  return ( ret_status );
}

ErrCode ConnectivityManager::start_discovery_server()
{
  if( !pcz_impl->_is_initialized )
  {
    DBG_ERROR( "ConnectivityManager::start_discovery_server:%d# Not initialized", __LINE__ );
    return ( ErrCode::NOT_INITIALIZED );
  }

  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "ConnectivityManager::start_discovery_server:%d# ENTER", __LINE__ );

  ret_status = AdapterManager::Inst().StartServers();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR( "ConnectivityManager::start_discovery_server:%d# StartServer FAILED", __LINE__ );
    goto exit_label_;
  }

exit_label_:
  DBG_INFO( "ConnectivityManager::start_discovery_server:%d# EXIT status %d", __LINE__, (int) ret_status );

  return ( ret_status );
}

/***
 * Starts the passed adapter type.
 * @param adapter_type - need to pass single adapter type.
 * @return
 */
ErrCode ConnectivityManager::select_network( uint16_t adapter_type )
{
  if( !pcz_impl->_is_initialized )
  {
    DBG_ERROR( "ConnectivityManager::select_network:%d# Not initialized", __LINE__ );
    return ( ErrCode::NOT_INITIALIZED );
  }

  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "ConnectivityManager::select_network:%d# ENTER adapter_type[%x]", __LINE__, (int) adapter_type );

  if( IsBitSet( adapter_type, kAdapterType_ip ) )
  {
    ret_status = AdapterManager::Inst().StartAdapter( kAdapterType_ip );

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

/***
 * Stops the passed adapter type.
 * @param adapter_type - need to pass single adapter type.
 * @return
 */

ErrCode ConnectivityManager::unselect_network( uint16_t adapter_type )
{
  if( !pcz_impl->_is_initialized )
  {
    DBG_ERROR( "ConnectivityManager::unselect_network:%d# Not initialized", __LINE__ );
    return ( ErrCode::NOT_INITIALIZED );
  }

  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "ConnectivityManager::unselect_network:%d# ENTER adapter_type[%x]", __LINE__, (int) adapter_type );

  if( IsBitSet( adapter_type, kAdapterType_ip ) )
  {
    ret_status = AdapterManager::Inst().StopAdapter( kAdapterType_ip );

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

ErrCode ConnectivityManager::send_request( RequestPacket *pcz_request_packet )
{
  if( !pcz_impl->_is_initialized )
  {
    DBG_ERROR( "ConnectivityManager::send_request:%d# Not initialized", __LINE__ );
    return ( ErrCode::NOT_INITIALIZED );
  }

  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "ConnectivityManager::send_request:%d# ENTER", __LINE__ );

  if( pcz_request_packet == nullptr )
  {
    DBG_ERROR( "ConnectivityManager::send_request:%d# request_pkt NULL", __LINE__ );
    ret_status = ErrCode::INVALID_PARAMS; goto exit_label_;
  }

  pcz_impl->_cz_msg_processor.send_msg( pcz_request_packet );

exit_label_:
  DBG_INFO( "ConnectivityManager::send_request:%d# EXIT status %d", __LINE__, (int) ret_status );

  return ( ret_status );
}

ErrCode ConnectivityManager::send_response( ResponsePacket *pcz_response_packet )
{
  if( !pcz_impl->_is_initialized )
  {
    DBG_ERROR( "ConnectivityManager::send_response:%d# Not initialized", __LINE__ );
    return ( ErrCode::NOT_INITIALIZED );
  }

  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "ConnectivityManager::send_response:%d# ENTER", __LINE__ );

  if( pcz_response_packet == nullptr )
  {
    DBG_ERROR( "ConnectivityManager::send_response:%d# response_pkt NULL", __LINE__ );
    ret_status = ErrCode::INVALID_PARAMS; goto exit_label_;
  }

  pcz_impl->_cz_msg_processor.send_msg( pcz_response_packet );

exit_label_:
  DBG_INFO( "ConnectivityManager::send_response:%d# EXIT status %d", __LINE__, (int) ret_status );

  return ( ret_status );
}

ErrCode ConnectivityManager::handle_request_response()
{
  if( pcz_impl->_is_initialized )
  {
    pcz_impl->_cz_msg_processor.handle_req_res_callbacks();
  }

  return ( ErrCode::OK );
}

ErrCode ConnectivityManager::set_packet_event_handler( PacketEventHandler *pcz_packet_event_handler )
{
  if( !pcz_impl->_is_initialized )
  {
    DBG_ERROR( "ConnectivityManager::unselect_network:%d# Not initialized", __LINE__ );
    return ( ErrCode::NOT_INITIALIZED );
  }

  pcz_impl->_cz_msg_processor.set_packet_event_handler( pcz_packet_event_handler );

  return ( ErrCode::OK );
}
}
}