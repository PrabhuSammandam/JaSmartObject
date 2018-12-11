/*
 * IpAdapterBase.cpp
 *
 *  Created on: Jul 3, 2017
 *      Author: psammand
 */

#include <vector>
#include <adapter_mgr.h>
#include "OsalMgr.h"
#include <cstdint>
#include <string>
#include <string.h>
#include <common/inc/logging_network.h>
#include <i_nwk_platform_factory.h>
#include <common/inc/ip_adapter_base.h>
#include <ip_adapter_config.h>
#include <config_mgr.h>
#include "IMemAllocator.h"
#include "config_network.h"
#include "base_utils.h"
#include "ScopedMutex.h"
#include "common/inc/SocketHelper.h"
#include "common/inc/common_utils.h"

#define __FILE_NAME__ "IpAdapterBase"

using namespace ja_iot::osal;
using namespace ja_iot::base;

namespace ja_iot {
namespace network {
using namespace base;

static void ip_adapter_sender_task_handle_msg_cb( void *pv_task_arg, void *pv_user_data );
static void ip_adapter_sender_task_delete_msg_cb( void *pv_task_arg, void *pv_user_data );
static void join_mcast_group( IUdpSocket *udp_socket, IpAddress &ip_multicast_addr, const uint32_t if_index );
IpAdapterBase::IpAdapterBase ()
{
}
IpAdapterBase::~IpAdapterBase ()
{
}

ErrCode IpAdapterBase::initialize()
{
  DBG_INFO2( "ENTER" );

  /* call platform specific initialize */
  auto ret_status = do_pre_intialize();

  sender_task_mutex_ = OsalMgr::Inst()->AllocMutex();

  if( sender_task_mutex_ == nullptr )
  {
    DBG_ERROR2( "SendMsgQ mutex creation FAILED" );
    ret_status = ErrCode::ERR;
    goto exit_label_;
  }

  _access_mutex = OsalMgr::Inst()->AllocMutex();

  if( _access_mutex == nullptr )
  {
    DBG_ERROR2( "Access mutex creation FAILED" );
    ret_status = ErrCode::ERR;
    goto exit_label_;
  }

  /* call platform specific post initialize */
  ret_status = do_post_intialize();

  // JA_ERR_OK_PRINT_GO( ret_status, " DoPostIntialize() FAILED" );

exit_label_:
  DBG_INFO2( "EXIT %d", int(ret_status) );

  return ( ret_status );
}

ErrCode IpAdapterBase::terminate()
{
  DBG_INFO2( "ENTER" );

  auto ret_status = do_pre_terminate();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR2( "do_pre_terminate FAILED" );
    goto exit_label_;
  }

  ret_status = terminate_internal();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR2( "terminate_internal FAILED" );
    goto exit_label_;
  }

  if( sender_task_ != nullptr )
  {
    sender_task_->Destroy();
    OsalMgr::Inst()->FreeTask( sender_task_ );
    sender_task_ = nullptr;
  }

  if( sender_task_mutex_ )
  {
    OsalMgr::Inst()->FreeMutex( sender_task_mutex_ );
    sender_task_mutex_ = nullptr;
  }

  if( _access_mutex != nullptr )
  {
    OsalMgr::Inst()->FreeMutex( _access_mutex );
    _access_mutex = nullptr;
  }

  ret_status = do_post_terminate();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR2( "do_post_terminate FAILED" );
  }

exit_label_:
  DBG_INFO2( "EXIT status %d", int(ret_status) );
  return ( ret_status );
}

/*
 * Start the adapter. Following are needs to done
 * 1. Start the interface monitor. DONE
 * 2. Create the send thread. DONE
 * 3. Create the endpoint list
 * 4. Start the send thread. DONE
 * 5. Start the ip server
 *              1. Initialize the windows socket layer. DONE
 *              2. initialize the multicast addresses.
 *              3. if ipv6 enabled, create the ipv6 sockets for both unicast and multicast. DONE
 *              4. if ipv4 enabled, create the ipv4 sockets for both unicast and multicast. DONE
 *              5. initialize fast shutdown mechanism. DONE
 *              6. register for interface address change
 *              7. get all the address available now
 *              8. join to multicast group for both ipv4 & ipv6
 *              9. start the receiving thread.
 * */
ErrCode IpAdapterBase::start_adapter()
{
  DBG_INFO2( "ENTER" );

  /* start the sending thread */
  auto ret_status = start_sending_thread();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR2( "create_and_start_sending_thread FAILED" );
    goto exit_label_;
  }

  ret_status = start_server();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR2( "start_server FAILED" );
  }

exit_label_:
  DBG_INFO2( "EXIT status %d", int(ret_status) );

  return ( ErrCode::OK );
}

