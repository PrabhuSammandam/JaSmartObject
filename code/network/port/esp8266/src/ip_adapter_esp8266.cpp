/*
 * IpAdapterImplEsp8266.cpp
 *
 *  Created on: Sep 11, 2017
 *      Author: psammand
 */

#ifdef _OS_FREERTOS_

#include <functional>
#include <cstring>
#include <lwip/sockets.h>
#include <lwip/ipv4/lwip/inet.h>
#include "common/inc/logging_network.h"
#include "port/esp8266/inc/ip_adapter_esp8266.h"
#include "ErrCode.h"
#include "adapter_mgr.h"
#include "config_mgr.h"
#include "base_consts.h"
#include "ja_debug.h"
#include "OsalMgr.h"
#include "port/esp8266/inc/udp_socket_esp8266.h"

constexpr const char *const __FILE_NAME__ = "ip_adapter_esp8266";


constexpr const char *const IP_ADAPTER_ESP8266_RECEIVE_TASK_NAME       = "RCV_TASK";
constexpr uint32_t          IP_ADAPTER_ESP8266_RECEIVE_TASK_PRIORITY   = 5;
constexpr uint32_t          IP_ADAPTER_ESP8266_RECEIVE_TASK_STACK_SIZE = 256;

namespace ja_iot {
namespace network {
static void receive_data_routine( void *arg );
using namespace ja_iot::base;
using namespace ja_iot::osal;
IpAdapterImplEsp8266::IpAdapterImplEsp8266 ()
{
}
IpAdapterImplEsp8266::~IpAdapterImplEsp8266 ()
{
}

ErrCode IpAdapterImplEsp8266::initialize()
{
  ErrCode ret_status = ErrCode::OK;

  DBG_INFO2( "ENTER" );

  this->_ipv4_ucast_socket = INetworkPlatformFactory::GetCurrFactory()->AllocSocket();

  if( this->_ipv4_ucast_socket == nullptr )
  {
    DBG_ERROR2( "Failed to alloc ipv4 ucast socket" );
    ret_status = ErrCode::ERR; goto exit_label;
  }

  this->_ipv4_mcast_socket = INetworkPlatformFactory::GetCurrFactory()->AllocSocket();

  if( this->_ipv4_mcast_socket == nullptr )
  {
    DBG_ERROR2( "Failed to alloc ipv4 mcast socket" );
    ret_status = ErrCode::ERR; goto exit_label;
  }

  DBG_INFO2( "EXIT" );

exit_label:
  return ( ret_status );
}

ErrCode IpAdapterImplEsp8266::terminate()
{
  ErrCode ret_status = ErrCode::OK;

  DBG_INFO2( "ENTER");

  if( this->_ipv4_ucast_socket != nullptr )
  {
    INetworkPlatformFactory::GetCurrFactory()->free_socket( this->_ipv4_ucast_socket );
    this->_ipv4_ucast_socket = nullptr;
  }

  if( this->_ipv4_mcast_socket != nullptr )
  {
    INetworkPlatformFactory::GetCurrFactory()->free_socket( this->_ipv4_mcast_socket );
    this->_ipv4_mcast_socket = nullptr;
  }

  _adapter_event_cb      = nullptr;
  _adapter_event_cb_data = nullptr;

  DBG_INFO2( "EXIT");

  return ( ret_status );
}

ErrCode IpAdapterImplEsp8266::start_adapter()
{
  ErrCode ret_status = ErrCode::OK;

  DBG_INFO2( "ENTER");

  ret_status = start_server();

  JA_ERR_OK_PRINT_GO( ret_status, "IpAdapterImplEsp8266::StartAdapter:%d# StartServer() Failed"  );


exit_label_:

  DBG_INFO2( "EXIT");

  return ( ret_status );
}

ErrCode IpAdapterImplEsp8266::stop_adapter()
{
  ErrCode ret_status = ErrCode::OK;

  DBG_INFO2( "ENTER");

  ret_status = stop_server();

  JA_ERR_OK_PRINT_GO( ret_status, "IpAdapterImplEsp8266::StopAdapter:%d# StopServer FAIL"  );

exit_label_:
  DBG_INFO2( "EXIT");

  return ( ret_status );
}

ErrCode IpAdapterImplEsp8266::start_server()
{
  if( _is_server_started )
  {
    return ( ErrCode::OK );
  }

  ErrCode ret_status = ErrCode::OK;

  DBG_INFO2( "ENTER");

  auto ip_adapter_config = ConfigManager::Inst().get_ip_adapter_config();
  auto u16_port          = ip_adapter_config->get_port( IP_ADAPTER_CONFIG_IPV4_UCAST );

  /* Start IPv4 UNICAST server */
  ret_status = open_socket2( IpAddrFamily::IPv4, false, _ipv4_ucast_socket, u16_port );

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR2( "Failed to open socket IPv4 unicast socket[%p], port[%d]", _ipv4_ucast_socket, u16_port );
    return ( ret_status );
  }

