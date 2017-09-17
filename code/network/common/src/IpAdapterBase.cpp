/*
 * IpAdapterBase.cpp
 *
 *  Created on: Jul 3, 2017
 *      Author: psammand
 */

#include <INetworkPlatformFactory.h>
#include "IpAdapterBase.h"
#include "OsalMgr.h"
#include <cstdint>
#include "konstants.h"
#include <cstdio>
#include <common/inc/logging_network.h>
#include <AdapterMgr.h>
#include <IpAdapterConfig.h>

using namespace ja_iot::osal;

namespace ja_iot {
namespace network {
IpAdapterBase::IpAdapterBase ()
{
}

IpAdapterBase::~IpAdapterBase ()
{
}

ErrCode IpAdapterBase::Initialize()
{
  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "IpAdapterBase::Initialize:%d# ENTER", __LINE__ );

  /* call platform specific initialize */
  ret_status = DoPreIntialize();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR( "IpAdapterBase::Initialize:%d# DoPreIntialize() FAILED", __LINE__ );
    goto exit_label_;
  }

  ret_status = InitInternal();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR( "IpAdapterBase::Initialize:%d# InitInternal() FAILED", __LINE__ );
    goto exit_label_;
  }

  sender_task_mutex_ = OsalMgr::Inst()->AllocMutex();

  if( sender_task_mutex_ == nullptr )
  {
    DBG_ERROR( "IpAdapterBase::Initialize:%d# SendMsgQ mutex creation FAILED", __LINE__ );
    ret_status = ErrCode::ERR; goto exit_label_;
  }

  /* call platform specific post initialize */
  ret_status = DoPostIntialize();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR( "IpAdapterBase::Initialize:%d# DoPostIntialize() FAILED", __LINE__ );
    goto exit_label_;
  }

exit_label_:
  DBG_INFO( "IpAdapterBase::Initialize:%d# EXIT %d", __LINE__, (int) ret_status );

  return ( ret_status );
}

ErrCode IpAdapterBase::Terminate()
{
  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "IpAdapterBase::Terminate:%d# ENTER", __LINE__ );

  ret_status = DoPreTerminate();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR( "IpAdapterBase::Terminate:%d# DoPreTerminate() FAILED", __LINE__ );
    goto exit_label_;
  }

  ret_status = TerminateInternal();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR( "IpAdapterBase::Terminate:%d# TerminateInternal() FAILED", __LINE__ );
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

  ret_status = DoPostTerminate();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR( "IpAdapterBase::Terminate:%d# DoPostTerminate() FAILED", __LINE__ );
    goto exit_label_;
  }

exit_label_:
  DBG_INFO( "IpAdapterBase::Terminate:%d# EXIT status %d", __LINE__, (int) ret_status );
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
ErrCode IpAdapterBase::StartAdapter()
{
  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "IpAdapterBase::StartAdapter:%d# ENTER", __LINE__ );

  ret_status = StartInterfaceMonitor();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR( "IpAdapterBase::StartAdapter:%d# StartInterfaceMonitor() FAILED", __LINE__ );
    goto exit_label_;
  }

  /* start the sending thread*/
  ret_status = CreateAndStartSendingThread();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR( "IpAdapterBase::StartAdapter:%d# CreateAndStartSendingThread() FAILED", __LINE__ );
    goto exit_label_;
  }

  ret_status = StartServer();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR( "IpAdapterBase::StartAdapter:%d# StartServer() FAILED", __LINE__ );
    goto exit_label_;
  }

exit_label_:
  DBG_INFO( "IpAdapterBase::StartAdapter:%d# EXIT status %d", __LINE__, (int) ret_status );

  return ( ErrCode::OK );
}

ErrCode IpAdapterBase::StopAdapter()
{
  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "IpAdapterBase::StopAdapter:%d# ENTER", __LINE__ );

  if( sender_task_->Stop() != OsalError::OK )
  {
    DBG_ERROR( "IpAdapterBase::StopAdapter:%d# FAILED to stop the sender task", __LINE__ );
    ret_status = ErrCode::ERR; goto exit_label_;
  }

  ret_status = StopInterfaceMonitor();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR( "IpAdapterBase::StopAdapter:%d# StopInterfaceMonitor() FAILED ", __LINE__ );
    ret_status = ErrCode::ERR; goto exit_label_;
  }

  ret_status = StopServer();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR( "IpAdapterBase::StopAdapter:%d# StopServer() FAILED ", __LINE__ );
    ret_status = ErrCode::ERR; goto exit_label_;
  }

exit_label_:
  DBG_INFO( "IpAdapterBase::StopAdapter:%d# EXIT status %d", __LINE__, (int) ret_status );

  return ( ret_status );
}

void IpAdapterBase::HandleMsg( IpAdapterQMsg *msg )
{
  if( msg != nullptr )
  {
    DoHandleSendMsg( msg );
  }
}