ErrCode IpAdapterBase::stop_adapter()
{
  DBG_INFO2( "ENTER" );

  if( sender_task_->Stop() != OsalError::OK )
  {
    DBG_ERROR2( "FAILED stopping SenderTask" );
    return ( ErrCode::ERR );
  }

  auto ret_status = stop_server();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR2( "stop_server FAILED" );
    ret_status = ErrCode::ERR;
  }

  DBG_INFO2( " EXIT status %d", int(ret_status) );

  return ( ret_status );
}

/**
 * \brief Start the IP server.
 * \return
 */
ErrCode IpAdapterBase::start_server()
{
  if( is_started_ == true )
  {
    DBG_WARN2( "start_server called again" );
    return ( ErrCode::OK );
  }

  DBG_INFO2( " ENTER" );

  auto ret_status = do_pre_start_server();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR2( " do_pre_start_server FAILED" );
    return ( ret_status );
  }

  const auto ip_adapter_config = ConfigManager::Inst().get_ip_adapter_config();

  if( ip_adapter_config->is_ipv6_enabled() )
  {
    ret_status = open_ipv6_sockets();

    if( ret_status != ErrCode::OK )
    {
      DBG_ERROR2( " open_ipv6_sockets FAILED" );
      goto exit_label_;
    }
  }

  if( ip_adapter_config->is_ipv4_enabled() )
  {
    ret_status = open_ipv4_sockets();

    if( ret_status != ErrCode::OK )
    {
      DBG_ERROR2( " open_ipv4_sockets FAILED" );
      goto exit_label_;
    }
  }

  DBG_INFO2( " Initializing FastShutdown Feature" );
  do_init_fast_shutdown_mechanism();

  DBG_INFO2( " Initializing Addr Change Notify Feature" );
  do_init_address_change_notify_mechanism();

  ret_status = do_post_start_server();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR2( " do_post_start_server FAILED" );
    goto exit_label_;
  }

  ret_status = start_listening();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR2( " start_listening FAILED" );
    goto exit_label_;
  }

  is_terminated_ = false;

  ret_status = start_receive_task();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR2( " start_receive_task FAILED" );
    goto exit_label_;
  }

  is_started_ = true;

exit_label_:
  DBG_INFO2( " EXIT status %d", int(ret_status) );

  return ( ret_status );
}

ErrCode IpAdapterBase::stop_server()
{
  DBG_INFO2( " ENTER" );
  is_terminated_ = true;
  is_started_    = false;

  ErrCode ret_status = do_pre_stop_server();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR2( " do_pre_stop_server FAILED" );
    goto exit_label_;
  }

  ret_status = do_post_stop_server();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR2( " do_post_stop_server FAILED" );
  }

exit_label_:

  DBG_INFO2( " EXIT status %d", int(ret_status) );
  return ( ret_status );
}


ErrCode IpAdapterBase::start_listening()
{
  if( is_started_ )
  {
    DBG_ERROR2( "Calling start_listening again" );
    return ( ErrCode::OK );
  }

  ErrCode ret_status = ErrCode::OK;

  DBG_INFO2( "ENTER" );

  /* get all the valid adapters address in the system */
  auto if_addr_array = get_interface_address_for_index( 0 );

  DBG_INFO2( "No of interfaces %d", static_cast<int>( if_addr_array.size() ) );

  update_interface_listening( if_addr_array );
  delete_items_and_clear_list( if_addr_array );

  DBG_INFO2( "EXIT status %d", int(ret_status) );

  return ( ret_status );
}

ErrCode IpAdapterBase::stop_listening()
{
  DBG_INFO2( "ENTER" );

  for( auto &socket : _sockets )
  {
    if( is_bit_set( socket->get_flags(), k_network_flag_multicast ) )
    {
      socket->CloseSocket();
    }
  }

  DBG_INFO2( "EXIT" );

  return ( ErrCode::OK );
}

int32_t IpAdapterBase::send_unicast_data( Endpoint &end_point, const uint8_t *data, const uint16_t data_length )
{
  return ( post_data_to_send_task( end_point, data, data_length, false ) );
}

int32_t IpAdapterBase::send_multicast_data( Endpoint &end_point, const uint8_t *data, const uint16_t data_length )
{
  return ( post_data_to_send_task( end_point, data, data_length, true ) );
}

/**********************************************************************************************************************/
/*************************                   PROTECTED FUNCTIONS                              *************************/
/**********************************************************************************************************************/

ErrCode IpAdapterBase::do_post_stop_server()
{
  return ( ErrCode::OK );
}

