/*
 * main.cpp
 *
 *  Created on: Sep 15, 2017
 *      Author: psammand
 */

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

#define DBG_INFO( format, ... ) printf( format "\n", ## __VA_ARGS__ )
#define DBG_WARN( format, ... ) printf( format "\n", ## __VA_ARGS__ )
#define DBG_ERROR( format, ... ) printf( format "\n", ## __VA_ARGS__ )
#define DBG_FATAL( format, ... ) printf( format "\n", ## __VA_ARGS__ )

using namespace ja_iot::base;
using namespace ja_iot::memory;
using namespace ja_iot::network;
using namespace ja_iot::osal;

/***
 * The stack initialization is sequence is as follows
 * 1. Create the memory factory and assign the memory factory to the memory manager.
 * 2. Create the network platform and assign it as current network platform factory.
 * 3. Configure the adapter manager for the adapter type ,type of device, addressing type and secured communication.
 * @return
 */

class MyPacketEventHandler : public PacketEventHandler
{
public:
	MyPacketEventHandler(){}
	virtual ~MyPacketEventHandler(){}

	virtual void handle_packet_event( PacketEvent *pcz_packet_event ) override
	{
		printf("Received packet, type[%d]\n", (int)pcz_packet_event->get_packet()->get_packet_type());
	}
};

int main()
{
  auto mem_allocator = MemAllocatorFactory::create_mem_allocator( MemAlloctorType::kWindows );

  if( mem_allocator == nullptr )
  {
    DBG_ERROR( "main:%d# Failed to allocate the mem allocator", __LINE__ );
  }

  MemAllocatorFactory::set( mem_allocator );

  OsalMgr::Inst()->Init();

  auto platform_factory = INetworkPlatformFactory::CreateFactory( NetworkPlatform::kWindows );

  if( platform_factory == nullptr )
  {
    DBG_ERROR( "main:%d# INetworkPlatformFactory NULL for WINDOWS platform", __LINE__ );
  }

  INetworkPlatformFactory::SetCurrFactory( platform_factory );

  auto ip_adapter_config = ConfigManager::Inst().get_ip_adapter_config();

  ip_adapter_config->set_config_flag( IpAdapterConfigFlag::IPV4_UCAST_ENABLED, true );
  ip_adapter_config->set_config_flag(IpAdapterConfigFlag::IPV4_MCAST_ENABLED, true);
  // ip_adapter_config->set_config_flag(IpAdapterConfigFlag::IPV4_UCAST_SECURE_ENABLED, true);
  // ip_adapter_config->set_config_flag(IpAdapterConfigFlag::IPV4_MCAST_SECURE_ENABLED, true);

  // ip_adapter_config->set_config_flag(IpAdapterConfigFlag::IPV6_UCAST_ENABLED, true);
  // ip_adapter_config->set_config_flag(IpAdapterConfigFlag::IPV6_MCAST_ENABLED, true);
  // ip_adapter_config->set_config_flag(IpAdapterConfigFlag::IPV6_UCAST_SECURE_ENABLED, true);
  // ip_adapter_config->set_config_flag(IpAdapterConfigFlag::IPV6_MCAST_SECURE_ENABLED, true);

  MyPacketEventHandler cz_packet_handler;
  ConnectivityManager::Inst().initialize(kAdapterType_ip);
  ConnectivityManager::Inst().set_packet_event_handler(&cz_packet_handler);
  ConnectivityManager::Inst().select_network(kAdapterType_ip);
  ConnectivityManager::Inst().start_listening_server();

  while(1)
  {
	  ConnectivityManager::Inst().handle_request_response();

	  sleep(1);
  }

  auto semaphore = OsalMgr::Inst()->alloc_semaphore();

  DBG_INFO( "main:%d# Going to wait", __LINE__ );

  semaphore->Init( 0, 1 );
  semaphore->Wait();

  DBG_INFO( "main:%d# EXIT", __LINE__ );

  return ( 0 );
}