  _ipv4_ucast_socket->set_flags( k_network_flag_ipv4 );
  /* update the currently opened socket port in the config */
  ip_adapter_config->set_current_port( IP_ADAPTER_CONFIG_IPV4_UCAST, u16_port );

  u16_port = ip_adapter_config->get_port( IP_ADAPTER_CONFIG_IPV4_MCAST );

  /* Start MULTICAST server */
  ret_status = open_socket( IpAddrFamily::IPv4, true, _ipv4_mcast_socket, u16_port );

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR2( "Failed to open socket IPv4 multicast socket[%p], port[%d]", _ipv4_mcast_socket, u16_port );
    return ( ret_status );
  }

  _ipv4_mcast_socket->set_flags( k_network_flag_ipv4 | k_network_flag_multicast );

  ret_status = start_listening();

  _receive_task = OsalMgr::Inst()->AllocTask();

  if( _receive_task == nullptr )
  {
    DBG_ERROR2( "failed allocate receive task" );
    return ( ret_status );
  }

  auto osal_status = _receive_task->Init( task_creation_params_t{ IP_ADAPTER_ESP8266_RECEIVE_TASK_NAME, IP_ADAPTER_ESP8266_RECEIVE_TASK_PRIORITY,
                                                                  IP_ADAPTER_ESP8266_RECEIVE_TASK_STACK_SIZE, receive_data_routine, this } );

  if( osal_status != OsalError::OK )
  {
    DBG_ERROR2( "failed init receive task" );
    OsalMgr::Inst()->FreeTask( _receive_task );
    return ( ErrCode::ERR );
  }

  _receive_task->Start();

  _is_server_started = true;

  DBG_INFO2( "EXIT");

  return ( ret_status );
}

ErrCode IpAdapterImplEsp8266::stop_server()
{
  ErrCode ret_status = ErrCode::OK;

  DBG_INFO2( "ENTER");

  /* Stop UNICAST server */
  if( _ipv4_ucast_socket != nullptr )
  {
    DBG_INFO2( "Closing IPv4 ucast socket");
    _ipv4_ucast_socket->CloseSocket();
  }

  /* Stop MULTICAST server */
  if( _ipv4_mcast_socket != nullptr )
  {
    DBG_INFO2( "Closing IPv4 mcast socket");
    _ipv4_mcast_socket->CloseSocket();
  }

  _is_server_started = false;

  DBG_INFO2( "EXIT");

  return ( ret_status );
}

ErrCode IpAdapterImplEsp8266::start_listening()
{
  ErrCode ret_status = ErrCode::OK;

  DBG_INFO2( "ENTER"  );

  const auto ip_adapter_config = ConfigManager::Inst().get_ip_adapter_config();

  if( ip_adapter_config->is_flag_set( IP_ADAPTER_CONFIG_IPV4_MCAST ) )
  {
    IpAddress ipv4_multicast_addr{ 224, 0, 1, 187 };

    if( _ipv4_mcast_socket->JoinMulticastGroup( ipv4_multicast_addr, 0 ) != SocketError::OK )
    {
      ret_status = ErrCode::ERR;
      DBG_ERROR2( " Join Multicast Failed Try:2"  );
    }
  }

  DBG_INFO2( " EXIT"  );

  return ( ret_status );
}

ErrCode IpAdapterImplEsp8266::stop_listening()
{
  ErrCode ret_status = ErrCode::OK;

  DBG_INFO2( "ENTER"  );

  IpAddress ipv4_multicast_addr{ 224, 0, 1, 187 };

  if( _ipv4_mcast_socket->LeaveMulticastGroup( ipv4_multicast_addr, 0 ) != SocketError::OK )
  {
    ret_status = ErrCode::ERR;
    DBG_ERROR2( "LeaveMulticastGroup() FAILED"  );
  }

  DBG_INFO2( "EXIT"  );

  return ( ret_status );
}

