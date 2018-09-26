
#define _GLIBCXX_USE_CXX11_ABI 0

#include <stdio.h>
#include <c_types.h>
#include <esp_system.h>
#include <vector>
#include <string>
#include <set>
#include <forward_list>
#include <list>
#include <unordered_map>
#include <memory>
#include <algorithm>

#include "esp_common.h"
#include <MsgStack.h>
#include <adapter_mgr.h>
#include <base_consts.h>
#include <i_nwk_platform_factory.h>
#include <IMemAllocator.h>
#include <OsalMgr.h>
#include <config_mgr.h>

#include <driver/ja_gpio_led.h>
#include "user/resources/SwitchResource.h"
#include "ResourceMgr.h"
#include "ResPropValue.h"

using namespace ja_iot::base;
using namespace ja_iot::osal;
using namespace ja_iot::network;
using namespace ja_iot::memory;
using namespace ja_iot::stack;

constexpr uint8_t STATUS_LED = 5;
constexpr uint8_t RELAY = 13;


 #define        PSTR( s ) ( __extension__( { static const char __c[] __attribute__( ( section( ".irom0.text" ) ) ) = ( s ); &__c[0]; } ) )
// #define PSTR( s ) s
#define DBG_INFO( format, ... ) printf( PSTR( format "\n" ), ## __VA_ARGS__ )
#define DBG_WARN( format, ... ) printf( PSTR( format "\n" ), ## __VA_ARGS__ )
#define DBG_ERROR( format, ... ) printf( PSTR( format "\n" ), ## __VA_ARGS__ )
#define DBG_FATAL( format, ... ) printf( PSTR( format "\n" ), ## __VA_ARGS__ )

static void wifi_task_cb( System_Event_t *evt );

const char *const FlashSizeMap[] = { "512 KB (256 KB + 256 KB)",      // 0x00
                                     "256 KB",  // 0x01
                                     "1024 KB (512 KB + 512 KB)", // 0x02
                                     "2048 KB (512 KB + 512 KB)", // 0x03
                                     "4096 KB (512 KB + 512 KB)", // 0x04
                                     "2048 KB (1024 KB + 1024 KB)", // 0x05
                                     "4096 KB (1024 KB + 1024 KB)", // 0x06
};

void print_system_info()
{
  printf( PSTR( "==== System info: ====\n" ) );
  printf( PSTR( "SDK version:%s rom %d\n" ), system_get_sdk_version(), system_upgrade_userbin_check() );
  printf( PSTR( "Time = %d\n" ), system_get_time() );
  printf( PSTR( "Chip id = 0x%x\n" ), system_get_chip_id() );
  printf( PSTR( "CPU freq = %d MHz\n" ), system_get_cpu_freq() );
  printf( PSTR( "Flash size map = %s\n" ), FlashSizeMap[system_get_flash_size_map()] );
  printf( PSTR( "Free heap size = %d\n" ), system_get_free_heap_size() );
  printf( PSTR( "==== End System info ====\n" ) );
}

void init_board()
{
  ja_gpio_led_init( STATUS_LED );
  ja_gpio_led_off( STATUS_LED );

//  gpio16_output_conf();
//  gpio16_output_set(0);
  ja_gpio_set_pin_output( RELAY );
  ja_gpio_set_pin_value( RELAY, 0 );
}

extern "C" void user_init( void )
{
  system_update_cpu_freq( SYS_CPU_160MHZ );
  init_board();
  print_system_info();

  auto mem_alloctor = MemAllocatorFactory::create_mem_allocator( MemAlloctorType::kFreeRTOS );

  if( mem_alloctor == nullptr )
  {
    printf( PSTR( "MemAllocatorFactory NULL for FreeRTOS platform\n" ) );
  }

  MemAllocatorFactory::set( mem_alloctor );

  OsalMgr::Inst()->Init();

  auto platform_factory = INetworkPlatformFactory::create_factory( NetworkPlatform::kEsp8266 );

  if( platform_factory == nullptr )
  {
    printf( PSTR( "INetworkPlatformFactory NULL for FreeRTOS platform\n" ) );
  }

  INetworkPlatformFactory::set_curr_factory( platform_factory );

  ConfigManager::Inst().get_ip_adapter_config()->set_port( IP_ADAPTER_CONFIG_IPV4_UCAST, 56775 );
  ConfigManager::Inst().get_ip_adapter_config()->set_flag( IP_ADAPTER_CONFIG_IPV4_UCAST, true );
  ConfigManager::Inst().get_ip_adapter_config()->set_flag( IP_ADAPTER_CONFIG_IPV4_MCAST, true );

  wifi_set_event_handler_cb( wifi_task_cb );
  wifi_set_opmode( STATION_MODE );
  struct station_config config;
  bzero( &config, sizeof( struct station_config ) );
  sprintf( (char *) config.ssid, (char *) "JinjuAmla" );
  sprintf( (char *) config.password, (char *) "Jinju124Amla" );
  wifi_station_set_config( &config );

  wifi_station_connect();
}


static void wifi_task_cb( System_Event_t *evt )
{
  DBG_INFO( "wifi_task_cb:%d# Callback event[%d]", __LINE__, evt->event_id );

  switch( evt->event_id )
  {
    case EVENT_STAMODE_SCAN_DONE:
    {
      DBG_INFO( "wifi_task_cb:%d# scan_done", __LINE__ );
    }
    break;

    case EVENT_STAMODE_CONNECTED:
    {
      DBG_INFO( "wifi_task_cb:%d# connected to ssid [%s], channel [%d]", __LINE__, evt->event_info.connected.ssid, evt->event_info.connected.channel );
    }
    break;

    case EVENT_STAMODE_GOT_IP:
    {
      DBG_INFO( "wifi_task_cb:%d# got_ip addr:%d.%d.%d.%d, mask:%d.%d.%d.%d, gw:%d.%d.%d.%d", __LINE__
              , IP2STR( &evt->event_info.got_ip.ip )
              , IP2STR( &evt->event_info.got_ip.mask )
              , IP2STR( &evt->event_info.got_ip.gw ) );

      ja_gpio_led_on( STATUS_LED );
      MsgStack::inst().initialize( k_adapter_type_ip );

      auto switch_resource = new SwitchResource{};
      ResourceMgr::inst().add_resource( switch_resource );
    }
    break;

    case EVENT_STAMODE_AUTHMODE_CHANGE:
    case EVENT_STAMODE_DISCONNECTED:
    case EVENT_SOFTAPMODE_STACONNECTED:
    case EVENT_SOFTAPMODE_STADISCONNECTED:
    {
    }
    break;

    default:
    {
    }
    break;
  }
}
