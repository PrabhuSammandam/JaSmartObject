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
#include "SoStackFacade.h"
#include "BinarySwitchResourceBase.h"

using namespace std;
using namespace ja_iot::base;
using namespace ja_iot::memory;
using namespace ja_iot::network;
using namespace ja_iot::osal;
using namespace ja_iot::so_stack;
using namespace ja_iot::resources;

class MySwitch : public BinarySwitchResourceBase
{
  public:
    MySwitch () : BinarySwitchResourceBase( "/light/1" ) {}

    void handle_property_change()
    {
      cout << "switch actuated" << endl;
    }
};

void init_adapter_mgr()
{
  auto &so_stack_facade = SoStackFacade::inst();

  if( so_stack_facade.initialize() != ErrCode::OK )
  {
    return;
  }

  so_stack_facade.enable_ipv4( true )
  .enable_ipv4_mcast( true )
		.enable_ipv4_secure(true)
		.enable_ipv4_mcast_secure(true)
  .enable_ipv6( true )
  .enable_ipv6_mcast( true )
		.enable_ipv6_secure(true)
		.enable_ipv6_mcast_secure(true);

  DeviceInfo device_info{ "SimpleClient", "aa", "ocf.1.0.0", "ocf.res.1.3.0" };
  so_stack_facade.set_device_info( device_info );

  so_stack_facade.start();
}

void init_resources()
{
  auto switch_res = new MySwitch{};

  ResourceMgr::inst().add_resource( switch_res );
}

int main( int argc, char *argv[] )
{
  init_adapter_mgr();

  init_resources();

  auto semaphore = OsalMgr::Inst()->alloc_semaphore();

  DBG_INFO( "main:%d# Going to wait", __LINE__ );

  semaphore->Init( 0, 1 );
  semaphore->Wait();

  DBG_INFO( "main:%d# EXIT", __LINE__ );

  return ( 0 );
}
