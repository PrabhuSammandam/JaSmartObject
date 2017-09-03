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

using namespace ja_iot::osal;

namespace ja_iot {
namespace network {
IpAdapterBase::IpAdapterBase ()
{}

IpAdapterBase::~IpAdapterBase ()
{
}

ErrCode IpAdapterBase::Initialize()
{
  ErrCode ret_status = DoPreIntialize();

  if( ret_status != ErrCode::OK )
  {
    return ( ret_status );
  }

  ret_status = InitInternal();

  if( ret_status != ErrCode::OK )
  {
    return ( ret_status );
  }

  send_msg_q_mutex_ = OsalMgr::Inst()->AllocMutex();

  if( send_msg_q_mutex_ == nullptr )
  {
    return ( ErrCode::ERR );
  }

  ret_status = DoPostIntialize();

  return ( ret_status );
}

ErrCode IpAdapterBase::Terminate()
{
  ErrCode ret_status = DoPreTerminate();

  if( ret_status != ErrCode::OK )
  {
    return ( ret_status );
  }

  ret_status = TerminateInternal();

  if( ret_status != ErrCode::OK )
  {
    return ( ret_status );
  }

  if( _msgSendtask != nullptr )
  {
    _msgSendtask->Destroy();
    OsalMgr::Inst()->FreeTask( _msgSendtask );
    _msgSendtask = nullptr;
  }

  if( send_msg_q_mutex_ )
  {
    OsalMgr::Inst()->FreeMutex( send_msg_q_mutex_ );
    send_msg_q_mutex_ = nullptr;
  }

  ret_status = DoPostTerminate();

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

  StartInterfaceMonitor();

  /* start the sending thread*/
  ret_status = CreateAndStartSendingThread();

  if( ret_status != ErrCode::OK )
  {
    return ( ret_status );
  }

  ret_status = StartServer();

  if( ret_status != ErrCode::OK )
  {
    return ( ret_status );
  }

  return ( ErrCode::OK );
}

ErrCode IpAdapterBase::StopAdapter()
{
  _msgSendtask->Stop();

  StopInterfaceMonitor();
  StopServer();

  return ( ErrCode::OK );
}

void IpAdapterBase::HandleMsg( IpAdapterQMsg *msg )
{
  if( msg == nullptr )
  {
    return;
  }

  DoHandleSendMsg( msg );
}

ErrCode IpAdapterBase::CreateAndStartSendingThread()
{
  _msgSendtask = OsalMgr::Inst()->AllocTask();

  if( _msgSendtask == nullptr )
  {
    return ( ErrCode::OUT_OF_MEM );
  }

  TaskMsgQParam taskMsgQParam{};

  taskMsgQParam.msgQ           = &_ipAdapterMsgQ;
  taskMsgQParam.taskMsgHandler = &_ipSendTaskMsgHandler;

  auto osalRetStatus = _msgSendtask->InitWithMsgQ( (uint8_t*) "IpTask", 0, 0, &taskMsgQParam, nullptr );

  if( osalRetStatus != OsalError::OK )
  {
    _msgSendtask->Destroy();
    OsalMgr::Inst()->FreeTask( _msgSendtask );
    _msgSendtask = nullptr;
    return ( ErrCode::ERR );
  }

  osalRetStatus = _msgSendtask->Start();

  if( osalRetStatus != OsalError::OK )
  {
    _msgSendtask->Destroy();
    OsalMgr::Inst()->FreeTask( _msgSendtask );
    _msgSendtask = nullptr;
    return ( ErrCode::ERR );
  }

  return ( ErrCode::OK );
}

IpAdapterQMsg * IpAdapterBase::CreateNewSendMsg( const Endpoint &end_point, const uint8_t *data, uint16_t data_length, bool is_multicast )
{
  send_msg_q_mutex_->Lock();
  auto msg_q_entry = _ipAdapterMsgList.Alloc();
  send_msg_q_mutex_->Unlock();

  if( msg_q_entry == nullptr )
  {
    return ( nullptr );
  }

  msg_q_entry->end_point_ = end_point;
  msg_q_entry->_data      = new uint8_t[data_length] {};

  if( msg_q_entry->_data == nullptr )
  {
    send_msg_q_mutex_->Lock();
    _ipAdapterMsgList.Free( msg_q_entry );
    send_msg_q_mutex_->Unlock();

    return ( nullptr );
  }

  memcpy( &msg_q_entry->_data[0], data, data_length );

  msg_q_entry->_dataLength  = data_length;
  msg_q_entry->is_multicast = false;

  return ( msg_q_entry );
}

ErrCode IpAdapterBase::CreateAndStartReceiveTask()
{
  receive_task_ = OsalMgr::Inst()->AllocTask();

  if( receive_task_ == nullptr )
  {
    return ( ErrCode::OUT_OF_MEM );
  }

  auto osal_ret_status = receive_task_->Init( (uint8_t*) "IpTask", 0, 0, &receive_task_routine_, this );

  if( osal_ret_status != OsalError::OK )
  {
    receive_task_->Destroy();
    OsalMgr::Inst()->FreeTask( receive_task_ );
    receive_task_ = nullptr;

    return ( ErrCode::ERR );
  }

  osal_ret_status = receive_task_->Start();

  if( osal_ret_status != OsalError::OK )
  {
    receive_task_->Destroy();
    OsalMgr::Inst()->FreeTask( receive_task_ );
    receive_task_ = nullptr;
    return ( ErrCode::ERR );
  }

  return ( ErrCode::OK );
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

void IpAdapterBase::StartIpv4MulticastAtInterface( uint32_t if_index )
{
  IpAddress ipv4_multicast_addr{ 224, 0, 1, 187 };

  ApplyMulticastAtInterfaceByAddress( ipv4_multicast_socket_, ipv4_multicast_addr, if_index );
  ApplyMulticastAtInterfaceByAddress( ipv4_multicast_secure_socket_, ipv4_multicast_addr, if_index );
}

void IpAdapterBase::StartIpv6MulticastAtInterface( uint32_t if_index )
{
  IpAddress ipv6_address{ Ipv6AddrScope::LINK, 0x158 };

  ApplyMulticastAtInterfaceByAddress( ipv6_multicast_socket_, ipv6_address, if_index );
  ApplyMulticastAtInterfaceByAddress( ipv6_multicast_secure_socket_, ipv6_address, if_index );

  ipv6_address.SetAddrByScope( Ipv6AddrScope::REALM, 0x158 );
  ApplyMulticastAtInterfaceByAddress( ipv6_multicast_socket_, ipv6_address, if_index );
  ApplyMulticastAtInterfaceByAddress( ipv6_multicast_secure_socket_, ipv6_address, if_index );

  ipv6_address.SetAddrByScope( Ipv6AddrScope::SITE, 0x158 );
  ApplyMulticastAtInterfaceByAddress( ipv6_multicast_socket_, ipv6_address, if_index );
  ApplyMulticastAtInterfaceByAddress( ipv6_multicast_secure_socket_, ipv6_address, if_index );
}

void IpAdapterBase::ApplyMulticastAtInterfaceByAddress( IUdpSocket *udp_socket, IpAddress &ip_multicast_addr, uint32_t if_index )
{
  if( udp_socket->JoinMulticastGroup( ip_multicast_addr, if_index ) != SocketError::OK )
  {
    udp_socket->LeaveMulticastGroup( ip_multicast_addr, if_index );

    udp_socket->JoinMulticastGroup( ip_multicast_addr, if_index );
  }
}

void IpAdapterBase::DeleteMsg( IpAdapterQMsg *msg )
{
  DoDeleteMsg( msg );

  if( msg->_data != nullptr )
  {
    delete[] msg->_data;
  }

  send_msg_q_mutex_->Lock();
  _ipAdapterMsgList.Free( msg );
  send_msg_q_mutex_->Unlock();
}

ErrCode IpAdapterBase::InitInternal()
{
  ErrCode ret_status = CreateSockets();

  return ( ret_status );
}

ErrCode IpAdapterBase::TerminateInternal()
{
  ipv4_unicast_socket_->CloseSocket();
  ipv4_unicast_secure_socket_->CloseSocket();

  ipv4_multicast_socket_->CloseSocket();
  ipv4_multicast_secure_socket_->CloseSocket();

  ipv6_unicast_socket_->CloseSocket();
  ipv6_unicast_secure_socket_->CloseSocket();

  ipv6_multicast_socket_->CloseSocket();
  ipv6_multicast_secure_socket_->CloseSocket();

  DoUnInitAddressChangeNotifyMechanism();

  return ( ErrCode::OK );
}

int32_t IpAdapterBase::SendUnicastData( const Endpoint &end_point, const uint8_t *data, uint16_t data_length )
{
  if( ( data == nullptr ) || ( data_length == 0 ) )
  {
    return ( -1 );
  }

  auto new_msg_to_send = CreateNewSendMsg( end_point, data, data_length );

  if( new_msg_to_send == nullptr )
  {
    return ( -1 );
  }

  _msgSendtask->SendMsg( new_msg_to_send );

  return ( data_length );
}

int32_t IpAdapterBase::SendMulticastData( const Endpoint &end_point, const uint8_t *data, uint16_t data_length )
{
  if( ( data == nullptr ) || ( data_length == 0 ) )
  {
    return ( -1 );
  }

  auto new_msg_to_send = CreateNewSendMsg( end_point, data, data_length, true );

  if( new_msg_to_send == nullptr )
  {
    return ( -1 );
  }

  _msgSendtask->SendMsg( new_msg_to_send );

  return ( data_length );
}

ErrCode IpAdapterBase::StartServer()
{
  if( is_started_ == true )
  {
    return ( ErrCode::OK );
  }

  ErrCode ret_status = DoPreStartServer();

  if( ret_status != ErrCode::OK )
  {
    return ( ret_status );
  }

  if( !is_ipv6_enabled_ && !is_ipv4_enabled_ )
  {
    is_ipv4_enabled_ = true;
    is_ipv6_enabled_ = true;
  }

  if( is_ipv6_enabled_ )
  {
    OpenIPV6Sockets();
  }

  if( is_ipv4_enabled_ )
  {
    OpenIPV4Sockets();
  }

  InitFastShutdownMechanism();

  InitAddressChangeNotifyMechanism();

  ret_status = DoPostStartServer();

  if( ret_status != ErrCode::OK )
  {
    return ( ret_status );
  }

  ret_status = StartListening();

  if( ret_status != ErrCode::OK )
  {
    return ( ret_status );
  }

  is_terminated_ = false;

  IpAdapterBase::CreateAndStartReceiveTask();

  is_started_ = true;

  return ( ErrCode::OK );
}

ErrCode IpAdapterBase::StopServer()
{
  is_terminated_ = true;
  is_started_    = false;

  ErrCode ret_status = DoPreStopServer();

  if( ret_status != ErrCode::OK )
  {
    return ( ret_status );
  }

  ret_status = DoPostStopServer();

  return ( ret_status );
}

ErrCode IpAdapterBase::StartListening()
{
  if( is_started_ )
  {
    return ( ErrCode::OK );
  }

  auto interface_monitor = INetworkPlatformFactory::GetCurrFactory()->GetInterfaceMonitor();

  if( interface_monitor == nullptr )
  {
    return ( ErrCode::OK );
  }

  ja_iot::base::StaticPtrArray<InterfaceAddress *, 10> if_addr_array{};

  interface_monitor->GetInterfaceAddrList( if_addr_array );

  for( auto &if_addr : if_addr_array )
  {
    if( if_addr != nullptr )
    {
      if( ( if_addr->getFamily() == IpAddrFamily::IPV4 ) && is_ipv4_enabled_ )
      {
        StartIpv4MulticastAtInterface( if_addr->getIndex() );
      }

      if( ( if_addr->getFamily() == IpAddrFamily::IPv6 ) && is_ipv6_enabled_ )
      {
        StartIpv6MulticastAtInterface( if_addr->getIndex() );
      }

      delete if_addr;
    }
  }

  return ( ErrCode::OK );
}

ErrCode IpAdapterBase::StopListening()
{
  auto interface_monitor = INetworkPlatformFactory::GetCurrFactory()->GetInterfaceMonitor();

  if( interface_monitor == nullptr )
  {
    return ( ErrCode::OK );
  }

  ja_iot::base::StaticPtrArray<InterfaceAddress *, 10> if_addr_array{};

  interface_monitor->GetInterfaceAddrList( if_addr_array );

  for( auto &if_addr : if_addr_array )
  {
    if( if_addr != nullptr )
    {
      if( if_addr->getFamily() == IpAddrFamily::IPV4 )
      {
        ipv4_multicast_socket_->CloseSocket();
        ipv4_multicast_secure_socket_->CloseSocket();
      }

      if( if_addr->getFamily() == IpAddrFamily::IPv6 )
      {
        ipv6_multicast_socket_->CloseSocket();
        ipv6_multicast_secure_socket_->CloseSocket();
      }

      delete if_addr;
    }
  }

  return ( ErrCode::OK );
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
  ipv4_unicast_socket_ = INetworkPlatformFactory::GetCurrFactory()->AllocSocket();

  if( ipv4_unicast_socket_ == nullptr )
  {
    return ( ErrCode::ERR );
  }

  ipv4_unicast_secure_socket_ = INetworkPlatformFactory::GetCurrFactory()->AllocSocket();

  if( ipv4_unicast_secure_socket_ == nullptr )
  {
    return ( ErrCode::ERR );
  }

  ipv4_multicast_socket_ = INetworkPlatformFactory::GetCurrFactory()->AllocSocket();

  if( ipv4_multicast_socket_ == nullptr )
  {
    return ( ErrCode::ERR );
  }

  ipv4_multicast_secure_socket_ = INetworkPlatformFactory::GetCurrFactory()->AllocSocket();

  if( ipv4_multicast_secure_socket_ == nullptr )
  {
    return ( ErrCode::ERR );
  }

  ipv6_unicast_socket_ = INetworkPlatformFactory::GetCurrFactory()->AllocSocket();

  if( ipv6_unicast_socket_ == nullptr )
  {
    return ( ErrCode::ERR );
  }

  ipv6_unicast_secure_socket_ = INetworkPlatformFactory::GetCurrFactory()->AllocSocket();

  if( ipv6_unicast_secure_socket_ == nullptr )
  {
    return ( ErrCode::ERR );
  }

  ipv6_multicast_socket_ = INetworkPlatformFactory::GetCurrFactory()->AllocSocket();

  if( ipv6_multicast_socket_ == nullptr )
  {
    return ( ErrCode::ERR );
  }

  ipv6_multicast_secure_socket_ = INetworkPlatformFactory::GetCurrFactory()->AllocSocket();

  if( ipv6_multicast_secure_socket_ == nullptr )
  {
    return ( ErrCode::ERR );
  }

  return ( ErrCode::OK );
}

ErrCode IpAdapterBase::OpenSocket( IpAddrFamily ip_addr_family, bool is_multicast, IUdpSocket *udp_socket, uint16_t port )
{
  IpAddress ip_addr{ ip_addr_family };

  if( udp_socket->OpenSocket( ip_addr_family ) != SocketError::OK )
  {
    return ( ErrCode::ERR );
  }

  udp_socket->EnablePacketInfo();

  if( ip_addr_family == IpAddrFamily::IPv6 )
  {
    udp_socket->EnableIpv6Only();
  }

  if( is_multicast && ( port != 0 ) )
  {
    udp_socket->EnableReuseAddr();
  }

  if( udp_socket->BindSocket( ip_addr, port ) != SocketError::OK )
  {
    udp_socket->CloseSocket();

    return ( ErrCode::ERR );
  }

  return ( ErrCode::OK );
}

ErrCode IpAdapterBase::OpenSocket2( IpAddrFamily ip_addr_family, bool is_multicast, IUdpSocket *udp_socket, uint16_t &port )
{
  ErrCode ret_status = OpenSocket( ip_addr_family, is_multicast, udp_socket, port );

  if( ret_status != ErrCode::OK )
  {
    ret_status = OpenSocket( ip_addr_family, is_multicast, udp_socket, 0 );

    if( ret_status != ErrCode::OK )
    {
      return ( ret_status );
    }
  }

  if( ret_status == ErrCode::OK )
  {
    port = udp_socket->GetLocalPort();
    printf( "Created %d socket at port %d\n", (int32_t)ip_addr_family, port );
  }

  return ( ret_status );
}

ErrCode IpAdapterBase::OpenIPV6Sockets()
{
  ErrCode ret_status = ErrCode::OK;

  ret_status = OpenSocket2( IpAddrFamily::IPv6, false, ipv6_unicast_socket_, ipv6_unicast_port_ );

  if( ret_status != ErrCode::OK )
  {
    return ( ret_status );
  }

  ret_status = OpenSocket2( IpAddrFamily::IPv6, false, ipv6_unicast_secure_socket_, ipv6_unicast_secure_port_ );

  if( ret_status != ErrCode::OK )
  {
    return ( ret_status );
  }


  ret_status = OpenSocket( IpAddrFamily::IPv6, true, ipv6_multicast_secure_socket_, COAP_SECURE_PORT );

  if( ret_status != ErrCode::OK )
  {
    return ( ret_status );
  }

  ret_status = OpenSocket( IpAddrFamily::IPv6, true, ipv6_multicast_socket_, COAP_PORT );

  if( ret_status != ErrCode::OK )
  {
    return ( ret_status );
  }

  return ( ErrCode::OK );
}

ErrCode IpAdapterBase::OpenIPV4Sockets()
{
  ErrCode ret_status = ErrCode::OK;

  ret_status = OpenSocket2( IpAddrFamily::IPV4, false, ipv4_unicast_socket_, ipv4_unicast_port_ );

  if( ret_status != ErrCode::OK )
  {
    return ( ret_status );
  }

  ret_status = OpenSocket2( IpAddrFamily::IPV4, false, ipv4_unicast_secure_socket_, ipv4_unicast_secure_port_ );

  if( ret_status != ErrCode::OK )
  {
    return ( ret_status );
  }

  ret_status = OpenSocket( IpAddrFamily::IPV4, true, ipv4_multicast_socket_, COAP_PORT );

  if( ret_status != ErrCode::OK )
  {
    return ( ret_status );
  }

  ret_status = OpenSocket( IpAddrFamily::IPV4, true, ipv4_multicast_secure_socket_, COAP_SECURE_PORT );

  if( ret_status != ErrCode::OK )
  {
    return ( ret_status );
  }

  return ( ErrCode::OK );
}

ErrCode IpAdapterBase::HandleInterfaceEvent( InterfaceEvent *interface_event )
{
  ErrCode ret_status = ErrCode::OK;

  if( interface_event != nullptr )
  {
    ret_status = DoHandleInterfaceEvent( interface_event );
  }

  return ( ret_status );
}

ErrCode IpAdapterBase::StartInterfaceMonitor()
{
  auto interface_monitor = INetworkPlatformFactory::GetCurrFactory()->GetInterfaceMonitor();

  if( interface_monitor != nullptr )
  {
    interface_monitor->AddInterfaceEventHandler( &if_event_handler_ );
    interface_monitor->StartMonitor( AdapterType::IP );
  }

  return ( ErrCode::OK );
}

ErrCode IpAdapterBase::StopInterfaceMonitor()
{
  auto interface_monitor = INetworkPlatformFactory::GetCurrFactory()->GetInterfaceMonitor();

  if( interface_monitor != nullptr )
  {
    interface_monitor->RemoveInterfaceEventHandler( &if_event_handler_ );
    interface_monitor->StopMonitor( AdapterType::IP );
  }

  return ( ErrCode::OK );
}
}
}