ErrCode IpAdapterBase::start_receive_task()
{
  DBG_INFO2( "ENTER" );

  if( ( _receive_task = OsalMgr::Inst()->AllocTask() ) == nullptr )
  {
    DBG_ERROR2( "FAILED to alloc receiver_task" );
    goto exit_label_;
  }

  if( _receive_task->Init( task_creation_params_t{ IP_ADAPTER_RECEIVER_TASK_NAME, IP_ADAPTER_RECEIVER_TASK_PRIORITY,
                                                   IP_ADAPTER_RECEIVER_TASK_STACK_SIZE, receive_data_routine, this } ) != OsalError::OK )
  {
    DBG_ERROR2( "FAILED to init receive_task" );
    goto exit_label_;
  }

  if( _receive_task->Start() != OsalError::OK )
  {
    DBG_ERROR2( "FAILED to start receive_task" );
    goto exit_label_;
  }

  return ( ErrCode::OK );

exit_label_:

  if( _receive_task != nullptr )
  {
    _receive_task->Destroy();
    OsalMgr::Inst()->FreeTask( _receive_task );
    _receive_task = nullptr;
  }

  DBG_INFO2( "EXIT" );

  return ( ErrCode::ERR );
}

/*
 * A socket is distinguished by two parameters.
 * 1. port
 * 2. local_interface_address (this is local network interface address)
 *
 * If the socket is binded with ANY address then it will receive data for the binded port from any network interface.
 *
 * So socket can be denoted as S<no>[port][local_interface_address]
 *
 * e.g
 *              S1[56775][0]
 *              S2[56776][192.168.0.100]
 *
 *
 * receiving the data from ipv4 and ipv6 including multicast
 * ---------------------------------------------------------
 * S1-5683 (IPV4 ucast)
 *      -> 0=>192.168.0.1
 *      -> 1=>192.168.0.2
 *      -> 2=>192.168.0.3
 *
 * S2-5683	(IPV6 ucast)
 *      -> 0=>fe80::01
 *      -> 1=>fe80::02
 *      -> 2=>fe80::03
 *
 * S3-5683 (IPV4 mcast)
 *      -> 0=>192.168.0.1:224.0.1.187
 *      -> 1=>192.168.0.2:224.0.1.187
 *      -> 2=>192.168.0.3:224.0.1.187
 *
 * S4-5683	(IPV6 mcast)
 *      -> 0=>fe80::01,ff02::158,ff03::158,ff04::158
 *      -> 1=>fe80::02,ff02::158,ff03::158,ff04::158
 *      -> 2=>fe80::03,ff02::158,ff03::158,ff04::158
 *
 * As seen in the code, the same socket is used for the ADD_MEMBERSHIP.
 * As checked in the fnet, lwip code it is required to use the same socket with same port
 * for the multicast group in each interface.
 * i.e
 * socket = 123
 *              -> port = 5683
 *              -> group 1
 *                      -> multicast addr = 224.0.1.287
 *                      -> interface = 1
 *              -> group 2
 *                      -> multicast addr = 224.0.1.287
 *                      -> interface = 2
 * */

void IpAdapterBase::start_ipv4_mcast_at_interface( const uint32_t if_index ) const
{
  IpAddress ipv4_multicast_addr{ 224, 0, 1, 187 };

  DBG_INFO2( "ENTER if_index[%d]", if_index );

  for( auto &socket : _sockets )
  {
    /* filter only IPV4 multicast socket or  IPV4 multicast secure socket */
    if( ( ( socket->get_flags() & k_network_flag_ipv4_mcast ) == k_network_flag_ipv4_mcast ) ||
      ( ( socket->get_flags() & k_network_flag_ipv4_secure_mcast ) == k_network_flag_ipv4_secure_mcast ) )
    {
      DBG_INFO2( "joining mcast, flag[0x%x]\n", socket->get_flags() );
      join_mcast_group( socket, ipv4_multicast_addr, if_index );
    }
  }

  DBG_INFO2( " EXIT" );
}

void IpAdapterBase::start_ipv6_mcast_at_interface( const uint32_t if_index ) const
{
  DBG_INFO2( " ENTER if_index[%d]", if_index );

  for( auto &socket : _sockets )
  {
    /* filter only IPV6 multicast socket or  IPV6 multicast secure socket */
    if( ( ( socket->get_flags() & k_network_flag_ipv6_mcast ) == k_network_flag_ipv6_mcast )
      || ( ( socket->get_flags() & k_network_flag_ipv6_secure_mcast ) == k_network_flag_ipv6_secure_mcast ) )
    {
      DBG_INFO2( "joining mcast, flag[0x%x]\n", socket->get_flags() );

      /* IPV6 LINK scope */
      IpAddress ipv6_address{ LINK, 0x158 };
      join_mcast_group( socket, ipv6_address, if_index );

      /* IPV6 REALM scope */
      ipv6_address.set_addr_by_scope( REALM, 0x158 );
      join_mcast_group( socket, ipv6_address, if_index );

      /* IPV6 SITE scope */
      ipv6_address.set_addr_by_scope( SITE, 0x158 );
      join_mcast_group( socket, ipv6_address, if_index );
    }
  }

  DBG_INFO2( " EXIT" );
}

