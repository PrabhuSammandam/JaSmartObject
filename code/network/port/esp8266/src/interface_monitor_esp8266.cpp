/*
 * InterfaceMonitorImplEsp8266.cpp
 *
 *  Created on: Sep 11, 2017
 *      Author: psammand
 */

#ifdef _OS_FREERTOS_
#include <functional>
#include <esp_common.h>
#include <ErrCode.h>
#include <lwip/sockets.h>
#include <lwip/ipv4/lwip/ip4_addr.h>
#include <port/esp8266/inc/interface_monitor_esp8266.h>

namespace ja_iot {
namespace network {

using namespace ja_iot::base;

InterfaceMonitorImplEsp8266::InterfaceMonitorImplEsp8266 ()
{
}

ErrCode InterfaceMonitorImplEsp8266::start_monitor( uint16_t adapter_type )
{
  return ( ErrCode::OK );
}

ErrCode InterfaceMonitorImplEsp8266::stop_monitor( uint16_t adapter_type )
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
std::vector<InterfaceAddress*> InterfaceMonitorImplEsp8266::get_interface_addr_list(   bool skip_if_down )
{
	std::vector<InterfaceAddress*> if_ptr_array{};

  if( !is_if_addr_valid_ )
  {
    struct ip_info ipv4_addr;

    if( !wifi_get_ip_info( STATION_IF, &ipv4_addr ) )
    {
      return ( if_ptr_array );
    }

    if_addr_.set_family( IpAddrFamily::IPv4 );
    if_addr_.set_index( 1 );
    if_addr_.set_flags( 0 );

    char temp_str[16] = { 0 };

    ipaddr_ntoa_r( (const ip_addr_t *) &ipv4_addr.ip, &temp_str[0], 16 );

    if_addr_.set_addr(IpAddrFamily::IPv4, &temp_str[0] );

    is_if_addr_valid_ = true;
  }

  if( is_if_addr_valid_ )
  {
	  if_ptr_array.push_back(new InterfaceAddress{if_addr_});
//    auto new_if_addr = new InterfaceAddress( if_addr_ );
//    if_ptr_array.Add( new_if_addr );
  }
  return ( if_ptr_array );
}

std::vector<InterfaceAddress *> InterfaceMonitorImplEsp8266::get_newly_found_interface(  )
{
  return std::vector<InterfaceAddress *>();
}

void InterfaceMonitorImplEsp8266::add_interface_event_callback( pfn_interface_monitor_cb cz_if_monitor_cb, void* pv_user_data )
{

}
void InterfaceMonitorImplEsp8266::remove_interface_event_callback( pfn_interface_monitor_cb cz_if_monitor_callback )
{

}
}
}
#endif /* _OS_FREERTOS_ */
