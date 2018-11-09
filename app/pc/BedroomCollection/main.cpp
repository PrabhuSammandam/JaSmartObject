#include <base_consts.h>
#include <OsalError.h>
#include <Sem.h>
#include <cstdint>
#include <iostream>
#include <stdio.h>
#include "CollectionResource.h"
#include "BinarySwitchResourceBase.h"

#define __FILE_NAME__ "main.cpp"

#ifdef __GNUC__
#define DBG_INFO( format, ... ) printf( "INF:%s::%s:%d# " format "\n", __FILE_NAME__, __FUNCTION__, __LINE__, ## __VA_ARGS__ )
#define DBG_WARN( format, ... ) printf( "WRN:%s::%s:%d# " format "\n", __FILE_NAME__, __FUNCTION__, __LINE__, ## __VA_ARGS__ )
#define DBG_ERROR( format, ... ) printf( "ERR:%s::%s:%d# " format "\n", __FILE_NAME__, __FUNCTION__, __LINE__, ## __VA_ARGS__ )
#define DBG_FATAL( format, ... ) printf( "FTL:%s::%s:%d# " format "\n", __FILE_NAME__, __FUNCTION__, __LINE__, ## __VA_ARGS__ )
#else
#define DBG_INFO( format, ... ) printf( "INF:%s::%s:%d# " format "\n", __FILE_NAME__, __func__, __LINE__, ## __VA_ARGS__ )
#define DBG_WARN( format, ... ) printf( "WRN:%s::%s:%d# " format "\n", __FILE_NAME__, __func__, __LINE__, ## __VA_ARGS__ )
#define DBG_ERROR( format, ... ) printf( "ERR:%s::%s:%d# " format "\n", __FILE_NAME__, __func__, __LINE__, ## __VA_ARGS__ )
#define DBG_FATAL( format, ... ) printf( "FTL:%s::%s:%d# " format "\n", __FILE_NAME__, __func__, __LINE__, ## __VA_ARGS__ )
#endif

#include "OsalMgr.h"
#include "i_nwk_platform_factory.h"
#include "config_mgr.h"
#include <MsgStack.h>
#include "ResourceMgr.h"
#include "SoStackFacade.h"

using namespace std;
using namespace ja_iot::base;
using namespace ja_iot::network;
using namespace ja_iot::osal;
using namespace ja_iot::so_stack;
using namespace ja_iot::resources;

typedef void ( *handle_switch_callback ) ( std::string &switch_uri );

class Switch : public BinarySwitchResourceBase
{
  public:
    Switch( std::string uri, handle_switch_callback callback ) : BinarySwitchResourceBase( uri ), _callback{ callback }
    {
    }

    void handle_property_change() override
    {
      if( _callback != nullptr )
      {
        cout << "Switch " << get_uri() << " actuated" << endl;
        _callback( get_uri() );
      }
    }

    handle_switch_callback _callback{};
};

Switch *fan_switch{};
Switch *light_switch_north{};
Switch *light_switch_south{};
Switch *light_switch_east{};

void handle_switch_cb( std::string &switch_uri )
{
}

void init_adapter_mgr()
{
  auto &so_stack_facade = SoStackFacade::inst();

  if( so_stack_facade.initialize() != ErrCode::OK )
  {
    DBG_ERROR( "Failed to initialize STACK" );
    return;
  }

  so_stack_facade.enable_ipv4( true )
  .enable_ipv4_mcast( true )
  .enable_ipv6( true )
  .enable_ipv6_mcast( true );

  DeviceInfo device_info{ "BedroomCollection", "aa", "ocf.1.0.0", "ocf.res.1.3.0" };
  so_stack_facade.set_device_info( device_info );

  so_stack_facade.start();
}

void init_resources()
{
  Switch *fan_switch         = new Switch{ "/sw/fan", handle_switch_cb };
  Switch *light_switch_north = new Switch{ "/sw/light/north", handle_switch_cb };
  Switch *light_switch_south = new Switch{ "/sw/light/south", handle_switch_cb };
  Switch *light_switch_east  = new Switch{ "/sw/light/east", handle_switch_cb };

  auto    &res_mgr = ResourceMgr::inst();

  res_mgr.add_resource( new CollectionResource{} );
  res_mgr.add_resource( fan_switch );
  res_mgr.add_resource( light_switch_north );
  res_mgr.add_resource( light_switch_south );
  res_mgr.add_resource( light_switch_east );
}

int main( int argc, char *argv[] )
{
  init_adapter_mgr();

  init_resources();

  auto semaphore = OsalMgr::Inst()->alloc_semaphore();

  DBG_INFO( "Going to wait" );

  semaphore->Init( 0, 1 );
  semaphore->Wait();

  DBG_INFO( "EXIT" );

  return ( 0 );
}