IUdpSocket * IpAdapterBase::get_socket_by_mask( uint16_t socket_type_mask )
{
  for( auto &socket : _sockets )
  {
    if( socket->get_flags() == socket_type_mask )
    {
      return ( socket );
    }
  }

  return ( nullptr );
}

void IpAdapterBase::send_data_to_socket( IUdpSocket *pcz_udp_socket, Endpoint &endpoint, const data_buffer_t& data_buffer ) const
{
  const auto send_status = pcz_udp_socket->SendData( endpoint.get_addr(), endpoint.get_port(), data_buffer._pu8_data, data_buffer._u16_data_len );

  if( send_status != SocketError::OK && _event_handler)
  {
	AdapterEvent cz_adapter_event{ ADAPTER_EVENT_TYPE_ERROR, &endpoint, data_buffer._pu8_data, data_buffer._u16_data_len, k_adapter_type_ip };
	cz_adapter_event.set_error_code( ErrCode::SEND_DATA_FAILED );

	_event_handler->handle_event(cz_adapter_event);

//	if( _adapter_event_callback != nullptr )
//	{
//	  _adapter_event_callback( &cz_adapter_event, _adapter_event_cb_data );
//	}
  }
}
/**********************************************************************************************************************/
/*************************                   PRIVATE FUNCTIONS                                *************************/
/**********************************************************************************************************************/
ErrCode IpAdapterBase::terminate_internal()
{
  DBG_INFO2( "ENTER" );

  for( auto &socket : _sockets )
  {
    socket->CloseSocket();
  }

  do_un_init_address_change_notify_mechanism();

  DBG_INFO2( "EXIT" );

  return ( ErrCode::OK );
}


int32_t IpAdapterBase::post_data_to_send_task( Endpoint &end_point, const uint8_t *pu8_data, const uint16_t u16_data_length, const bool is_mcast )
{
  DBG_INFO2( "ENTER port[%d], msg_data[%p], data_length[%d], mcast[%d]", end_point.get_port(), pu8_data, u16_data_length, is_mcast );

  if( ( pu8_data == nullptr ) || ( u16_data_length == 0 ) )
  {
    DBG_ERROR2( "invalid msg_data or msg_data length 0" );
    return ( -1 );
  }

  const auto pcz_new_msg_to_send = create_new_send_msg( end_point, pu8_data, u16_data_length, is_mcast );

  if( pcz_new_msg_to_send == nullptr )
  {
    DBG_ERROR2( "FAILED to create new message" );
    return ( -1 );
  }

  if( sender_task_->SendMsg( pcz_new_msg_to_send ) != OsalError::OK )
  {
    DBG_ERROR2( "FAILED to send msg" );
    return ( -1 );
  }

  return ( (int32_t)u16_data_length );
}

ErrCode IpAdapterBase::open_socket( const IpAddrFamily ip_addr_family, const bool is_multicast, IUdpSocket *pcz_udp_socket, const uint16_t port )
{
  ErrCode ret_status = ErrCode::OK;
  IpAddress ip_addr{ ip_addr_family };

  DBG_INFO2( "ENTER family[%d], is_mcast[%d], pcz_udp_socket[%p] port[%d]", (int) ip_addr_family, is_multicast, pcz_udp_socket, port );

  if( pcz_udp_socket == nullptr )
  {
    DBG_ERROR2( "NULL socket" );
    ret_status = ErrCode::INVALID_PARAMS;
    goto exit_label_;
  }

  if( pcz_udp_socket->OpenSocket( ip_addr_family ) != SocketError::OK )
  {
    DBG_ERROR2( "Failed to open with family[%d]", (int) ip_addr_family );
    ret_status = ErrCode::INVALID_PARAMS;
    goto exit_label_;
  }

  if( pcz_udp_socket->EnablePacketInfo( true ) != SocketError::OK )
  {
    DBG_ERROR2( "EnablePacketInfo() FAILED" );
  }

  if( ip_addr_family == IpAddrFamily::IPv6 )
  {
    pcz_udp_socket->EnableIpv6Only( true );
    DBG_INFO2( "enable_ipv6_only()" );
  }

  if( is_multicast && ( port != 0 ) )
  {
    pcz_udp_socket->EnableReuseAddr( true );
    DBG_INFO2( "enable_reuse_addr()" );
  }

  if( pcz_udp_socket->BindSocket( ip_addr, port ) != SocketError::OK )
  {
    DBG_ERROR2( " BindSocket FAILED with port[%d]", port );
    pcz_udp_socket->CloseSocket();
    ret_status = ErrCode::INVALID_PARAMS;
  }

exit_label_:

  DBG_INFO2( " EXIT status %d", int(ret_status) );

  return ( ret_status );
}