ErrCode IpAdapterBase::CreateAndStartSendingThread()
{
  ErrCode ret_status    = ErrCode::OK;
  auto    osalRetStatus = OsalError::OK;
  TaskMsgQParam taskMsgQParam{};

  DBG_INFO( "IpAdapterBase::CreateAndStartSendingThread:%d# ENTER", __LINE__ );

  sender_task_ = OsalMgr::Inst()->AllocTask();

  if( sender_task_ == nullptr )
  {
    DBG_ERROR( "IpAdapterBase::CreateAndStartSendingThread:%d# FAILED to alloc sender_task", __LINE__ );
    ret_status = ErrCode::OUT_OF_MEM; goto exit_label_;
  }

  taskMsgQParam.msgQ           = &ip_adapter_msg_q_;
  taskMsgQParam.taskMsgHandler = &sender_task_msg_q_handler_;

  osalRetStatus = sender_task_->InitWithMsgQ( (uint8_t *) "IpTask", 0, 0, &taskMsgQParam, nullptr );

  if( osalRetStatus != OsalError::OK )
  {
    sender_task_->Destroy();
    OsalMgr::Inst()->FreeTask( sender_task_ );
    sender_task_ = nullptr;
    DBG_ERROR( "IpAdapterBase::CreateAndStartSendingThread:%d# FAILED to init sender_task", __LINE__ );
    ret_status = ErrCode::ERR; goto exit_label_;
  }

  osalRetStatus = sender_task_->Start();

  if( osalRetStatus != OsalError::OK )
  {
    sender_task_->Destroy();
    OsalMgr::Inst()->FreeTask( sender_task_ );
    sender_task_ = nullptr;
    DBG_ERROR( "IpAdapterBase::CreateAndStartSendingThread:%d# FAILED to start sender_task", __LINE__ );
    ret_status = ErrCode::ERR; goto exit_label_;
  }

exit_label_:
  DBG_INFO( "IpAdapterBase::CreateAndStartSendingThread:%d# EXIT status %d", __LINE__, (int) ret_status );

  return ( ret_status );
}

IpAdapterQMsg * IpAdapterBase::CreateNewSendMsg( const Endpoint &end_point, const uint8_t *data, uint16_t data_length, bool is_multicast )
{
  sender_task_mutex_->Lock();
  auto msg_q_entry = ip_adapter_msg_q_list_.Alloc();
  sender_task_mutex_->Unlock();

  if( msg_q_entry == nullptr )
  {
    return ( nullptr );
  }

  msg_q_entry->end_point_ = end_point;
  msg_q_entry->_data      = new uint8_t[data_length] {};

  if( msg_q_entry->_data == nullptr )
  {
    sender_task_mutex_->Lock();
    ip_adapter_msg_q_list_.Free( msg_q_entry );
    sender_task_mutex_->Unlock();

    return ( nullptr );
  }

  memcpy( &msg_q_entry->_data[0], data, data_length );

  msg_q_entry->_dataLength  = data_length;
  msg_q_entry->is_multicast = false;

  return ( msg_q_entry );
}

ErrCode IpAdapterBase::CreateAndStartReceiveTask()
{
  ErrCode ret_status      = ErrCode::OK;
  auto    osal_ret_status = OsalError::OK;

  DBG_INFO( "IpAdapterBase::CreateAndStartReceiveTask:%d# ENTER", __LINE__ );

  receiver_task_ = OsalMgr::Inst()->AllocTask();

  if( receiver_task_ == nullptr )
  {
    DBG_ERROR( "IpAdapterBase::CreateAndStartReceiveTask:%d# FAILED to alloc receiver_task", __LINE__ );
    ret_status = ErrCode::OUT_OF_MEM; goto exit_label_;
  }

  osal_ret_status = receiver_task_->Init( (uint8_t *) "IpTask", 0, 0, &receive_task_routine_, this );

  if( osal_ret_status != OsalError::OK )
  {
    receiver_task_->Destroy();
    OsalMgr::Inst()->FreeTask( receiver_task_ );
    receiver_task_ = nullptr;

    DBG_ERROR( "IpAdapterBase::CreateAndStartReceiveTask:%d# FAILED to init receiver_task", __LINE__ );
    ret_status = ErrCode::ERR; goto exit_label_;
  }

  osal_ret_status = receiver_task_->Start();

  if( osal_ret_status != OsalError::OK )
  {
    receiver_task_->Destroy();
    OsalMgr::Inst()->FreeTask( receiver_task_ );
    receiver_task_ = nullptr;

    DBG_ERROR( "IpAdapterBase::CreateAndStartReceiveTask:%d# FAILED to start receiver_task", __LINE__ );
    ret_status = ErrCode::ERR; goto exit_label_;
  }

exit_label_:
  DBG_INFO( "IpAdapterBase::CreateAndStartReceiveTask:%d# EXIT status %d", __LINE__, (int) ret_status );

  return ( ret_status );
}

IUdpSocket * IpAdapterBase::get_socket( IpAddrFamily ip_addr_family, bool is_multicast, bool is_secure )
{
  if( ip_addr_family == IpAddrFamily::IPV4 )
  {
    if( is_multicast )
    {
      return ( ( is_secure ) ? ipv4_multicast_secure_socket_ : ipv4_multicast_socket_ );
    }
    else
    {
      return ( ( is_secure ) ? ipv4_unicast_secure_socket_ : ipv4_unicast_socket_ );
    }
  }
  else
  {
    if( is_multicast )
    {
      return ( ( is_secure ) ? ipv6_multicast_secure_socket_ : ipv6_multicast_socket_ );
    }
    else
    {
      return ( ( is_secure ) ? ipv6_unicast_secure_socket_ : ipv6_unicast_socket_ );
    }
  }
}

