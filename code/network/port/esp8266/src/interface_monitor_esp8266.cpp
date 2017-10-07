/*
 * InterfaceMonitorImplEsp8266.cpp
 *
 *  Created on: Sep 11, 2017
 *      Author: psammand
 */

#ifdef _OS_FREERTOS_

#include <esp_common.h>
#include <lwip/sockets.h>
#include <lwip/ipv4/lwip/ip4_addr.h>
#include <port/esp8266/inc/interface_monitor_esp8266.h>

namespace ja_iot {
namespace network {
InterfaceMonitorImplEsp8266::InterfaceMonitorImplEsp8266 ()
{
}

ErrCode InterfaceMonitorImplEsp8266::StartMonitor( uint16_t adapter_type )
{
  return ( ErrCode::OK );
}

ErrCode InterfaceMonitorImplEsp8266::StopMonitor( uint16_t adapter_type )
{
  return ( ErrCode::OK );
}

/***
 * This function add all the found interface address to the passed array. In ESP8266 there are
 * no more than one interface, then it is simple to return always the previously found interface
 * address.
 *
 * @param if_ptr_array	-	list to hold all the interface address
 * @param skip_if_down	-	flag to indicate whether to include the interface which is DOWN.
 * @return ErrCode::OK	-	on success
 */
ErrCode InterfaceMonitorImplEsp8266::GetInterfaceAddrList( InterfaceAddressPtrArray &if_ptr_array, bool skip_if_down )
{
  if_ptr_array.Clear();

  if( !is_if_addr_valid_ )
  {
    struct ip_info ipv4_addr;

    if( !wifi_get_ip_info( STATION_IF, &ipv4_addr ) )
    {
      return ( ErrCode::ERR );
    }

    if_addr_.setFamily( IpAddrFamily::IPV4 );
    if_addr_.setIndex( 1 );
    if_addr_.setFlags( 0 );

    char temp_str[16] = { 0 };

    ipaddr_ntoa_r( (const ip_addr_t *) &ipv4_addr.ip, &temp_str[0], 16 );

    if_addr_.set_addr( &temp_str[0] );

    is_if_addr_valid_ = true;
  }

  if( is_if_addr_valid_ )
  {
    auto new_if_addr = new InterfaceAddress( if_addr_ );
    if_ptr_array.Add( new_if_addr );
  }

  return ( ErrCode::OK );
}

ErrCode InterfaceMonitorImplEsp8266::GetNewlyFoundInterface( InterfaceAddressPtrArray &if_addr_ptr_array )
{
	return ( ErrCode::OK );
}

void InterfaceMonitorImplEsp8266::AddInterfaceEventHandler( IInterfaceEventHandler *interface_event_handler )
{
}

void InterfaceMonitorImplEsp8266::RemoveInterfaceEventHandler( IInterfaceEventHandler *interface_event_handler )
{
}
}
}
#endif /* _OS_FREERTOS_ */