ErrCode IpAdapterBase::open_socket_with_retry( const IpAddrFamily ip_addr_family, const bool is_multicast, IUdpSocket *pcz_udp_socket, uint16_t &ru16_port ) const
{
  DBG_INFO2( "ENTER family[%d], is_mcast[%d], pcz_udp_socket[%p] port[%d]", (int) ip_addr_family, is_multicast, pcz_udp_socket, ru16_port );
  auto ret_status = open_socket( ip_addr_family, is_multicast, pcz_udp_socket, ru16_port );

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR2( "Failed to open socket with port[%d], family[%d], mcast[%d]", ru16_port, int(ip_addr_family), is_multicast );
    DBG_ERROR2( "Retrying with port 0" );
    ret_status = open_socket( ip_addr_family, is_multicast, pcz_udp_socket, 0 );

    if( ret_status != ErrCode::OK )
    {
      DBG_ERROR2(
        "Failed to open socket after retrying with port[%d], family[%d], mcast[%d]", ru16_port, int(ip_addr_family), is_multicast );
      goto exit_label_;
    }
  }

  ru16_port = pcz_udp_socket->GetLocalPort();
  DBG_INFO2( "Opened socket with ****************** port[%d] ****************, family[%d], mcast[%d]", ru16_port, (int) ip_addr_family, is_multicast );

exit_label_:

  DBG_INFO2( "EXIT status %d", int(ret_status) );

  return ( ret_status );
}

ErrCode IpAdapterBase::open_ipv6_sockets()
{
  auto    ip_adapter_config = ConfigManager::Inst().get_ip_adapter_config();
  ErrCode ret_status        = ErrCode::OK;

  DBG_INFO2( "ENTER" );

  /* open ipv6 unicast socket */
  if( ip_adapter_config->is_ipv6_ucast_enabled() )
  {
    ret_status = add_socket( k_network_flag_ipv6, ip_adapter_config->get_port( IP_ADAPTER_CONFIG_IPV6_UCAST ) );
  }

  if( ip_adapter_config->is_ipv6_secure_ucast_enabled() )
  {
    ret_status = add_socket( k_network_flag_ipv6_secure_ucast, ip_adapter_config->get_port( IP_ADAPTER_CONFIG_IPV6_UCAST_SECURE ) );
  }

  if( ip_adapter_config->is_ipv6_mcast_enabled() )
  {
    ret_status = add_socket( k_network_flag_ipv6_mcast, ip_adapter_config->get_port( IP_ADAPTER_CONFIG_IPV6_MCAST ) );
  }

  if( ip_adapter_config->is_ipv6_secure_mcast_enabled() )
  {
    ret_status = add_socket( k_network_flag_ipv6_secure_mcast, ip_adapter_config->get_port( IP_ADAPTER_CONFIG_IPV6_MCAST_SECURE ) );
  }

  DBG_INFO2( "EXIT status %d", int(ret_status) );

  return ( ret_status );
}

ErrCode IpAdapterBase::open_ipv4_sockets()
{
  ErrCode ret_status = ErrCode::OK;

  DBG_INFO2( "ENTER" );

  auto ip_adapter_config = ConfigManager::Inst().get_ip_adapter_config();

  /* open ipv4 unicast socket */
  if( ip_adapter_config->is_ipv4_ucast_enabled() )
  {
    ret_status = add_socket( k_network_flag_ipv4, ip_adapter_config->get_port( IP_ADAPTER_CONFIG_IPV4_UCAST ) );
  }

  /* open ipv4 unicast secure socket */
  if( ip_adapter_config->is_ipv4_secure_ucast_enabled() )
  {
    ret_status = add_socket( k_network_flag_ipv4_secure_ucast, ip_adapter_config->get_port( IP_ADAPTER_CONFIG_IPV4_UCAST_SECURE ) );
  }

  /* open ipv4 multicast socket */
  if( ip_adapter_config->is_ipv4_mcast_enabled() )
  {
    ret_status = add_socket( k_network_flag_ipv4_mcast, ip_adapter_config->get_port( IP_ADAPTER_CONFIG_IPV4_MCAST ) );
  }

  /* open ipv4 multicast secure socket */
  if( ip_adapter_config->is_ipv4_secure_mcast_enabled() )
  {
    ret_status = add_socket( k_network_flag_ipv4_secure_mcast, ip_adapter_config->get_port( IP_ADAPTER_CONFIG_IPV4_MCAST_SECURE ) );
  }

  DBG_INFO2( "EXIT status %d", int(ret_status) );

  return ( ret_status );
}