/*
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

void IpAdapterBase::start_ipv4_mcast_at_interface( uint32_t if_index )
{
  IpAddress ipv4_multicast_addr{ 224, 0, 1, 187 };

  DBG_INFO( "IpAdapterBase::start_ipv4_mcast_at_interface:%d# ENTER if_index[%d]", __LINE__, if_index );

  join_mcast_group( ipv4_multicast_socket_, ipv4_multicast_addr, if_index );
  join_mcast_group( ipv4_multicast_secure_socket_, ipv4_multicast_addr, if_index );

  DBG_INFO( "IpAdapterBase::start_ipv4_mcast_at_interface:%d# EXIT", __LINE__ );
}

void IpAdapterBase::start_ipv6_mcast_at_interface( uint32_t if_index )
{
  IpAddress ipv6_address{ Ipv6AddrScope::LINK, 0x158 };

  DBG_INFO( "IpAdapterBase::start_ipv6_mcast_at_interface:%d# ENTER if_index[%d]", __LINE__, if_index );

  join_mcast_group( ipv6_multicast_socket_, ipv6_address, if_index );
  join_mcast_group( ipv6_multicast_secure_socket_, ipv6_address, if_index );

  ipv6_address.set_addr_by_scope( Ipv6AddrScope::REALM, 0x158 );
  join_mcast_group( ipv6_multicast_socket_, ipv6_address, if_index );
  join_mcast_group( ipv6_multicast_secure_socket_, ipv6_address, if_index );

  ipv6_address.set_addr_by_scope( Ipv6AddrScope::SITE, 0x158 );
  join_mcast_group( ipv6_multicast_socket_, ipv6_address, if_index );
  join_mcast_group( ipv6_multicast_secure_socket_, ipv6_address, if_index );

  DBG_INFO( "IpAdapterBase::start_ipv4_mcast_at_interface:%d# EXIT", __LINE__ );
}

void IpAdapterBase::join_mcast_group( IUdpSocket *udp_socket, IpAddress &ip_multicast_addr, uint32_t if_index )
{
  DBG_INFO( "IpAdapterBase::join_mcast_group:%d# ENTER udp_socket[%p], if_index[%d]", __LINE__, udp_socket, if_index );

  if( udp_socket != nullptr )
  {
#ifdef _NETWORK_DEBUG_
    uint8_t ascii_addr[46];

    ip_multicast_addr.to_string( &ascii_addr[0], 46 );

    DBG_INFO( "IpAdapterBase::join_mcast_group:%d# Joinig multicast group for addr %s at if_index[%d]", __LINE__, &ascii_addr[0], if_index );
#endif /* _NETWORK_DEBUG_ */

    if( udp_socket->JoinMulticastGroup( ip_multicast_addr, if_index ) != SocketError::OK )
    {
      udp_socket->LeaveMulticastGroup( ip_multicast_addr, if_index );

      udp_socket->JoinMulticastGroup( ip_multicast_addr, if_index );
    }
  }
  else
  {
    DBG_ERROR( "IpAdapterBase::join_mcast_group:%d# udp_socket NULL", __LINE__ );
  }
}

void IpAdapterBase::DeleteMsg( IpAdapterQMsg *msg )
{
  DBG_INFO( "IpAdapterBase::DeleteMsg:%d# ENTER msg[%p]", __LINE__, msg );

  DoDeleteMsg( msg );

  if( msg->_data != nullptr )
  {
    delete[] msg->_data;
  }

  sender_task_mutex_->Lock();
  ip_adapter_msg_q_list_.Free( msg );
  sender_task_mutex_->Unlock();

  DBG_INFO( "IpAdapterBase::DeleteMsg:%d# EXIT", __LINE__ );
}

ErrCode IpAdapterBase::InitInternal()
{
  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "IpAdapterBase::InitInternal:%d# ENTER", __LINE__ );

  ret_status = CreateSockets();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR( "IpAdapterBase::InitInternal:%d# CreateSockets() FAILED", __LINE__ );
  }

  DBG_INFO( "IpAdapterBase::InitInternal:%d# EXIT status %d", __LINE__, (int) ret_status );

  return ( ret_status );
}

ErrCode IpAdapterBase::TerminateInternal()
{
  DBG_INFO( "IpAdapterBase::TerminateInternal:%d# ENTER", __LINE__ );

  ipv4_unicast_socket_->CloseSocket();
  ipv4_unicast_secure_socket_->CloseSocket();

  ipv4_multicast_socket_->CloseSocket();
  ipv4_multicast_secure_socket_->CloseSocket();

  ipv6_unicast_socket_->CloseSocket();
  ipv6_unicast_secure_socket_->CloseSocket();

  ipv6_multicast_socket_->CloseSocket();
  ipv6_multicast_secure_socket_->CloseSocket();

  DoUnInitAddressChangeNotifyMechanism();

  DBG_INFO( "IpAdapterBase::TerminateInternal:%d# EXIT", __LINE__ );

  return ( ErrCode::OK );
}

int32_t IpAdapterBase::SendUnicastData( Endpoint &end_point, const uint8_t *data, uint16_t data_length )
{
  return ( post_data_to_send_task( end_point, data, data_length, false ) );
}

int32_t IpAdapterBase::SendMulticastData( Endpoint &end_point, const uint8_t *data, uint16_t data_length )
{
  return ( post_data_to_send_task( end_point, data, data_length, true ) );
}

