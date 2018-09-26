/*
 * main.cpp
 *
 *  Created on: Sep 15, 2017
 *      Author: psammand
 */
#include <vector>
#include <iostream>
#include <base_consts.h>
#include <config_mgr.h>
#include <connectivity_mgr.h>
#include <i_nwk_platform_factory.h>
#include <ip_adapter_config.h>
#include <IMemAllocator.h>
#include <OsalError.h>
#include <OsalMgr.h>
#include <Sem.h>
#include <cstdio>
#include <unistd.h>
#include "end_point.h"
#include "adapter_mgr.h"

#define DBG_INFO( format, ... ) printf( format "\n", ## __VA_ARGS__ )
#define DBG_WARN( format, ... ) printf( format "\n", ## __VA_ARGS__ )
#define DBG_ERROR( format, ... ) printf( format "\n", ## __VA_ARGS__ )
#define DBG_FATAL( format, ... ) printf( format "\n", ## __VA_ARGS__ )

using namespace ja_iot::base;
using namespace ja_iot::memory;
using namespace ja_iot::network;
using namespace ja_iot::osal;

/***
 * The stack initialization sequence is as follows
 * 1. Create the memory factory and assign the memory factory to the memory manager.
 * 2. Create the network platform and assign it as current network platform factory.
 * 3. Configure the adapter manager for the adapter type ,type of device, addressing type and secured communication.
 * @return
 */

void packet_received_callback( void *pv_user_data, Endpoint const &end_point, const uint8_t *data, uint16_t data_len )
{
  printf( "received packet\n" );
}

int main()
{
  NetworkPlatform e_network_platform   = NetworkPlatform::kWindows;
  MemAlloctorType e_mem_allocator_type = MemAlloctorType::kWindows;

#ifdef _OS_LINUX_
  e_network_platform   = NetworkPlatform::kLinux;
  e_mem_allocator_type = MemAlloctorType::kLinux
#endif /* _OS_LINUX_ */

#ifdef _OS_FREERTOS_
  e_network_platform   = NetworkPlatform::kEsp8266;
  e_mem_allocator_type = MemAlloctorType::kFreeRTOS
#endif /* _OS_FREERTOS_ */

  auto mem_allocator = MemAllocatorFactory::create_mem_allocator( e_mem_allocator_type );

  if( mem_allocator == nullptr )
  {
    DBG_ERROR( "main:%d# Failed to allocate the mem allocator", __LINE__ );
  }

  MemAllocatorFactory::set( mem_allocator );

  OsalMgr::Inst()->Init();

  auto platform_factory = INetworkPlatformFactory::create_factory( e_network_platform );

  if( platform_factory == nullptr )
  {
    DBG_ERROR( "main:%d# INetworkPlatformFactory NULL for WINDOWS platform", __LINE__ );
  }

  INetworkPlatformFactory::set_curr_factory( platform_factory );

  auto ip_adapter_config = ConfigManager::Inst().get_ip_adapter_config();

  ip_adapter_config->set_port( IP_ADAPTER_CONFIG_IPV4_UCAST, 56775 );

  ip_adapter_config->set_flag( IP_ADAPTER_CONFIG_IPV4_UCAST, true );
  // ip_adapter_config->set_flag( IP_ADAPTER_CONFIG_IPV4_MCAST, true );
  // ip_adapter_config->set_flag(IP_ADAPTER_CONFIG_IPV4_UCAST_SECURE, true);
  // ip_adapter_config->set_flag(IP_ADAPTER_CONFIG_IPV4_MCAST_SECURE, true);

  // ip_adapter_config->set_flag( IP_ADAPTER_CONFIG_IPV6_UCAST, true );
  // ip_adapter_config->set_flag( IP_ADAPTER_CONFIG_IPV6_MCAST, true );
  // ip_adapter_config->set_flag(IP_ADAPTER_CONFIG_IPV6_UCAST_SECURE, true);
  // ip_adapter_config->set_flag(IP_ADAPTER_CONFIG_IPV6_MCAST_SECURE, true);

  AdapterManager::Inst().set_packet_received_cb( packet_received_callback, nullptr );
  auto ret_status = AdapterManager::Inst().initialize_adapters( k_adapter_type_ip );
  ret_status = AdapterManager::Inst().start_adapter( k_adapter_type_ip );
  ret_status = AdapterManager::Inst().start_servers();

  auto semaphore = OsalMgr::Inst()->alloc_semaphore();

  DBG_INFO( "main:%d# Going to wait", __LINE__ );

  semaphore->Init( 0, 1 );
  semaphore->Wait();

  DBG_INFO( "main:%d# EXIT", __LINE__ );

  return ( 0 );
}