int32_t IpAdapterImplEsp8266::send_unicast_data( Endpoint &end_point, const uint8_t *data, uint16_t data_length )
{
  int32_t send_bytes = 0;

  DBG_INFO2( "ENTER"  );

  send_bytes = send_data( end_point, data, data_length, false );

  if( send_bytes < 0 )
  {
    DBG_ERROR2( "send_data() Failed"  );
  }

  DBG_INFO2( "EXIT"  );

  return ( send_bytes );
}

int32_t IpAdapterImplEsp8266::send_multicast_data( Endpoint &end_point, const uint8_t *data, uint16_t data_length )
{
  int32_t send_bytes = 0;

  DBG_INFO2( "ENTER"  );

  send_bytes = send_data( end_point, data, data_length, true );

  if( send_bytes <= 0 )
  {
    DBG_ERROR2( "send_data() Failed"  );
  }

  DBG_INFO2( "EXIT"  );

  return ( send_bytes );
}

void IpAdapterImplEsp8266::send_packet_received_adapter_event( uint8_t *pu8_pdu_buf, int16_t u16_pdu_buf_len, uint16_t u16_remote_port, IpAddress remote_addr, bool is_mcast )
{
  if( this->_adapter_event_cb != nullptr )
  {
    Endpoint end_point{ k_adapter_type_ip, (uint16_t)( ( is_mcast ) ? ( k_network_flag_ipv4 | k_network_flag_multicast ) : k_network_flag_ipv4 ), u16_remote_port, 0, remote_addr };
    AdapterEvent cz_adapter_event{ ADAPTER_EVENT_TYPE_PACKET_RECEIVED, &end_point, pu8_pdu_buf, (uint16_t) u16_pdu_buf_len, k_adapter_type_ip };
    this->_adapter_event_cb( &cz_adapter_event, this->_adapter_event_cb_data );
  }
}

void IpAdapterImplEsp8266::read_data()
{
  // DBG_INFO2( "ENTER"  );

  IpAddress remote_addr{};
  int16_t  data_length   = COAP_MAX_PDU_SIZE;
  uint16_t port          = 0;
  uint8_t  addr_buff[17] = { 0 };

  _ipv4_ucast_socket->ReceiveData( remote_addr, port, &_receive_buffer[0], data_length );

  if( data_length > 0 )
  {
    remote_addr.to_string( addr_buff, sizeof( addr_buff ) );
    DBG_INFO2( "Received unicast from %s port %d data length %d" , addr_buff, port, data_length );
    auto data_buffer = new uint8_t[data_length];
    memcpy( &data_buffer[0], &_receive_buffer[0], data_length );
    send_packet_received_adapter_event( &data_buffer[0], data_length, port, remote_addr, false );
  }

  _ipv4_mcast_socket->ReceiveData( remote_addr, port, &_receive_buffer[0], data_length );

  if( data_length > 0 )
  {
    remote_addr.to_string( addr_buff, sizeof( addr_buff ) );
    DBG_INFO2( "Received multicast from %s port %d data length %d" , addr_buff, port, data_length );
    auto data_buffer = new uint8_t[data_length];
    memcpy( &data_buffer[0], &_receive_buffer[0], data_length );
    send_packet_received_adapter_event( &data_buffer[0], data_length, port, remote_addr, true );
  }

  // DBG_INFO2( "EXIT"  );
}

uint16_t IpAdapterImplEsp8266::get_type()
{
  return ( k_adapter_type_ip );
}

void IpAdapterImplEsp8266::set_adapter_event_cb( const pfn_adapter_event_cb pfn_adapter_event_callback, void *pv_user_data )
{
  _adapter_event_cb      = pfn_adapter_event_callback;
  _adapter_event_cb_data = pv_user_data;
}

ErrCode IpAdapterImplEsp8266::get_endpoints_list( std::deque<Endpoint *> &rcz_endpoint_list )
{
  return ( ErrCode::OK );
}

