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

#define __FILE_NAME__ "IpAdapterBase"

using namespace ja_iot::osal;
using namespace ja_iot::base;

namespace ja_iot {
namespace network {
using namespace base;

static void ip_adapter_sender_task_handle_msg_cb( void *pv_task_arg, void *pv_user_data );
static void ip_adapter_sender_task_delete_msg_cb( void *pv_task_arg, void *pv_user_data );
static void join_mcast_group( IUdpSocket *udp_socket, IpAddress &ip_multicast_addr, const uint32_t if_index );

#ifdef _OS_WINDOWS_
static void interface_monitor_cb( const InterfaceEvent *pcz_interface_event, void *pv_user_data );
#endif
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

  auto ret_status = start_interface_monitor();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR2( "start_interface_monitor FAILED" );
    goto exit_label_;
  }

  /* start the sending thread */
  ret_status = start_sending_thread();

  if( ret_status != ErrCode::OK )
  {
    stop_interface_monitor();

    DBG_ERROR2( "create_and_start_sending_thread FAILED" );
    goto exit_label_;
  }

  ret_status = start_server();

  if( ret_status != ErrCode::OK )
  {
    stop_interface_monitor();

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
    DBG_ERROR2( " FAILED to stop the sender task" );
    return ( ErrCode::ERR );
  }

  auto ret_status = stop_interface_monitor();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR2( " StopInterfaceMonitor() FAILED " );
    ret_status = ErrCode::ERR;
    goto exit_label_;
  }

  ret_status = stop_server();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR2( " StopServer() FAILED " );
    ret_status = ErrCode::ERR;
  }

exit_label_:
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
    DBG_WARN( " start_server called again" );
    return ( ErrCode::OK );
  }

  DBG_INFO2( " ENTER" );

  auto ret_status = do_pre_start_server();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR2( " DoPrestart_server() FAILED" );
    return ( ret_status );
  }

  const auto ip_adapter_config = ConfigManager::Inst().get_ip_adapter_config();

  if( ip_adapter_config->is_ipv6_enabled() )
  {
    ret_status = open_ipv6_sockets();

    if( ret_status != ErrCode::OK )
    {
      DBG_ERROR2( " OpenIPV6Sockets() FAILED" );
      goto exit_label_;
    }
  }

  if( ip_adapter_config->is_ipv4_enabled() )
  {
    ret_status = open_ipv4_sockets();

    if( ret_status != ErrCode::OK )
    {
      DBG_ERROR2( " OpenIPV4Sockets() FAILED" );
      goto exit_label_;
    }
  }

  DBG_INFO2( " calling InitFastShutdownMechanism()" );
  init_fast_shutdown_mechanism();

  DBG_INFO2( " calling InitAddressChangeNotifyMechanism()" );
  init_address_change_notify_mechanism();

  ret_status = do_post_start_server();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR2( " DoPoststart_server() FAILED" );
    goto exit_label_;
  }

  ret_status = start_listening();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR2( " StartListening() FAILED" );
    goto exit_label_;
  }

  is_terminated_ = false;

  ret_status = start_receive_task();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR2( " CreateAndStartReceiveTask() FAILED" );
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
    DBG_ERROR2( " DoPreStopServer() FAILED" );
    goto exit_label_;
  }

  ret_status = do_post_stop_server();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR2( " DoPostStopServer() FAILED" );
  }

exit_label_:

  DBG_INFO2( " EXIT status %d", int(ret_status) );
  return ( ret_status );
}