int32_t IpAdapterBase::post_data_to_send_task( Endpoint &end_point, const uint8_t *data, uint16_t data_length, bool is_mcast )
{
  int32_t        sent_bytes      = -1;
  IpAdapterQMsg *new_msg_to_send = nullptr;

  DBG_INFO( "IpAdapterBase::SendUnicastData:%d# ENTER port[%d], data[%p], data_length[%d], mcast[%d]", __LINE__, end_point.getPort(), data, data_length, is_mcast );

  if( ( data == nullptr ) || ( data_length == 0 ) )
  {
    DBG_ERROR( "IpAdapterBase::SendUnicastData:%d# invalid data or data length 0", __LINE__ );
    sent_bytes = -1; goto exit_label_;
  }

  new_msg_to_send = CreateNewSendMsg( end_point, data, data_length, is_mcast );

  if( new_msg_to_send == nullptr )
  {
    DBG_ERROR( "IpAdapterBase::SendUnicastData:%d# FAILED to create new message", __LINE__ );
    sent_bytes = -1; goto exit_label_;
  }

  if( sender_task_->SendMsg( new_msg_to_send ) != OsalError::OK )
  {
    DBG_ERROR( "IpAdapterBase::SendUnicastData:%d# FAILED to send msg", __LINE__ );
    sent_bytes = -1; goto exit_label_;
  }

exit_label_:

  DBG_INFO( "IpAdapterBase::SendUnicastData:%d# EXIT bytes_send %d", __LINE__, sent_bytes );

  return ( sent_bytes );
}

ErrCode IpAdapterBase::StartServer()
{
  if( is_started_ == true )
  {
    DBG_WARN( "IpAdapterBase::StartServer:%d# StartServer called again", __LINE__ );
    return ( ErrCode::OK );
  }

  DBG_INFO( "IpAdapterBase::StartServer:%d# ENTER", __LINE__ );

  ErrCode          ret_status        = ErrCode::OK;
  IpAdapterConfig *ip_adapter_config = nullptr;

  ret_status = DoPreStartServer();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR( "IpAdapterBase::StartServer:%d# DoPreStartServer() FAILED", __LINE__ );
    goto exit_label_;
  }

  ip_adapter_config = AdapterManager::Inst().get_ip_adapter_config();

  if( ip_adapter_config->is_ipv6_enabled() )
  {
    ret_status = OpenIPV6Sockets( true, true, true );

    if( ret_status != ErrCode::OK )
    {
      DBG_ERROR( "IpAdapterBase::StartServer:%d# OpenIPV6Sockets() FAILED", __LINE__ );
      goto exit_label_;
    }
  }

  if( ip_adapter_config->is_ipv4_enabled() )
  {
    ret_status = OpenIPV4Sockets( true, true, true );

    if( ret_status != ErrCode::OK )
    {
      DBG_ERROR( "IpAdapterBase::StartServer:%d# OpenIPV4Sockets() FAILED", __LINE__ );
      goto exit_label_;
    }
  }

  DBG_INFO( "IpAdapterBase::StartServer:%d# calling InitFastShutdownMechanism()", __LINE__ );
  InitFastShutdownMechanism();

  DBG_INFO( "IpAdapterBase::StartServer:%d# calling InitAddressChangeNotifyMechanism()", __LINE__ );
  InitAddressChangeNotifyMechanism();

  ret_status = DoPostStartServer();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR( "IpAdapterBase::StartServer:%d# DoPostStartServer() FAILED", __LINE__ );
    goto exit_label_;
  }

  ret_status = StartListening();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR( "IpAdapterBase::StartServer:%d# StartListening() FAILED", __LINE__ );
    goto exit_label_;
  }

  is_terminated_ = false;

  ret_status = IpAdapterBase::CreateAndStartReceiveTask();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR( "IpAdapterBase::StartServer:%d# CreateAndStartReceiveTask() FAILED", __LINE__ );
    goto exit_label_;
  }

  is_started_ = true;

exit_label_:
  DBG_INFO( "IpAdapterBase::StartServer:%d# EXIT status %d", __LINE__, (int) ret_status );

  return ( ret_status );
}

ErrCode IpAdapterBase::StopServer()
{
  DBG_INFO( "IpAdapterBase::StopServer:%d# ENTER", __LINE__ );
  is_terminated_ = true;
  is_started_    = false;

  ErrCode ret_status = ErrCode::OK;

  ret_status = DoPreStopServer();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR( "IpAdapterBase::StopServer:%d# DoPreStopServer() FAILED", __LINE__ );
    goto exit_label_;
  }

  ret_status = DoPostStopServer();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR( "IpAdapterBase::StopServer:%d# DoPostStopServer() FAILED", __LINE__ );
    goto exit_label_;
  }

exit_label_:

  DBG_INFO( "IpAdapterBase::StopServer:%d# EXIT status %d", __LINE__, (int) ret_status );
  return ( ret_status );
}

ErrCode IpAdapterBase::StartListening()
{
  if( is_started_ )
  {
    DBG_ERROR( "IpAdapterBase::StartListening:%d# Calling StartListening again", __LINE__ );
    return ( ErrCode::OK );
  }

  ErrCode            ret_status        = ErrCode::OK;
  IInterfaceMonitor *interface_monitor = INetworkPlatformFactory::GetCurrFactory()->GetInterfaceMonitor();
  IpAdapterConfig *  ip_adapter_config = AdapterManager::Inst().get_ip_adapter_config();
  ja_iot::base::StaticPtrArray<InterfaceAddress *, 10> if_addr_array{};

  DBG_INFO( "IpAdapterBase::StartListening:%d# ENTER", __LINE__ );

  if( interface_monitor == nullptr )
  {
    DBG_ERROR( "IpAdapterBase::StartListening:%d# interface_monitor NULL", __LINE__ );
    ret_status = ErrCode::ERR; goto exit_label_;
  }

  interface_monitor->GetInterfaceAddrList( if_addr_array );

  DBG_INFO( "IpAdapterBase::StartListening:%d# No of interfaces %d", __LINE__, if_addr_array.Count() );

  for( int i = 0; i < if_addr_array.Count(); i++ )
  {
    auto if_addr = if_addr_array.GetItem( i );

    if( if_addr != nullptr )
    {
      if( ( if_addr->getFamily() == IpAddrFamily::IPV4 ) && ip_adapter_config->is_config_flag_set( IpAdapterConfigFlag::IPV4_MCAST_ENABLED ) )
      {
        DBG_INFO( "IpAdapterBase::StartListening:%d# Starting ipv4 mcast at interface [%d]", __LINE__, if_addr->getIndex() );
        start_ipv4_mcast_at_interface( if_addr->getIndex() );
      }

      if( ( if_addr->getFamily() == IpAddrFamily::IPv6 ) && ip_adapter_config->is_config_flag_set( IpAdapterConfigFlag::IPV6_MCAST_ENABLED ) )
      {
        DBG_INFO( "IpAdapterBase::StartListening:%d# Starting ipv6 mcast at interface [%d]", __LINE__, if_addr->getIndex() );
        start_ipv6_mcast_at_interface( if_addr->getIndex() );
      }

      delete if_addr;
    }
  }

exit_label_:
  DBG_INFO( "IpAdapterBase::StartListening:%d# EXIT status %d", __LINE__, (int) ret_status );

  return ( ret_status );
}