ErrCode IpAdapterImplEsp8266::open_socket( IpAddrFamily ip_addr_family, bool is_multicast, IUdpSocket *udp_socket, uint16_t port )
{
  IpAddress ip_addr{ ip_addr_family };

  DBG_INFO2( "ENTER family[%d], mcast[%d], port[%d]" , (int) ip_addr_family, is_multicast, port );

  if( udp_socket->OpenSocket( ip_addr_family ) != SocketError::OK )
  {
    DBG_ERROR2( "Failed to open with family[%d]" , (int) ip_addr_family );
    return ( ErrCode::ERR );
  }

  if( udp_socket->EnablePacketInfo( true ) != SocketError::OK )
  {
    DBG_ERROR2( "EnablePacketInfo() FAILED"  );
  }

  udp_socket->SetBlocking( false );

  if( ip_addr_family == IpAddrFamily::IPv6 )
  {
    udp_socket->EnableIpv6Only( true );
    DBG_INFO2( "Calling EnableIpv6Only()"  );
  }

  if( is_multicast && ( port != 0 ) )
  {
    udp_socket->EnableReuseAddr( true );
    DBG_INFO2( "Calling EnableReuseAddr()"  );
  }

  if( udp_socket->BindSocket( ip_addr, port ) != SocketError::OK )
  {
    DBG_ERROR2( "BindSocket FAILED with port[%d]" , port );
    udp_socket->CloseSocket();

    return ( ErrCode::ERR );
  }

  DBG_INFO2( "EXIT"  );

  return ( ErrCode::OK );
}

ErrCode IpAdapterImplEsp8266::open_socket2( IpAddrFamily ip_addr_family, bool is_multicast, IUdpSocket *udp_socket, uint16_t &port )
{
  DBG_INFO2( "ENTER family[%d], mcast[%d], port[%d]" , (int) ip_addr_family, is_multicast, port );
  ErrCode ret_status = open_socket( ip_addr_family, is_multicast, udp_socket, port );

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR2( "Failed to open socket with port[%d], family[%d], mcast[%d]" , port, (int) ip_addr_family, is_multicast );
    DBG_ERROR2( "Retrying with port 0"  );
    ret_status = open_socket( ip_addr_family, is_multicast, udp_socket, 0 );

    if( ret_status != ErrCode::OK )
    {
      DBG_ERROR2( "Failed to open socket after retrying with port[%d], family[%d], mcast[%d]" , port, (int) ip_addr_family, is_multicast );
      return ( ret_status );
    }
  }

  if( ret_status == ErrCode::OK )
  {
    port = udp_socket->GetLocalPort();
    DBG_ERROR2( "Opened socket with port[%d], family[%d], mcast[%d]" , port, (int) ip_addr_family, (int) is_multicast );
  }

  DBG_INFO2( "Successfully opened socket at port[%d]" , port );

  DBG_INFO2( "EXIT"  );

  return ( ret_status );
}

int32_t IpAdapterImplEsp8266::send_data( Endpoint &end_point, const uint8_t *data, uint16_t data_length, bool is_multicast )
{
  int32_t bytes_sent = 0;

  DBG_INFO2( "ENTER data_length[%d], mcast[%d]" , data_length, is_multicast );

  if( data == nullptr )
  {
    DBG_ERROR2( "data is NULL"  );
    bytes_sent = -1; goto exit_label_;
  }

  if( data_length <= 0 )
  {
    DBG_ERROR2( "data length is INVALID %d" , data_length );
    bytes_sent = -1; goto exit_label_;
  }

  if( is_multicast )
  {
    IpAddress ip_addr{ 224, 0, 1, 187 };

    DBG_INFO2( "SelectingMulticastInterface"  );

    if( _ipv4_ucast_socket->SelectMulticastInterface( ip_addr, 1 ) != SocketError::OK )
    {
      DBG_ERROR2( "SelectMulticastInterface() FAILED, if_index %d" , 1 );
      bytes_sent = -1; goto exit_label_;
    }

    DBG_INFO2( "Multicast SendData ip[], port[%d], len[%d]" , end_point.get_port(), data_length );

    if( _ipv4_ucast_socket->SendData( ip_addr, end_point.get_port(), (uint8_t *) data, data_length ) != SocketError::OK )
    {
      DBG_ERROR2( "Multicast SendData() FAILED, if_index %d" , 1 );
      bytes_sent = -1; goto exit_label_;
    }
  }
  else
  {
    // IpAddress ip_addr{};

    // ipaddr_aton( (const char *) end_point.get_addr().get_addr(), (ip_addr_t *) ip_addr.get_addr() );

    DBG_INFO2( "Unicast SendData ip[], port[%d], len[%d]" , end_point.get_port(), data_length );

    if( _ipv4_ucast_socket->SendData( end_point.get_addr(), end_point.get_port(), (uint8_t *) data, data_length ) != SocketError::OK )
    {
      DBG_ERROR2( "Unicast SendData() FAILED, if_index %d" , 1 );
      bytes_sent = -1; goto exit_label_;
    }
  }

exit_label_:
  DBG_INFO2( "EXIT"  );

  return ( bytes_sent );
}

