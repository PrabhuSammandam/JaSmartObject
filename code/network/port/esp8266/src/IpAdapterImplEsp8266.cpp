/*
 * IpAdapterImplEsp8266.cpp
 *
 *  Created on: Sep 11, 2017
 *      Author: psammand
 */

#ifdef _OS_FREERTOS_

#include <lwip/sockets.h>
#include <lwip/ipv4/lwip/inet.h>
#include <common/inc/logging_network.h>
#include <port/esp8266/inc/IpAdapterImplEsp8266.h>
#include <AdapterMgr.h>

namespace ja_iot {
namespace network {
IpAdapterImplEsp8266::IpAdapterImplEsp8266 ()
{
}

IpAdapterImplEsp8266::~IpAdapterImplEsp8266 ()
{
}

ErrCode IpAdapterImplEsp8266::Initialize()
{
  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "IpAdapterImplEsp8266::Initialize:%d# ENTER", __LINE__ );

  this->ipv4_ucast_socket_ = INetworkPlatformFactory::GetCurrFactory()->AllocSocket();

  if( this->ipv4_ucast_socket_ == nullptr )
  {
    DBG_ERROR( "IpAdapterImplEsp8266::Initialize:%d# Failed to alloc ipv4 ucast socket", __LINE__ );
    ret_status = ErrCode::ERR; goto exit_label;
  }

  this->ipv4_mcast_socket_ = INetworkPlatformFactory::GetCurrFactory()->AllocSocket();

  if( this->ipv4_mcast_socket_ == nullptr )
  {
    DBG_ERROR( "IpAdapterImplEsp8266::Initialize:%d# Failed to alloc ipv4 mcast socket", __LINE__ );
    ret_status = ErrCode::ERR; goto exit_label;
  }

  DBG_INFO( "IpAdapterImplEsp8266::Initialize:%d# EXIT", __LINE__ );

exit_label:
  return ( ret_status );
}

ErrCode IpAdapterImplEsp8266::Terminate()
{
  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "IpAdapterImplEsp8266::Terminate:%d# ENTER", __LINE__ );

  if( this->ipv4_ucast_socket_ != nullptr )
  {
    INetworkPlatformFactory::GetCurrFactory()->FreeSocket( this->ipv4_ucast_socket_ );
  }

  if( this->ipv4_mcast_socket_ != nullptr )
  {
    INetworkPlatformFactory::GetCurrFactory()->FreeSocket( this->ipv4_mcast_socket_ );
  }

  SetAdapterHandler( nullptr );

  DBG_INFO( "IpAdapterImplEsp8266::Terminate:%d# EXIT", __LINE__ );

  return ( ret_status );
}

ErrCode IpAdapterImplEsp8266::StartAdapter()
{
  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "IpAdapterImplEsp8266::StartAdapter:%d# ENTER", __LINE__ );

  auto ip_adapter_config = AdapterManager::Inst().get_ip_adapter_config();

  this->ipv4_ucast_port_ = ip_adapter_config->getIpv4UnicastPort();

  ret_status = StartServer();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR( "IpAdapterImplEsp8266::StartAdapter:%d# StartServer() Failed", __LINE__ );
    goto exit_label_;
  }

  ret_status = StartListening();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR( "IpAdapterImplEsp8266::StartAdapter:%d# StartListening() Failed", __LINE__ );
    goto exit_label_;
  }

exit_label_:

  DBG_INFO( "IpAdapterImplEsp8266::StartAdapter:%d# EXIT", __LINE__ );

  return ( ret_status );
}

ErrCode IpAdapterImplEsp8266::StopAdapter()
{
  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "IpAdapterImplEsp8266::StopAdapter:%d# ENTER", __LINE__ );

  ret_status = StopServer();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR( "IpAdapterImplEsp8266::StopAdapter:%d# StopServer() Failed", __LINE__ );
    goto exit_label_;
  }

  this->ipv4_ucast_port_ = 0;

exit_label_:
  DBG_INFO( "IpAdapterImplEsp8266::StopAdapter:%d# EXIT", __LINE__ );

  return ( ret_status );
}