ErrCode IpAdapterBase::StopListening()
{
  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "IpAdapterBase::StopListening:%d# ENTER", __LINE__ );
  auto    interface_monitor = INetworkPlatformFactory::GetCurrFactory()->GetInterfaceMonitor();
  ja_iot::base::StaticPtrArray<InterfaceAddress *, 10> if_addr_array{};

  if( interface_monitor == nullptr )
  {
    DBG_ERROR( "IpAdapterBase::StopListening:%d# interface_monitor NULL", __LINE__ );
    ret_status = ErrCode::ERR; goto exit_label_;
  }

  interface_monitor->GetInterfaceAddrList( if_addr_array );

  DBG_INFO( "IpAdapterBase::StopListening:%d# No of interfaces [%d]", __LINE__, if_addr_array.Count() );

  for( int i = 0; i < if_addr_array.Count(); i++ )
  {
    auto if_addr = if_addr_array.GetItem( i );

    if( if_addr != nullptr )
    {
      if( if_addr->getFamily() == IpAddrFamily::IPV4 )
      {
        DBG_INFO( "IpAdapterBase::StopListening:%d# Closing IPV4 mcast sockets", __LINE__ );
        ipv4_multicast_socket_->CloseSocket();
        ipv4_multicast_secure_socket_->CloseSocket();
      }

      if( if_addr->getFamily() == IpAddrFamily::IPv6 )
      {
        DBG_INFO( "IpAdapterBase::StopListening:%d# Closing IPV6 mcast sockets", __LINE__ );
        ipv6_multicast_socket_->CloseSocket();
        ipv6_multicast_secure_socket_->CloseSocket();
      }

      delete if_addr;
    }
  }

exit_label_:
  DBG_INFO( "IpAdapterBase::StopListening:%d# EXIT status %d", __LINE__, (int) ret_status );

  return ( ret_status );
}

void IpAdapterBase::InitFastShutdownMechanism()
{
  DoInitFastShutdownMechanism();
}

void IpAdapterBase::InitAddressChangeNotifyMechanism()
{
  DoInitAddressChangeNotifyMechanism();
}