void IpAdapterImplEsp8266::receive_task_loop()
{
  int max_fd            = 0;
  int ipv4_ucast_socket = -1;
  int ipv4_mcast_socket = -1;

  if( _ipv4_ucast_socket != nullptr )
  {
    ipv4_ucast_socket = ( (UdpSocketImplEsp8266 *) _ipv4_ucast_socket )->getSocket();

    if( ipv4_ucast_socket > max_fd )
    {
      max_fd = ipv4_ucast_socket;
    }
  }

  if( _ipv4_mcast_socket != nullptr )
  {
    ipv4_mcast_socket = ( (UdpSocketImplEsp8266 *) _ipv4_mcast_socket )->getSocket();

    if( ipv4_mcast_socket > max_fd )
    {
      max_fd = ipv4_mcast_socket;
    }
  }

  while( true )
  {
    fd_set readFds;
    FD_ZERO( &readFds );

    if( ipv4_ucast_socket != -1 )
    {
      FD_SET( ipv4_ucast_socket, &readFds );
    }

    if( ipv4_mcast_socket != -1 )
    {
      FD_SET( ipv4_mcast_socket, &readFds );
    }

    int select_status = select( max_fd + 1, &readFds, nullptr, nullptr, nullptr );

    if( select_status == 0 )
    {
      continue;
    }
    else if( select_status > 0 )
    {
      while( true )
      {
        IUdpSocket *received_socket = nullptr;

        if( ( ipv4_ucast_socket != -1 ) && FD_ISSET( ipv4_ucast_socket, &readFds ) )
        {
          received_socket = _ipv4_ucast_socket;
        }
        else if( ( ipv4_mcast_socket != -1 ) && FD_ISSET( ipv4_mcast_socket, &readFds ) )
        {
          received_socket = _ipv4_mcast_socket;
        }
        else
        {
          break;
        }

        DBG_INFO2( "received data from socket %d", ( (UdpSocketImplEsp8266 *) received_socket )->getSocket() );
        FD_CLR( ( (UdpSocketImplEsp8266 *) received_socket )->getSocket(), &readFds );

        IpAddress remote_addr{};
        int16_t  data_length   = COAP_MAX_PDU_SIZE;
        uint16_t port          = 0;
        uint8_t  addr_buff[17] = { 0 };

        received_socket->ReceiveData( remote_addr, port, &_receive_buffer[0], data_length );

        if( data_length > 0 )
        {
          remote_addr.to_string( addr_buff, sizeof( addr_buff ) );
          DBG_INFO2( "Received from %s port %d data length %d" , addr_buff, port, data_length );
          auto data_buffer = new uint8_t[data_length];
          memcpy( &data_buffer[0], &_receive_buffer[0], data_length );

          if( this->_adapter_event_cb != nullptr )
          {
            Endpoint end_point{ k_adapter_type_ip, received_socket->get_flags(), port, 0, remote_addr };
            AdapterEvent cz_adapter_event{ ADAPTER_EVENT_TYPE_PACKET_RECEIVED, &end_point, data_buffer, (uint16_t) data_length, k_adapter_type_ip };
            this->_adapter_event_cb( &cz_adapter_event, this->_adapter_event_cb_data );
          }
        }
      }
    }
  }
}

void receive_data_routine( void *arg )
{
  if( arg != nullptr )
  {
    ( (IpAdapterImplEsp8266 *) arg )->receive_task_loop();
  }
}
}  // namespace network
}  // namespace ja_iot

#endif /* _OS_FREERTOS_ */