ErrCode IpAdapterImplEsp8266::StartServer()
{
  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "IpAdapterImplEsp8266::StartServer:%d# ENTER", __LINE__ );

  auto ip_adapter_config = AdapterManager::Inst().get_ip_adapter_config();

  /* Start UNICAST server */
  ret_status = OpenSocket2( IpAddrFamily::IPV4, false, ipv4_ucast_socket_, this->ipv4_ucast_port_ );

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR( "IpAdapterBase::StartServer:%d# Failed to open socket IPV4 unicast socket[%p], port[%d]", __LINE__, ipv4_ucast_socket_, this->ipv4_ucast_port_ );
    return ( ret_status );
  }

  /* Start MULTICAST server */
  ret_status = OpenSocket( IpAddrFamily::IPV4, true, ipv4_mcast_socket_, ip_adapter_config->getIpv4MulticastPort() );

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR( "IpAdapterBase::StartServer:%d# Failed to open socket IPV4 multicast socket[%p], port[%d]", __LINE__, ipv4_mcast_socket_, ip_adapter_config->getIpv4MulticastPort() );
    return ( ret_status );
  }

  DBG_INFO( "IpAdapterImplEsp8266::StartServer:%d# EXIT", __LINE__ );

  return ( ret_status );
}

ErrCode IpAdapterImplEsp8266::StopServer()
{
  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "IpAdapterImplEsp8266::StopServer:%d# ENTER", __LINE__ );

  /* Stop UNICAST server */
  if( ipv4_ucast_socket_ != nullptr )
  {
    DBG_INFO( "IpAdapterImplEsp8266::StopServer:%d# Closing IPV4 ucast socket", __LINE__ );
    ipv4_ucast_socket_->CloseSocket();
  }

  /* Stop MULTICAST server */
  if( ipv4_mcast_socket_ != nullptr )
  {
    DBG_INFO( "IpAdapterImplEsp8266::StopServer:%d# Closing IPV4 mcast socket", __LINE__ );
    ipv4_mcast_socket_->CloseSocket();
  }

  DBG_INFO( "IpAdapterImplEsp8266::StopServer:%d# EXIT", __LINE__ );

  return ( ret_status );
}

ErrCode IpAdapterImplEsp8266::StartListening()
{
  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "IpAdapterImplEsp8266::StartListening:%d# ENTER", __LINE__ );

  IpAddress ipv4_multicast_addr{ 224, 0, 1, 187 };

  if( ipv4_mcast_socket_->JoinMulticastGroup( ipv4_multicast_addr, 0 ) != SocketError::OK )
  {
    DBG_ERROR( "IpAdapterImplEsp8266::StartListening:%d# Join Multicast Failed Try:1", __LINE__ );
    ipv4_mcast_socket_->LeaveMulticastGroup( ipv4_multicast_addr, 0 );

    if( ipv4_mcast_socket_->JoinMulticastGroup( ipv4_multicast_addr, 0 ) != SocketError::OK )
    {
      ret_status = ErrCode::ERR;
      DBG_ERROR( "IpAdapterImplEsp8266::StartListening:%d# Join Multicast Failed Try:2", __LINE__ );
    }
  }

  DBG_INFO( "IpAdapterImplEsp8266::StartListening:%d# EXIT", __LINE__ );

  return ( ret_status );
}

ErrCode IpAdapterImplEsp8266::StopListening()
{
  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "IpAdapterImplEsp8266::StopListening:%d# ENTER", __LINE__ );

  IpAddress ipv4_multicast_addr{ 224, 0, 1, 187 };

  if( ipv4_mcast_socket_->LeaveMulticastGroup( ipv4_multicast_addr, 0 ) != SocketError::OK )
  {
    ret_status = ErrCode::ERR;
    DBG_ERROR( "IpAdapterImplEsp8266::StopListening:%d# LeaveMulticastGroup() FAILED", __LINE__ );
  }

  DBG_INFO( "IpAdapterImplEsp8266::StopListening:%d# EXIT", __LINE__ );

  return ( ret_status );
}

int32_t IpAdapterImplEsp8266::SendUnicastData( Endpoint &end_point, const uint8_t *data, uint16_t data_length )
{
  int32_t send_bytes = 0;

  DBG_INFO( "IpAdapterImplEsp8266::SendUnicastData:%d# ENTER", __LINE__ );

  send_bytes = send_data( end_point, data, data_length, false );

  if( send_bytes <= 0 )
  {
    DBG_ERROR( "IpAdapterImplEsp8266::SendUnicastData:%d# send_data() Failed", __LINE__ );
  }

  DBG_INFO( "IpAdapterImplEsp8266::SendUnicastData:%d# EXIT", __LINE__ );

  return ( send_bytes );
}

