/*
 * main.cpp
 *
 *  Created on: Sep 15, 2017
 *      Author: psammand
 */
#include <AdapterMgr.h>
#include <INetworkPlatformFactory.h>
#include <OsalMgr.h>
#include <IMemAllocator.h>

#define DBG_INFO( format, ... ) printf( format "\n", ## __VA_ARGS__ )
#define DBG_WARN( format, ... ) printf( format "\n", ## __VA_ARGS__ )
#define DBG_ERROR( format, ... ) printf( format "\n", ## __VA_ARGS__ )
#define DBG_FATAL( format, ... ) printf( format "\n", ## __VA_ARGS__ )

using namespace ja_iot::base;
using namespace ja_iot::memory;
using namespace ja_iot::network;
using namespace ja_iot::osal;

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

  auto ip_adapter_config = AdapterManager::Inst().get_ip_adapter_config();

  ip_adapter_config->setClientNetworkFlag( NetworkFlag::IPV4 | NetworkFlag::IPV6 | NetworkFlag::MULTICAST );
  ip_adapter_config->set_config_flag(IpAdapterConfigFlag::IPV4_UCAST_ENABLED, true);
  ip_adapter_config->set_config_flag(IpAdapterConfigFlag::IPV4_MCAST_ENABLED, true);
  ip_adapter_config->set_config_flag(IpAdapterConfigFlag::IPV4_UCAST_SECURE_ENABLED, true);
  ip_adapter_config->set_config_flag(IpAdapterConfigFlag::IPV4_MCAST_SECURE_ENABLED, true);

  ip_adapter_config->set_config_flag(IpAdapterConfigFlag::IPV6_UCAST_ENABLED, true);
  ip_adapter_config->set_config_flag(IpAdapterConfigFlag::IPV6_MCAST_ENABLED, true);
  ip_adapter_config->set_config_flag(IpAdapterConfigFlag::IPV6_UCAST_SECURE_ENABLED, true);
  ip_adapter_config->set_config_flag(IpAdapterConfigFlag::IPV6_MCAST_SECURE_ENABLED, true);

  AdapterManager::Inst().SetPlatformFactory( platform_factory );
  AdapterManager::Inst().InitializeAdapters( (uint16_t) AdapterType::IP );
  AdapterManager::Inst().StartAdapter( AdapterType::IP );

  auto semaphore = OsalMgr::Inst()->alloc_semaphore();

  DBG_INFO( "main:%d# Going to wait", __LINE__ );

  semaphore->Init( 0, 1 );
  semaphore->Wait();

  DBG_INFO( "main:%d# EXIT", __LINE__ );

  return ( 0 );
}
