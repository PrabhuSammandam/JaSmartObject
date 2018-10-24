#include <base_consts.h>
#include <ip_adapter_config.h>
#include <OsalError.h>
#include <Sem.h>
#include <cstdint>
#include <iostream>
#include <stdio.h>

#define DBG_INFO( format, ... ) printf( format "\n", ## __VA_ARGS__ )
#define DBG_WARN( format, ... ) printf( format "\n", ## __VA_ARGS__ )
#define DBG_ERROR( format, ... ) printf( format "\n", ## __VA_ARGS__ )
#define DBG_FATAL( format, ... ) printf( format "\n", ## __VA_ARGS__ )

#include "IMemAllocator.h"
#include "OsalMgr.h"
#include "i_nwk_platform_factory.h"
#include "config_mgr.h"
#include "adapter_mgr.h"
#include <MsgStack.h>
#include "ResourceMgr.h"
#include "BinarySwitchResource.h"

using namespace std;
using namespace ja_iot::base;
using namespace ja_iot::memory;
using namespace ja_iot::network;
using namespace ja_iot::osal;

void init_adapter_mgr()
{
  if( MemAllocatorFactory::create_set_mem_allocator( MemAlloctorType::kLinux ) == nullptr )
  {
    DBG_ERROR( "main:%d# Failed to allocate the mem allocator", __LINE__ );
  }

  OsalMgr::Inst()->Init();

  if( INetworkPlatformFactory::create_set_factory( NetworkPlatform::kLinux ) == nullptr )
  {
    DBG_ERROR( "main:%d# INetworkPlatformFactory NULL for WINDOWS platform", __LINE__ );
  }

  auto ip_adapter_config = ConfigManager::Inst().get_ip_adapter_config();

  ip_adapter_config->set_port( IP_ADAPTER_CONFIG_IPV4_UCAST, 56775 );

  ip_adapter_config->set_flag( IP_ADAPTER_CONFIG_IPV4_UCAST, true );
  ip_adapter_config->set_flag( IP_ADAPTER_CONFIG_IPV4_MCAST, true );
  // ip_adapter_config->set_flag(IP_ADAPTER_CONFIG_IPV4_UCAST_SECURE, true);
  // ip_adapter_config->set_flag(IP_ADAPTER_CONFIG_IPV4_MCAST_SECURE, true);

  ip_adapter_config->set_flag( IP_ADAPTER_CONFIG_IPV6_UCAST, true );
  ip_adapter_config->set_flag( IP_ADAPTER_CONFIG_IPV6_MCAST, true );
  // ip_adapter_config->set_flag(IP_ADAPTER_CONFIG_IPV6_UCAST_SECURE, true);
  // ip_adapter_config->set_flag(IP_ADAPTER_CONFIG_IPV6_MCAST_SECURE, true);
}

void init_resources()
{
  auto switch_res = new BinarySwitchResource{};

  ResourceMgr::inst().add_resource( switch_res );
}

int main( int argc, char *argv[] )
{
  init_adapter_mgr();

  MsgStack::inst().initialize( k_adapter_type_ip );

  init_resources();

  auto semaphore = OsalMgr::Inst()->alloc_semaphore();

  DBG_INFO( "main:%d# Going to wait", __LINE__ );

  semaphore->Init( 0, 1 );
  semaphore->Wait();

  DBG_INFO( "main:%d# EXIT", __LINE__ );

  return ( 0 );
}