int32_t IpAdapterImplEsp8266::SendMulticastData( Endpoint &end_point, const uint8_t *data, uint16_t data_length )
{
  int32_t send_bytes = 0;

  DBG_INFO( "IpAdapterImplEsp8266::SendMulticastData:%d# ENTER", __LINE__ );

  send_bytes = send_data( end_point, data, data_length, true );

  if( send_bytes <= 0 )
  {
    DBG_ERROR( "IpAdapterImplEsp8266::SendMulticastData:%d# send_data() Failed", __LINE__ );
  }

  DBG_INFO( "IpAdapterImplEsp8266::SendMulticastData:%d# EXIT", __LINE__ );

  return ( send_bytes );
}

void IpAdapterImplEsp8266::ReadData()
{
  // DBG_INFO( "IpAdapterImplEsp8266::ReadData:%d# ENTER", __LINE__ );

  IpAddress remote_addr;
  auto      data        = new uint8_t[100];
  int16_t   data_length = 100;
  uint16_t  port;
  uint8_t   addr_buff[16];

  ipv4_ucast_socket_->ReceiveData( remote_addr, port, &data[0], data_length );

  if( data_length > 0 )
  {
    remote_addr.to_string( addr_buff, sizeof(addr_buff) );
    DBG_INFO( "IpAdapterImplEsp8266::ReadData:%d# Received unicast from %s port %d data length %d", __LINE__, addr_buff, port, data_length );
  }

  ipv4_mcast_socket_->ReceiveData( remote_addr, port, &data[0], data_length );

  if( data_length > 0 )
  {
    remote_addr.to_string( addr_buff, sizeof(addr_buff) );
    DBG_INFO( "IpAdapterImplEsp8266::ReadData:%d# Received multicast from %s port %d data length %d", __LINE__, addr_buff, port, data_length );
  }

  delete data;
  // DBG_INFO( "IpAdapterImplEsp8266::ReadData:%d# EXIT", __LINE__ );
}

AdapterType IpAdapterImplEsp8266::GetType()
{
  return ( AdapterType::IP );
}

void IpAdapterImplEsp8266::SetAdapterHandler( IAdapterEventHandler *adapter_event_handler )
{
  DBG_INFO( "IpAdapterImplEsp8266::SetAdapterHandler:%d# p_adapter_handler_ %p", __LINE__, adapter_event_handler );
  p_adapter_handler_ = adapter_event_handler;
}

ErrCode IpAdapterImplEsp8266::OpenSocket( IpAddrFamily ip_addr_family, bool is_multicast, IUdpSocket *udp_socket, uint16_t port )
{
  IpAddress ip_addr{ ip_addr_family };

  DBG_INFO( "IpAdapterImplEsp8266::OpenSocket:%d# ENTER family[%d], mcast[%d], port[%d]", __LINE__, (int) ip_addr_family, is_multicast, port );

  if( udp_socket->OpenSocket( ip_addr_family ) != SocketError::OK )
  {
    DBG_ERROR( "IpAdapterImplEsp8266::OpenSocket:%d# Failed to open with family[%d]", __LINE__, (int) ip_addr_family );
    return ( ErrCode::ERR );
  }

  if( udp_socket->EnablePacketInfo( true ) != SocketError::OK )
  {
    DBG_ERROR( "IpAdapterImplEsp8266::OpenSocket:%d# EnablePacketInfo() FAILED", __LINE__ );
  }

  udp_socket->SetBlocking( false );

  if( ip_addr_family == IpAddrFamily::IPv6 )
  {
    udp_socket->EnableIpv6Only( true );
    DBG_INFO( "IpAdapterImplEsp8266::OpenSocket:%d# Calling EnableIpv6Only()", __LINE__ );
  }

  if( is_multicast && ( port != 0 ) )
  {
    udp_socket->EnableReuseAddr( true );
    DBG_INFO( "IpAdapterImplEsp8266::OpenSocket:%d# Calling EnableReuseAddr()", __LINE__ );
  }

  if( udp_socket->BindSocket( ip_addr, port ) != SocketError::OK )
  {
    DBG_ERROR( "IpAdapterImplEsp8266::OpenSocket:%d# BindSocket FAILED with port[%d]", __LINE__, port );
    udp_socket->CloseSocket();

    return ( ErrCode::ERR );
  }

  DBG_INFO( "IpAdapterImplEsp8266::OpenSocket:%d# EXIT", __LINE__ );

  return ( ErrCode::OK );
}