ErrCode IpAdapterBase::add_socket( uint16_t socket_flags, uint16_t u16_port )
{
  auto         new_socket  = INetworkPlatformFactory::GetCurrFactory()->AllocSocket();
  ErrCode      ret_status  = ErrCode::OK;
  IpAddrFamily addr_family = is_bit_set( socket_flags, k_network_flag_ipv4 ) ? IpAddrFamily::IPv4 : IpAddrFamily::IPv6;

  if( new_socket == nullptr )
  {
    return ( ErrCode::OUT_OF_MEM );
  }

  if( is_bit_set( socket_flags, k_network_flag_multicast ) )
  {
    ret_status = open_socket( addr_family, true, new_socket, u16_port );
  }
  else
  {
    ret_status = open_socket_with_retry( addr_family, false, new_socket, u16_port );
  }

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR2( "Failed to create socket, type[0x%x] port[%d]", socket_flags, u16_port );
    INetworkPlatformFactory::GetCurrFactory()->free_socket( new_socket );
    return ( ret_status );
  }

  /* below block updates the port just opened above.
   * This case will come when the user didn't set the default ports for the unicast.
   * */
  if( !is_bit_set( socket_flags, k_network_flag_multicast ) )
  {
    auto ip_adapter_config = ConfigManager::Inst().get_ip_adapter_config();
    bool is_secure         = is_bit_set( socket_flags, k_network_flag_secure );

    if( addr_family == IpAddrFamily::IPv4 )
    {
      ip_adapter_config->set_port( ( is_secure ) ? IP_ADAPTER_CONFIG_IPV4_UCAST_SECURE : IP_ADAPTER_CONFIG_IPV4_UCAST, u16_port );
    }
    else
    {
      ip_adapter_config->set_port( ( is_secure ) ? IP_ADAPTER_CONFIG_IPV6_UCAST_SECURE : IP_ADAPTER_CONFIG_IPV6_UCAST, u16_port );
    }
  }

  new_socket->set_flags( socket_flags );

  _sockets.push_back( new_socket );

  return ( ret_status );
}

base::ErrCode IpAdapterBase::get_endpoints_list( std::vector<Endpoint *> &rcz_endpoint_list )
{
  if( _cz_end_points.size() == 0 )
  {
    ScopedMutex scoped_lock{ _access_mutex };

    refresh_end_point_list();
  }

  for( auto &ep : _cz_end_points )
  {
    rcz_endpoint_list.push_back( ep );
  }

  return ( ErrCode::OK );
}

void IpAdapterBase::handle_address_change_event()
{
  auto if_addr_array = get_interface_address_for_index( 0 );

  refresh_end_point_list( if_addr_array );
  update_interface_listening( if_addr_array );
  delete_items_and_clear_list( if_addr_array );
}

void IpAdapterBase::update_interface_listening( std::vector<InterfaceAddress *> &cz_interface_addr_list )
{
  const auto ip_adapter_config = ConfigManager::Inst().get_ip_adapter_config();

  for( auto &if_addr : cz_interface_addr_list )
  {
    auto interface_index = if_addr->get_index();

    if( ( if_addr->is_ipv4() ) && ( ip_adapter_config->is_ipv4_mcast_enabled() || ip_adapter_config->is_ipv4_secure_mcast_enabled() ) )
    {
      DBG_INFO2( "Starting ipv4 mcast at interface [%d]", interface_index );
      start_ipv4_mcast_at_interface( interface_index );
    }

    if( ( if_addr->is_ipv6() ) && ( ip_adapter_config->is_ipv6_mcast_enabled() || ip_adapter_config->is_ipv6_secure_mcast_enabled() ) )
    {
      DBG_INFO2( "Starting ipv6 mcast at interface [%d]", interface_index );
      start_ipv6_mcast_at_interface( interface_index );
    }
  }
}
void IpAdapterBase::refresh_end_point_list()
{
  auto cz_interface_addr_list = get_interface_address_for_index( 0 );

  refresh_end_point_list( cz_interface_addr_list );

  delete_items_and_clear_list( cz_interface_addr_list );
}