ErrCode IpAdapterBase::start_listening()
{
  if( is_started_ )
  {
    DBG_ERROR2( "Calling StartListening again" );
    return ( ErrCode::OK );
  }

  ErrCode    ret_status        = ErrCode::OK;
  const auto ip_adapter_config = ConfigManager::Inst().get_ip_adapter_config();

  DBG_INFO2( "ENTER" );

#ifdef _OS_WINDOWS_
  auto interface_monitor = INetworkPlatformFactory::GetCurrFactory()->get_interface_monitor();

  if( interface_monitor == nullptr )
  {
    DBG_ERROR2( "interface_monitor NULL" );
    return ( ErrCode::ERR );
  }

  auto if_addr_array = interface_monitor->get_interface_addr_list();
#else
  auto if_addr_array = get_interface_address_for_index( 0 );
#endif

  DBG_INFO2( "No of interfaces %d", static_cast<int>( if_addr_array.size() ) );

  for( auto &if_addr : if_addr_array )
  {
    if( ( if_addr->is_ipv4() ) && ( ip_adapter_config->is_ipv4_mcast_enabled() || ip_adapter_config->is_ipv4_secure_mcast_enabled() ) )
    {
      DBG_INFO2( "Starting ipv4 mcast at interface [%d]", if_addr->get_index() );
      start_ipv4_mcast_at_interface( if_addr->get_index() );
    }

    if( ( if_addr->is_ipv6() ) && ( ip_adapter_config->is_ipv6_mcast_enabled() || ip_adapter_config->is_ipv6_secure_mcast_enabled() ) )
    {
      DBG_INFO2( "Starting ipv6 mcast at interface [%d]", if_addr->get_index() );
      start_ipv6_mcast_at_interface( if_addr->get_index() );
    }

    delete if_addr;
  }

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

void IpAdapterBase::do_un_init_address_change_notify_mechanism()
{
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
    if( ( (socket->get_flags() & k_network_flag_ipv4_mcast) == k_network_flag_ipv4_mcast ) ||
    		( (socket->get_flags() & k_network_flag_ipv4_secure_mcast) == k_network_flag_ipv4_secure_mcast ) )
    {
      DBG_INFO2( "joining mcast, flag[0x%x]\n", socket->get_flags() );
      join_mcast_group( socket, ipv4_multicast_addr, if_index );
    }
  }

  DBG_INFO2( " EXIT" );
}