ErrCode IpAdapterBase::CreateSockets()
{
  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "IpAdapterBase::CreateSockets:%d# ENTER", __LINE__ );

  auto ip_adapter_config = AdapterManager::Inst().get_ip_adapter_config();

  /* create IPV4 sockets only if it enabled*/
  if( ip_adapter_config->is_config_flag_set( IpAdapterConfigFlag::IPV4_UCAST_ENABLED ) )
  {
    DBG_INFO( "IpAdapterBase::CreateSockets:%d# Alloc IPV4 ucast socket", __LINE__ );
    ipv4_unicast_socket_ = INetworkPlatformFactory::GetCurrFactory()->AllocSocket();

    if( ipv4_unicast_socket_ == nullptr )
    {
      DBG_ERROR( "IpAdapterBase::CreateSockets:%d# Failed to alloc socket for ipv4_unicast_socket_", __LINE__ );
      ret_status = ErrCode::ERR; goto exit_label_;
    }
  }

  if( ip_adapter_config->is_config_flag_set( IpAdapterConfigFlag::IPV4_UCAST_SECURE_ENABLED ) )
  {
    DBG_INFO( "IpAdapterBase::CreateSockets:%d# Alloc IPV4 ucast secure socket", __LINE__ );
    ipv4_unicast_secure_socket_ = INetworkPlatformFactory::GetCurrFactory()->AllocSocket();

    if( ipv4_unicast_secure_socket_ == nullptr )
    {
      DBG_ERROR( "IpAdapterBase::CreateSockets:%d# Failed to alloc socket for ipv4_unicast_secure_socket_", __LINE__ );
      ret_status = ErrCode::ERR; goto exit_label_;
    }
  }

  if( ip_adapter_config->is_config_flag_set( IpAdapterConfigFlag::IPV4_MCAST_ENABLED ) )
  {
    DBG_INFO( "IpAdapterBase::CreateSockets:%d# Alloc IPV4 mcast socket", __LINE__ );
    ipv4_multicast_socket_ = INetworkPlatformFactory::GetCurrFactory()->AllocSocket();

    if( ipv4_multicast_socket_ == nullptr )
    {
      DBG_ERROR( "IpAdapterBase::CreateSockets:%d# Failed to alloc socket for ipv4_multicast_socket_", __LINE__ );
      ret_status = ErrCode::ERR; goto exit_label_;
    }
  }

  if( ip_adapter_config->is_config_flag_set( IpAdapterConfigFlag::IPV4_MCAST_SECURE_ENABLED ) )
  {
    DBG_INFO( "IpAdapterBase::CreateSockets:%d# Alloc IPV4 mcast secure socket", __LINE__ );
    ipv4_multicast_secure_socket_ = INetworkPlatformFactory::GetCurrFactory()->AllocSocket();

    if( ipv4_multicast_secure_socket_ == nullptr )
    {
      DBG_ERROR( "IpAdapterBase::CreateSockets:%d# Failed to alloc socket for ipv4_multicast_secure_socket_", __LINE__ );
      ret_status = ErrCode::ERR; goto exit_label_;
    }
  }

  /* create IPV6 sockets only if it enabled*/
  if( ip_adapter_config->is_config_flag_set( IpAdapterConfigFlag::IPV6_UCAST_ENABLED ) )
  {
    DBG_INFO( "IpAdapterBase::CreateSockets:%d# Alloc IPV6 ucast socket", __LINE__ );
    ipv6_unicast_socket_ = INetworkPlatformFactory::GetCurrFactory()->AllocSocket();

    if( ipv6_unicast_socket_ == nullptr )
    {
      DBG_ERROR( "IpAdapterBase::CreateSockets:%d# Failed to alloc socket for ipv6_unicast_socket_", __LINE__ );
      ret_status = ErrCode::ERR; goto exit_label_;
    }
  }

  if( ip_adapter_config->is_config_flag_set( IpAdapterConfigFlag::IPV6_UCAST_SECURE_ENABLED ) )
  {
    DBG_INFO( "IpAdapterBase::CreateSockets:%d# Alloc IPV6 ucast secure socket", __LINE__ );
    ipv6_unicast_secure_socket_ = INetworkPlatformFactory::GetCurrFactory()->AllocSocket();

    if( ipv6_unicast_secure_socket_ == nullptr )
    {
      DBG_ERROR( "IpAdapterBase::CreateSockets:%d# Failed to alloc socket for ipv6_unicast_secure_socket_", __LINE__ );
      ret_status = ErrCode::ERR; goto exit_label_;
    }
  }

  if( ip_adapter_config->is_config_flag_set( IpAdapterConfigFlag::IPV6_MCAST_ENABLED ) )
  {
    DBG_INFO( "IpAdapterBase::CreateSockets:%d# Alloc IPV6 mcast socket", __LINE__ );
    ipv6_multicast_socket_ = INetworkPlatformFactory::GetCurrFactory()->AllocSocket();

    if( ipv6_multicast_socket_ == nullptr )
    {
      DBG_ERROR( "IpAdapterBase::CreateSockets:%d# Failed to alloc socket for ipv6_multicast_socket_", __LINE__ );
      ret_status = ErrCode::ERR; goto exit_label_;
    }
  }

  if( ip_adapter_config->is_config_flag_set( IpAdapterConfigFlag::IPV6_MCAST_SECURE_ENABLED ) )
  {
    DBG_INFO( "IpAdapterBase::CreateSockets:%d# Alloc IPV6 mcast secure socket", __LINE__ );
    ipv6_multicast_secure_socket_ = INetworkPlatformFactory::GetCurrFactory()->AllocSocket();

    if( ipv6_multicast_secure_socket_ == nullptr )
    {
      DBG_ERROR( "IpAdapterBase::CreateSockets:%d# Failed to alloc socket for ipv6_multicast_secure_socket_", __LINE__ );
      ret_status = ErrCode::ERR; goto exit_label_;
    }
  }

exit_label_:
  DBG_INFO( "IpAdapterBase::CreateSockets:%d# EXIT status %d", __LINE__, (int) ret_status );

  return ( ret_status );
}

ErrCode IpAdapterBase::OpenSocket( IpAddrFamily ip_addr_family, bool is_multicast, IUdpSocket *udp_socket, uint16_t port )
{
  ErrCode ret_status = ErrCode::OK;
  IpAddress ip_addr{ ip_addr_family };

  DBG_INFO( "IpAdapterBase::OpenSocket:%d# ENTER family[%d], is_mcast[%d], udp_socket[%p] port[%d]", __LINE__, (int) ip_addr_family, is_multicast, udp_socket, port );

  if( udp_socket == nullptr )
  {
    DBG_ERROR( "IpAdapterBase::OpenSocket:%d# Passed socket NULL", __LINE__ );
    ret_status = ErrCode::INVALID_PARAMS; goto exit_label_;
  }

  DBG_INFO( "IpAdapterBase::OpenSocket:%d# opening socket", __LINE__ );

  if( udp_socket->OpenSocket( ip_addr_family ) != SocketError::OK )
  {
    DBG_ERROR( "IpAdapterBase::OpenSocket:%d# Failed to open with family[%d]", __LINE__, (int) ip_addr_family );
    ret_status = ErrCode::INVALID_PARAMS; goto exit_label_;
  }

  DBG_INFO( "IpAdapterBase::OpenSocket:%d# enable_packet_info", __LINE__ );

  if( udp_socket->EnablePacketInfo( true ) != SocketError::OK )
  {
    DBG_ERROR( "IpAdapterBase::OpenSocket:%d# EnablePacketInfo() FAILED", __LINE__ );
  }

  if( ip_addr_family == IpAddrFamily::IPv6 )
  {
    udp_socket->EnableIpv6Only( true );
    DBG_INFO( "IpAdapterBase::OpenSocket:%d# enable_ipv6_only()", __LINE__ );
  }

  if( is_multicast && ( port != 0 ) )
  {
    udp_socket->EnableReuseAddr( true );
    DBG_INFO( "IpAdapterBase::OpenSocket:%d# enable_reuse_addr()", __LINE__ );
  }

  DBG_INFO( "IpAdapterBase::OpenSocket:%d# binding socket", __LINE__ );

  if( udp_socket->BindSocket( ip_addr, port ) != SocketError::OK )
  {
    DBG_ERROR( "IpAdapterBase::OpenSocket:%d# BindSocket FAILED with port[%d]", __LINE__, port );
    udp_socket->CloseSocket();
    ret_status = ErrCode::INVALID_PARAMS; goto exit_label_;
  }

exit_label_:

  DBG_INFO( "IpAdapterBase::OpenSocket:%d# EXIT status %d", __LINE__, (int) ret_status );

  return ( ret_status );
}