void IpAdapterBase::refresh_end_point_list( std::vector<InterfaceAddress *> &cz_interface_addr_list )
{
  ScopedMutex scoped_mutex{ _access_mutex };

  delete_items_and_clear_list( _cz_end_points );

  for( auto &socket : _sockets )
  {
    SocketHelper sk_helper{ socket };

    /* select only the non multicast socket */
    if( !sk_helper.is_multicast() )
    {
      for( auto &if_addr : cz_interface_addr_list )
      {
        if( ( sk_helper.is_ipv4() && if_addr->is_ipv4() )
          || ( sk_helper.is_ipv6() && if_addr->is_ipv6() ) )
        {
          auto ep = new Endpoint{ k_adapter_type_ip,
                                  socket->get_flags(),
                                  socket->GetLocalPort(),
                                  if_addr->get_index(),
                                  IpAddress{ (uint8_t *) if_addr->get_addr(), if_addr->get_family() } };
          _cz_end_points.push_back( ep );
        }
      }
    }
  }
}

ErrCode IpAdapterBase::start_sending_thread()
{
  DBG_INFO2( "ENTER" );

  if( ( sender_task_ = OsalMgr::Inst()->AllocTask() ) == nullptr )
  {
    DBG_ERROR2( " FAILED to alloc sender_task" );
    goto exit_label_;
  }

  if( sender_task_->Init( task_creation_params_t{ IP_ADAPTER_SENDER_TASK_NAME, IP_ADAPTER_SENDER_TASK_PRIORITY, IP_ADAPTER_SENDER_TASK_STACK_SIZE,
                                                  &ip_adapter_msg_q_, ip_adapter_sender_task_handle_msg_cb, this, ip_adapter_sender_task_delete_msg_cb, this } ) != OsalError::OK )
  {
    DBG_ERROR2( "FAILED to init sender_task" );
    goto exit_label_;
  }

  if( sender_task_->Start() != OsalError::OK )
  {
    DBG_ERROR2( " FAILED to start sender_task" );
    goto exit_label_;
  }

  return ( ErrCode::OK );
exit_label_:

  if( sender_task_ != nullptr )
  {
    sender_task_->Destroy();
    OsalMgr::Inst()->FreeTask( sender_task_ );
    sender_task_ = nullptr;
  }

  DBG_INFO2( "EXIT" );

  return ( ErrCode::ERR );
}

/***
 * Allocate new message and fill in the fields as passed.
 *
 * @param end_point - the end point for the destination
 * @param data - the payload for the message
 * @param data_length - the payload size.
 * @param is_multicast - is the message for unicast or multicast.
 * @return
 */
IpAdapterQMsg * IpAdapterBase::create_new_send_msg( const Endpoint &end_point, const uint8_t *data, uint16_t data_length, const bool is_multicast )
{
  ScopedMutex lock{sender_task_mutex_};

  /* allocate new message */
  auto pcz_new_ip_adapter_msg = ip_adapter_msg_q_list_.Alloc();

  if( pcz_new_ip_adapter_msg == nullptr )
  {
    return ( nullptr );
  }

  pcz_new_ip_adapter_msg->end_point_ = end_point;
  pcz_new_ip_adapter_msg->_data      = static_cast<uint8_t *>( mnew_g( data_length ) );

  if( pcz_new_ip_adapter_msg->_data == nullptr )
  {
    ip_adapter_msg_q_list_.Free( pcz_new_ip_adapter_msg );

    return ( nullptr );
  }

  memcpy( &pcz_new_ip_adapter_msg->_data[0], data, data_length );

  pcz_new_ip_adapter_msg->_dataLength  = data_length;
  pcz_new_ip_adapter_msg->is_multicast = is_multicast;

  return ( pcz_new_ip_adapter_msg );
}

/*
 * There are two ways to send message
 * 1. unicast message
 * 2. multicast message
 *
 * For unicast message it is straight forward and need destination address and
 * destination port.
 *
 * For multicast message it needs to be sent in all the interfaces, but the socket
 * used to send the message should be unicast socket and also before sending need to
 * select the interface to send the multicast message.
 * */

/***
 * Handles the new message arrived for the task.
 * @param pv_adapter_q_msg
 */