void IpAdapterBase::start_ipv6_mcast_at_interface( const uint32_t if_index ) const
{
  IpAddress ipv6_address{ LINK, 0x158 };

  DBG_INFO2( " ENTER if_index[%d]", if_index );

  for( auto &socket : _sockets )
  {
    if( ( (socket->get_flags() & k_network_flag_ipv6_mcast) == k_network_flag_ipv6_mcast )
    		|| ( (socket->get_flags() & k_network_flag_ipv6_secure_mcast) == k_network_flag_ipv6_secure_mcast ) )
    {
      DBG_INFO2( "joining mcast, flag[0x%x]\n", socket->get_flags() );
      join_mcast_group( socket, ipv6_address, if_index );
      ipv6_address.set_addr_by_scope( REALM, 0x158 );
      join_mcast_group( socket, ipv6_address, if_index );
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

  return ( 1 );
}


void IpAdapterBase::init_fast_shutdown_mechanism()
{
  do_init_fast_shutdown_mechanism();
}

void IpAdapterBase::init_address_change_notify_mechanism()
{
  do_init_address_change_notify_mechanism();
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

ErrCode IpAdapterBase::open_socket2( const IpAddrFamily ip_addr_family, const bool is_multicast, IUdpSocket *pcz_udp_socket, uint16_t &ru16_port ) const
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

ErrCode IpAdapterBase::add_socket( uint16_t socket_type, uint16_t port )
{
  auto         new_socket  = INetworkPlatformFactory::GetCurrFactory()->AllocSocket();
  ErrCode      ret_status  = ErrCode::OK;
  IpAddrFamily addr_family = is_bit_set( socket_type, k_network_flag_ipv4 ) ? IpAddrFamily::IPv4 : IpAddrFamily::IPv6;

  if( new_socket == nullptr )
  {
    return ( ErrCode::OUT_OF_MEM );
  }

  if( is_bit_set( socket_type, k_network_flag_multicast ) )
  {
    ret_status = open_socket( addr_family, true, new_socket, port );
  }
  else
  {
    ret_status = open_socket2( addr_family, false, new_socket, port );
  }

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR2( "Failed to create socket, type[0x%x] port[%d]", socket_type, port );
    INetworkPlatformFactory::GetCurrFactory()->free_socket( new_socket );
    return ( ret_status );
  }

  /*
    below block updates the port just opened above. This case will come when the user didn't set the default ports for the 
unicast.
*/
  if( !is_bit_set( socket_type, k_network_flag_multicast ) )
  {
    auto ip_adapter_config = ConfigManager::Inst().get_ip_adapter_config();
    bool is_secure         = is_bit_set( socket_type, k_network_flag_secure );

    if( addr_family == IpAddrFamily::IPv4 )
    {
      ip_adapter_config->set_port( ( is_secure ) ? IP_ADAPTER_CONFIG_IPV4_UCAST_SECURE : IP_ADAPTER_CONFIG_IPV4_UCAST, port );
    }
    else
    {
      ip_adapter_config->set_port( ( is_secure ) ? IP_ADAPTER_CONFIG_IPV6_UCAST_SECURE : IP_ADAPTER_CONFIG_IPV6_UCAST, port );
    }
  }

  new_socket->set_flags( socket_type );

  _sockets.push_back( new_socket );

  return ( ret_status );
}

base::ErrCode IpAdapterBase::get_endpoints_list( std::deque<Endpoint *> &rcz_endpoint_list )
{
  const auto ip_adapter_config = ConfigManager::Inst().get_ip_adapter_config();

#ifdef _OS_WINDOWS_
  auto       interface_monitor = INetworkPlatformFactory::GetCurrFactory()->get_interface_monitor();

  if( interface_monitor == nullptr )
  {
    return ( ErrCode::OK );
  }

  auto interface_address_list = interface_monitor->get_interface_addr_list( true );
#else
  auto interface_address_list = get_interface_address_for_index( 0 );
#endif

  for( auto &if_addr : interface_address_list )
  {
    if( ( ( if_addr->is_ipv4() ) && !ip_adapter_config->is_ipv4_enabled() )
      || ( ( if_addr->is_ipv6() ) && !ip_adapter_config->is_ipv6_enabled() ) )
    {
      continue;
    }

    auto endpoint = new Endpoint{ k_adapter_type_ip, k_network_flag_ipv4, ip_adapter_config->get_port( IP_ADAPTER_CONFIG_IPV4_UCAST ),
                                  if_addr->get_index(), IpAddress{ reinterpret_cast<uint8_t *>( if_addr->get_addr() ), if_addr->get_family() } };

    if( if_addr->is_ipv6() )
    {
      endpoint->set_network_flags( k_network_flag_ipv6 );
      endpoint->set_port( ip_adapter_config->get_port( IP_ADAPTER_CONFIG_IPV6_UCAST ) );
    }

    rcz_endpoint_list.push_back( endpoint );

    delete if_addr;
  }

  return ( ErrCode::OK );
}

ErrCode IpAdapterBase::handle_interface_event( InterfaceEvent *pcz_interface_event )
{
  ErrCode ret_status = ErrCode::OK;

  DBG_INFO2( " ENTER interface_event[%p]", pcz_interface_event );

  if( pcz_interface_event != nullptr )
  {
    ret_status = do_handle_interface_event( pcz_interface_event );

    if( ret_status != ErrCode::OK )
    {
      DBG_ERROR2( " DoHandleInterfaceEvent FAILED" );
    }
  }
  else
  {
    DBG_WARN( " called with interface_event NULL" );
  }

  DBG_INFO2( " EXIT status %d", int(ret_status) );

  return ( ret_status );
}

ErrCode IpAdapterBase::start_interface_monitor()
{
  DBG_INFO2( " ENTER" );

#ifdef _OS_WINDOWS_
  auto interface_monitor = INetworkPlatformFactory::GetCurrFactory()->get_interface_monitor();

  if( interface_monitor != nullptr )
  {
    interface_monitor->add_interface_event_callback( interface_monitor_cb, this );
    interface_monitor->start_monitor( k_adapter_type_ip );
  }
  else
  {
    DBG_WARN( " interface_monitor NULL" );
  }

#else
  do_start_interface_monitor();
#endif

  DBG_INFO2( " EXIT" );

  return ( ErrCode::OK );
}

ErrCode IpAdapterBase::stop_interface_monitor()
{
  DBG_INFO2( " ENTER" );
#ifdef _OS_WINDOWS_
  auto interface_monitor = INetworkPlatformFactory::GetCurrFactory()->get_interface_monitor();

  if( interface_monitor != nullptr )
  {
    interface_monitor->remove_interface_event_callback( interface_monitor_cb );
    interface_monitor->stop_monitor( k_adapter_type_ip );
  }
  else
  {
    DBG_WARN( " interface_monitor NULL" );
  }

#else
  do_stop_interface_monitor();
#endif

  DBG_INFO2( " EXIT" );

  return ( ErrCode::OK );
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
  /* allocate new message */
  sender_task_mutex_->Lock();
  auto pcz_new_ip_adapter_msg = ip_adapter_msg_q_list_.Alloc();
  sender_task_mutex_->Unlock();

  if( pcz_new_ip_adapter_msg == nullptr )
  {
    return ( nullptr );
  }

  pcz_new_ip_adapter_msg->end_point_ = end_point;
  pcz_new_ip_adapter_msg->_data      = static_cast<uint8_t *>( mnew_g( data_length ) );

  if( pcz_new_ip_adapter_msg->_data == nullptr )
  {
    sender_task_mutex_->Lock();
    ip_adapter_msg_q_list_.Free( pcz_new_ip_adapter_msg );
    sender_task_mutex_->Unlock();

    return ( nullptr );
  }

  memcpy( &pcz_new_ip_adapter_msg->_data[0], data, data_length );

  pcz_new_ip_adapter_msg->_dataLength  = data_length;
  pcz_new_ip_adapter_msg->is_multicast = is_multicast;

  return ( pcz_new_ip_adapter_msg );
}

/***
 * Handles the new message arrived for the task.
 * @param pv_adapter_q_msg
 */
void IpAdapterBase::handle_msg( void *pv_adapter_q_msg )
{
  if( pv_adapter_q_msg != nullptr )
  {
    do_handle_send_msg( static_cast<IpAdapterQMsg *>( pv_adapter_q_msg ) );
  }
}

void IpAdapterBase::delete_msg( void *pv_adapter_q_msg )
{
  DBG_INFO2( " ENTER msg[%p]", pv_adapter_q_msg );

  IpAdapterQMsg *msg = static_cast<IpAdapterQMsg *>( pv_adapter_q_msg );

  /* give subclass an oppurtunity to delete any private msg_data */
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

    if( pcz_udp_socket->JoinMulticastGroup( rcz_ip_multicast_addr, u32_if_index ) != SocketError::OK )
    {
      pcz_udp_socket->LeaveMulticastGroup( rcz_ip_multicast_addr, u32_if_index );

      pcz_udp_socket->JoinMulticastGroup( rcz_ip_multicast_addr, u32_if_index );
    }
  }
  else
  {
    DBG_ERROR2( " udp_socket NULL" );
  }
}

#ifdef _OS_WINDOWS_
static void interface_monitor_cb( const InterfaceEvent *pcz_interface_event, void *pv_user_data )
{
  static_cast<IpAdapterBase *>( pv_user_data )->handle_interface_event( const_cast<InterfaceEvent *>( pcz_interface_event ) );
}
#endif

static void ip_adapter_sender_task_handle_msg_cb( void *pv_task_arg, void *pv_user_data )
{
  static_cast<IpAdapterBase *>( pv_user_data )->handle_msg( pv_task_arg );
}

static void ip_adapter_sender_task_delete_msg_cb( void *pv_task_arg, void *pv_user_data )
{
  static_cast<IpAdapterBase *>( pv_user_data )->delete_msg( pv_task_arg );
}
}
}