ErrCode IpAdapterImplEsp8266::OpenSocket2( IpAddrFamily ip_addr_family, bool is_multicast, IUdpSocket *udp_socket, uint16_t &port )
{
  DBG_INFO( "IpAdapterImplEsp8266::OpenSocket2:%d# ENTER family[%d], mcast[%d], port[%d]", __LINE__, (int) ip_addr_family, is_multicast, port );
  ErrCode ret_status = OpenSocket( ip_addr_family, is_multicast, udp_socket, port );

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR( "IpAdapterImplEsp8266::OpenSocket2:%d# Failed to open socket with port[%d], family[%d], mcast[%d]", __LINE__, port, (int) ip_addr_family, is_multicast );
    DBG_ERROR( "IpAdapterImplEsp8266::OpenSocket2:%d# Retrying with port 0", __LINE__ );
    ret_status = OpenSocket( ip_addr_family, is_multicast, udp_socket, 0 );

    if( ret_status != ErrCode::OK )
    {
      DBG_ERROR( "IpAdapterImplEsp8266::OpenSocket2:%d# Failed to open socket after retrying with port[%d], family[%d], mcast[%d]", __LINE__, port, (int) ip_addr_family, is_multicast );
      return ( ret_status );
    }
  }

  if( ret_status == ErrCode::OK )
  {
    port = udp_socket->GetLocalPort();
    DBG_ERROR( "IpAdapterImplEsp8266::OpenSocket2:%d# Opened socket with port[%d], family[%d], mcast[%d]", __LINE__, port, (int) ip_addr_family, (int) is_multicast );
  }

  DBG_INFO( "IpAdapterImplEsp8266::OpenSocket2:%d# Successfully opened socket at port[%d]", __LINE__, port );

  DBG_INFO( "IpAdapterImplEsp8266::OpenSocket2:%d# EXIT", __LINE__ );

  return ( ret_status );
}

int32_t IpAdapterImplEsp8266::send_data( Endpoint &end_point, const uint8_t *data, uint16_t data_length, bool is_multicast )
{
  int32_t bytes_sent = 0;

  DBG_INFO( "IpAdapterImplEsp8266::send_data:%d# ENTER data_length[%d], mcast[%d]", __LINE__, data_length, is_multicast );

  if( data == nullptr )
  {
    DBG_ERROR( "IpAdapterImplEsp8266::SendUnicastData:%d# data is NULL", __LINE__ );
    bytes_sent = -1; goto exit_label_;
  }

  if( data_length <= 0 )
  {
    DBG_ERROR( "IpAdapterImplEsp8266::SendUnicastData:%d# data length is INVALID %d", __LINE__, data_length );
    bytes_sent = -1; goto exit_label_;
  }

  if( is_multicast )
  {
    IpAddress ip_addr{ 224, 0, 1, 187 };

    DBG_INFO( "IpAdapterImplEsp8266::send_data:%d# SelectingMulticastInterface", __LINE__ );

    if( ipv4_ucast_socket_->SelectMulticastInterface( ip_addr, 1 ) != SocketError::OK )
    {
      DBG_ERROR( "IpAdapterImplEsp8266::send_data:%d# SelectMulticastInterface() FAILED, if_index %d", __LINE__, 1 );
      bytes_sent = -1; goto exit_label_;
    }

    DBG_INFO( "IpAdapterImplEsp8266::send_data:%d# Multicast SendData ip[], port[%d], len[%d]", __LINE__, end_point.getPort(), data_length );

    if( ipv4_ucast_socket_->SendData( ip_addr, end_point.getPort(), (uint8_t *) data, data_length ) != SocketError::OK )
    {
      DBG_ERROR( "IpAdapterImplEsp8266::send_data:%d# Multicast SendData() FAILED, if_index %d", __LINE__, 1 );
      bytes_sent = -1; goto exit_label_;
    }
  }
  else
  {
    IpAddress ip_addr{};

    ipaddr_aton( (const char *) end_point.getAddr(), (ip_addr_t *) ip_addr.get_addr() );

    DBG_INFO( "IpAdapterImplEsp8266::send_data:%d# Unicast SendData ip[], port[%d], len[%d]", __LINE__, end_point.getPort(), data_length );

    if( ipv4_ucast_socket_->SendData( ip_addr, end_point.getPort(), (uint8_t *) data, data_length ) != SocketError::OK )
    {
      DBG_ERROR( "IpAdapterImplEsp8266::send_data:%d# Unicast SendData() FAILED, if_index %d", __LINE__, 1 );
      bytes_sent = -1; goto exit_label_;
    }
  }

exit_label_:
  DBG_INFO( "IpAdapterImplEsp8266::send_data:%d# EXIT", __LINE__ );

  return ( bytes_sent );
}
}  // namespace network
}  // namespace ja_iot

#endif /* _OS_FREERTOS_ */