ErrCode IpAdapterBase::OpenSocket2( IpAddrFamily ip_addr_family, bool is_multicast, IUdpSocket *udp_socket, uint16_t &port )
{
  DBG_INFO( "IpAdapterBase::OpenSocket2:%d# ENTER family[%d], is_mcast[%d], udp_socket[%p] port[%d]", __LINE__, (int) ip_addr_family, is_multicast, udp_socket, port );
  ErrCode ret_status = OpenSocket( ip_addr_family, is_multicast, udp_socket, port );

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR( "IpAdapterBase::OpenSocket2:%d# Failed to open socket with port[%d], family[%d], mcast[%d]", __LINE__, port, (int) ip_addr_family, is_multicast );
    DBG_ERROR( "IpAdapterBase::OpenSocket2:%d# Retrying with port 0", __LINE__ );
    ret_status = OpenSocket( ip_addr_family, is_multicast, udp_socket, 0 );

    if( ret_status != ErrCode::OK )
    {
      DBG_ERROR( "IpAdapterBase::OpenSocket2:%d# Failed to open socket after retrying with port[%d], family[%d], mcast[%d]", __LINE__, port, (int) ip_addr_family, is_multicast );
      goto exit_label_;
    }
  }

  if( ret_status == ErrCode::OK )
  {
    port = udp_socket->GetLocalPort();
    DBG_ERROR( "IpAdapterBase::OpenSocket2:%d# Opened socket with port[%d], family[%d], mcast[%d]", __LINE__, port, (int) ip_addr_family, is_multicast );
  }

exit_label_:

  DBG_INFO( "IpAdapterBase::OpenSocket2:%d# EXIT status %d", __LINE__, (int) ret_status );

  return ( ret_status );
}

ErrCode IpAdapterBase::OpenIPV6Sockets( bool open_ucast_sec, bool open_mcast, bool open_mcast_sec )
{
  ErrCode  ret_status        = ErrCode::OK;
  auto     ip_adapter_config = AdapterManager::Inst().get_ip_adapter_config();
  uint16_t port              = ip_adapter_config->getIpv6UnicastPort();

  DBG_INFO( "IpAdapterBase::OpenIPV6Sockets:%d# ENTER ucast_secure_enabled[%d], mcast_enabled[%d], mcast_secure_enabled[%d]", __LINE__, open_ucast_sec, open_mcast, open_mcast_sec );

  ret_status = OpenSocket2( IpAddrFamily::IPv6, false, ipv6_unicast_socket_, port );

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR( "IpAdapterBase::OpenIPV6Sockets:%d# Failed to open socket IPV6 unicast socket[%p], port[%d]", __LINE__, ipv6_unicast_socket_, ipv6_unicast_port_ );
    goto exit_label_;
  }

  /* update the system generated port number in config */
  ip_adapter_config->setIpv6UnicastPort( port );

  if( open_ucast_sec == true )
  {
    port       = ip_adapter_config->getIpv6UnicastSecurePort();
    ret_status = OpenSocket2( IpAddrFamily::IPv6, false, ipv6_unicast_secure_socket_, port );

    if( ret_status != ErrCode::OK )
    {
      DBG_ERROR( "IpAdapterBase::OpenIPV6Sockets:%d# Failed to open socket IPV6 unicast socket[%p], port[%d]", __LINE__, ipv6_unicast_secure_socket_, ipv6_unicast_secure_port_ );
      goto exit_label_;
    }

    /* update the system generated port number in config */
    ip_adapter_config->setIpv6UnicastSecurePort( port );
  }

  if( open_mcast_sec == true )
  {
    ret_status = OpenSocket( IpAddrFamily::IPv6, true, ipv6_multicast_secure_socket_, ip_adapter_config->getIpv6MulticastSecurePort() );

    if( ret_status != ErrCode::OK )
    {
      DBG_ERROR( "IpAdapterBase::OpenIPV6Sockets:%d# Failed to open socket IPV6 multicast socket[%p], port[%d]", __LINE__, ipv6_multicast_secure_socket_, COAP_SECURE_PORT );
      goto exit_label_;
    }
  }

  if( open_mcast == true )
  {
    ret_status = OpenSocket( IpAddrFamily::IPv6, true, ipv6_multicast_socket_, ip_adapter_config->getIpv6MulticastPort() );

    if( ret_status != ErrCode::OK )
    {
      DBG_ERROR( "IpAdapterBase::OpenIPV6Sockets:%d# Failed to open socket IPV6 multicast socket[%p], port[%d]", __LINE__, ipv6_multicast_socket_, COAP_PORT );
      goto exit_label_;
    }
  }

exit_label_:
  DBG_INFO( "IpAdapterBase::OpenIPV6Sockets:%d# EXIT status %d", __LINE__, (int) ret_status );

  return ( ret_status );
}

