#include <stdio.h>
#include <iostream>

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

using namespace std;
using namespace ja_iot::base;
using namespace ja_iot::memory;
using namespace ja_iot::network;
using namespace ja_iot::osal;

void adapter_mgr_pkt_recvd_cb( void *pv_user_data, Endpoint const &end_point, const uint8_t *data, uint16_t data_len )
{
  cout << "received packet of length " << data_len << endl;
}
void init_adapter_mgr()
{
  const auto mem_allocator = MemAllocatorFactory::create_mem_allocator( MemAlloctorType::kWindows );

  if( mem_allocator == nullptr )
  {
    DBG_ERROR( "main:%d# Failed to allocate the mem allocator", __LINE__ );
  }

  MemAllocatorFactory::set( mem_allocator );

  OsalMgr::Inst()->Init();

  const auto platform_factory = INetworkPlatformFactory::create_factory( NetworkPlatform::kWindows );

  if( platform_factory == nullptr )
  {
    DBG_ERROR( "main:%d# INetworkPlatformFactory NULL for WINDOWS platform", __LINE__ );
  }

  INetworkPlatformFactory::set_curr_factory( platform_factory );

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

int main( int argc, char *argv[] )
{
	printf("Starting");
  init_adapter_mgr();

  MsgStack::inst().initialize( k_adapter_type_ip );

  auto semaphore = OsalMgr::Inst()->alloc_semaphore();

  DBG_INFO( "main:%d# Going to wait", __LINE__ );

  semaphore->Init( 0, 1 );
  semaphore->Wait();

  DBG_INFO( "main:%d# EXIT", __LINE__ );

  return ( 0 );
}