void IpAdapterBase::SEND_TASK_handle_msg( void *pv_adapter_q_msg )
{
  if( pv_adapter_q_msg != nullptr )
  {
	auto ip_adapter_q_msg = (IpAdapterQMsg *)pv_adapter_q_msg;

	do_handle_send_msg( ip_adapter_q_msg );

    auto       &endpoint        = ip_adapter_q_msg->end_point_;
    const auto pu8_data         = ip_adapter_q_msg->_data;
    const auto u16_data_len     = ip_adapter_q_msg->_dataLength;
    const auto network_flag     = endpoint.get_network_flags();
    const auto is_secure        = is_bit_set( network_flag, k_network_flag_secure );
    const auto is_ipv4_transfer = is_bit_set( network_flag, k_network_flag_ipv4 );
    const auto is_ipv6_transfer = is_bit_set( network_flag, k_network_flag_ipv6 );

    DBG_INFO2(
      "ENTER data_len[%d], mcast[%d], port[%d], adapter[%x], if_index[%d], nw_flag[%x]",
      u16_data_len,
      ip_adapter_q_msg->is_multicast,
      endpoint.get_port(),
      endpoint.get_adapter_type(),
      endpoint.get_if_index(),
      endpoint.get_network_flags() );

    if( ip_adapter_q_msg->is_multicast )
    {
      auto ipv4_socket = ( is_ipv4_transfer ) ? get_socket_by_mask( ( is_secure ) ? k_network_flag_ipv4_secure_ucast : k_network_flag_ipv4 ) : nullptr;
      auto ipv6_socket = ( is_ipv6_transfer ) ? get_socket_by_mask( ( is_secure ) ? k_network_flag_ipv6_secure_ucast : k_network_flag_ipv6 ) : nullptr;

      endpoint.set_port( is_secure ? COAP_SECURE_PORT : COAP_PORT );

      auto if_addr_array = get_interface_address_for_index( 0 );

      for( auto &if_addr : if_addr_array )
      {
        if( ( ipv4_socket != nullptr ) && if_addr->is_ipv4() )
        {
          ipv4_socket->SelectMulticastInterface( endpoint.get_addr(), if_addr->get_index() );
          ipv4_socket->SendData( endpoint.get_addr(), endpoint.get_port(), pu8_data, u16_data_len );
        }

        if( ( ipv6_socket != nullptr ) && if_addr->is_ipv6() )
        {
          ipv6_socket->SelectMulticastInterface( endpoint.get_addr(), if_addr->get_index() );
          ipv6_socket->SendData( endpoint.get_addr(), endpoint.get_port(), pu8_data, u16_data_len );
        }
      }
    }
    else     // it is unicast
    {
      clear_bit( (uint16_t) network_flag, k_network_flag_multicast );
      auto socket = get_socket_by_mask( network_flag );

      if( socket != nullptr )
      {
        send_data_to_socket( socket, endpoint, data_buffer_t{ pu8_data, u16_data_len });
      }
    }
  }
}

void IpAdapterBase::SEND_TASK_delete_msg( void *pv_adapter_q_msg )
{
  DBG_INFO2( " ENTER msg[%p]", pv_adapter_q_msg );

  IpAdapterQMsg *msg = static_cast<IpAdapterQMsg *>( pv_adapter_q_msg );

  /* give subclass an opportunity to delete any private msg_data */
  do_delete_msg( msg );

  if( msg->_data != nullptr )
  {
    delete[] msg->_data;
  }

  sender_task_mutex_->Lock();
  ip_adapter_msg_q_list_.Free( msg );
  sender_task_mutex_->Unlock();

  DBG_INFO2( " EXIT" );
}

void join_mcast_group( IUdpSocket *pcz_udp_socket, IpAddress &rcz_ip_multicast_addr, const uint32_t u32_if_index )
{
  DBG_INFO2( "ENTER pcz_udp_socket[%p], if_index[%d]", pcz_udp_socket, u32_if_index );

  if( pcz_udp_socket != nullptr )
  {
#ifdef _NETWORK_DEBUG_
    uint8_t ascii_addr[46] = { 0 };

    rcz_ip_multicast_addr.to_string( &ascii_addr[0], 46 );

    DBG_INFO2( "Joining multicast group for addr %s at if_index[%d]", &ascii_addr[0], u32_if_index );
#endif /* _NETWORK_DEBUG_ */

    pcz_udp_socket->EnableMulticastLoopback( false );
    pcz_udp_socket->LeaveMulticastGroup( rcz_ip_multicast_addr, u32_if_index );

    if( pcz_udp_socket->JoinMulticastGroup( rcz_ip_multicast_addr, u32_if_index ) != SocketError::OK )
    {
      DBG_ERROR2( "Join Multicast Group FAILED" );
    }
  }
  else
  {
    DBG_ERROR2( " udp_socket NULL" );
  }
}

static void ip_adapter_sender_task_handle_msg_cb( void *pv_task_arg, void *pv_user_data )
{
  static_cast<IpAdapterBase *>( pv_user_data )->SEND_TASK_handle_msg( pv_task_arg );
}

static void ip_adapter_sender_task_delete_msg_cb( void *pv_task_arg, void *pv_user_data )
{
  static_cast<IpAdapterBase *>( pv_user_data )->SEND_TASK_delete_msg( pv_task_arg );
}
}
}