ErrCode IpAdapterBase::OpenIPV4Sockets( bool open_ucast_sec, bool open_mcast, bool open_mcast_sec )
{
  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "IpAdapterBase::OpenIPV4Sockets:%d# ENTER ucast_secure_enabled[%d], mcast_enabled[%d], mcast_secure_enabled[%d]", __LINE__, open_ucast_sec, open_mcast, open_mcast_sec );

  auto     ip_adapter_config = AdapterManager::Inst().get_ip_adapter_config();
  uint16_t port              = ip_adapter_config->getIpv4UnicastPort();

  ret_status = OpenSocket2( IpAddrFamily::IPV4, false, ipv4_unicast_socket_, port );

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR( "IpAdapterBase::OpenIPV4Sockets:%d# Failed to open socket IPV4 unicast socket[%p], port[%d]", __LINE__, ipv4_unicast_socket_, ipv4_unicast_port_ );
    goto exit_label_;
  }

  /* update the system generated port number in config */
  ip_adapter_config->setIpv4UnicastPort( port );

  if( open_ucast_sec == true )
  {
    port       = ip_adapter_config->getIpv4UnicastSecurePort();
    ret_status = OpenSocket2( IpAddrFamily::IPV4, false, ipv4_unicast_secure_socket_, port );

    if( ret_status != ErrCode::OK )
    {
      DBG_ERROR( "IpAdapterBase::OpenIPV4Sockets:%d# Failed to open socket IPV4 unicast secure socket[%p], port[%d]", __LINE__, ipv4_unicast_secure_socket_, ipv4_unicast_secure_port_ );
      goto exit_label_;
    }

    /* update the system generated port number in config */
    ip_adapter_config->setIpv4UnicastSecurePort( port );
  }

  if( open_mcast == true )
  {
    ret_status = OpenSocket( IpAddrFamily::IPV4, true, ipv4_multicast_socket_, ip_adapter_config->getIpv4MulticastPort() );

    if( ret_status != ErrCode::OK )
    {
      DBG_ERROR( "IpAdapterBase::OpenIPV4Sockets:%d# Failed to open socket IPV4 multicast socket[%p], port[%d]", __LINE__, ipv4_multicast_socket_, COAP_PORT );
      goto exit_label_;
    }
  }

  if( open_mcast_sec == true )
  {
    ret_status = OpenSocket( IpAddrFamily::IPV4, true, ipv4_multicast_secure_socket_, ip_adapter_config->getIpv4MulticastSecurePort() );

    if( ret_status != ErrCode::OK )
    {
      DBG_ERROR( "IpAdapterBase::OpenIPV4Sockets:%d# Failed to open socket IPV4 multicast secure socket[%p], port[%d]", __LINE__, ipv4_multicast_secure_socket_, COAP_SECURE_PORT );
      goto exit_label_;
    }
  }

exit_label_:

  DBG_INFO( "IpAdapterBase::OpenIPV4Sockets:%d# EXIT status %d", __LINE__, (int) ret_status );

  return ( ret_status );
}

ErrCode IpAdapterBase::HandleInterfaceEvent( InterfaceEvent *interface_event )
{
  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "IpAdapterBase::HandleInterfaceEvent:%d# ENTER interface_event[%p]", __LINE__, interface_event );

  if( interface_event != nullptr )
  {
    ret_status = DoHandleInterfaceEvent( interface_event );

    if( ret_status != ErrCode::OK )
    {
      DBG_ERROR( "IpAdapterBase::HandleInterfaceEvent:%d# DoHandleInterfaceEvent FAILED", __LINE__ );
      goto exit_label_;
    }
  }
  else
  {
    DBG_WARN( "IpAdapterBase::HandleInterfaceEvent:%d# called with interface_event NULL", __LINE__ );
  }

exit_label_:

  DBG_INFO( "IpAdapterBase::HandleInterfaceEvent:%d# EXIT status %d", __LINE__, (int) ret_status );

  return ( ret_status );
}

ErrCode IpAdapterBase::StartInterfaceMonitor()
{
  DBG_INFO( "IpAdapterBase::StartInterfaceMonitor:%d# ENTER", __LINE__ );
  auto interface_monitor = INetworkPlatformFactory::GetCurrFactory()->GetInterfaceMonitor();

  if( interface_monitor != nullptr )
  {
    interface_monitor->AddInterfaceEventHandler( &if_event_handler_ );
    interface_monitor->StartMonitor( AdapterType::IP );
  }
  else
  {
    DBG_WARN( "IpAdapterBase::StartInterfaceMonitor:%d# interface_monitor NULL", __LINE__ );
  }

  DBG_INFO( "IpAdapterBase::StartInterfaceMonitor:%d# EXIT", __LINE__ );

  return ( ErrCode::OK );
}

ErrCode IpAdapterBase::StopInterfaceMonitor()
{
  DBG_INFO( "IpAdapterBase::StopInterfaceMonitor:%d# ENTER", __LINE__ );
  auto interface_monitor = INetworkPlatformFactory::GetCurrFactory()->GetInterfaceMonitor();

  if( interface_monitor != nullptr )
  {
    interface_monitor->RemoveInterfaceEventHandler( &if_event_handler_ );
    interface_monitor->StopMonitor( AdapterType::IP );
  }
  else
  {
    DBG_WARN( "IpAdapterBase::StopInterfaceMonitor:%d# interface_monitor NULL", __LINE__ );
  }

  DBG_INFO( "IpAdapterBase::StopInterfaceMonitor:%d# EXIT", __LINE__ );

  return ( ErrCode::OK );
}
}
}