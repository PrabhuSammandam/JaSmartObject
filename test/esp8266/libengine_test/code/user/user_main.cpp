#define _GLIBCXX_USE_CXX11_ABI 0

#include <stdio.h>
#include <c_types.h>
#include <esp_system.h>
#include <vector>
#include <string>
#include <set>
#include <deque>
#include <forward_list>
#include <list>
#include <memory>
#include <algorithm>
#include <chrono>

#include <adapter_mgr.h>
#include <base_consts.h>
#include <i_nwk_platform_factory.h>
#include <IMemAllocator.h>
#include <OsalMgr.h>
#include <connectivity_mgr.h>
#include <config_mgr.h>
#include "payload/ResPropValue.h"

using namespace ja_iot::base;
using namespace ja_iot::osal;
using namespace ja_iot::network;
using namespace ja_iot::memory;

 #define	PSTR(s)	(__extension__({static	const	char	__c[] __attribute__((section(".irom0.text"))) =	(s);	&__c[0];}))
//#define PSTR( s ) s

const char *const FlashSizeMap[] = { "512 KB (256 KB + 256 KB)",      // 0x00
                                     "256 KB",  // 0x01
                                     "1024 KB (512 KB + 512 KB)", // 0x02
                                     "2048 KB (512 KB + 512 KB)", // 0x03
                                     "4096 KB (512 KB + 512 KB)", // 0x04
                                     "2048 KB (1024 KB + 1024 KB)", // 0x05
                                     "4096 KB (1024 KB + 1024 KB)", // 0x06
};

/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
 *******************************************************************************/
extern "C" uint32 user_rf_cal_sector_set( void )
{
  flash_size_map size_map   = system_get_flash_size_map();
  uint32         rf_cal_sec = 0;

  switch( size_map )
  {
    case FLASH_SIZE_4M_MAP_256_256:
    {
      rf_cal_sec = 128 - 5;
    }
    break;

    case FLASH_SIZE_8M_MAP_512_512:
    {
      rf_cal_sec = 256 - 5;
    }
    break;

    case FLASH_SIZE_16M_MAP_512_512:
    case FLASH_SIZE_16M_MAP_1024_1024:
    {
      rf_cal_sec = 512 - 5;
    }
    break;

    case FLASH_SIZE_32M_MAP_512_512:
    case FLASH_SIZE_32M_MAP_1024_1024:
    {
      rf_cal_sec = 1024 - 5;
    }
    break;
    case FLASH_SIZE_64M_MAP_1024_1024:
    {
      rf_cal_sec = 2048 - 5;
    }
    break;
    case FLASH_SIZE_128M_MAP_1024_1024:
    {
      rf_cal_sec = 4096 - 5;
    }
    break;
    default:
    {
      rf_cal_sec = 0;
    }
    break;
  }

  return ( rf_cal_sec );
}

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

/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
 *******************************************************************************/

void test_res_prop_value_bool()
{
  /* value constructor */
  ResPropValue v1{ false };

  /* copy constructor */
  ResPropValue v2{ v1 };

  /* move constructor */
  ResPropValue v3{ std::move(v2) };

  /* get */
//  auto val = v3.get<bool>();
//
//  printf("val %d\n", val);

  /* set */
  v3.set(true);

  /* move value */

  ResPropValue v4{};
  v4.move<bool>(false);
  /* destroy */
}


void test_res_prop()
{
	test_res_prop_value_bool();

	printf("string size = %d\n", sizeof(std::string));

	printf("res_prop_value size = %d\n", sizeof(ResPropValue));// 48
	printf("prop_union size = %d\n", sizeof(PropUnion));// 40
	printf("res_props size = %d\n", sizeof(ResProps));// 24

	printf("vector_string_list size = %d\n", sizeof(std::vector<std::string>));// 12
	printf("de_queue_string_list size = %d\n", sizeof(std::deque<std::string>));// 40
	printf("forward_list_string_list size = %d\n", sizeof(std::forward_list<std::string>));// 4
	printf("list_string_list size = %d\n", sizeof(std::list<std::string>));// 8
	std::weak_ptr<ResPropValue> res_prop;
}

extern "C" void user_init( void )
{
  print_system_info();

  test_res_prop();

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

  ConfigManager::Inst().get_ip_adapter_config()->set_flag(IP_ADAPTER_CONFIG_IPV4_UCAST, true);
  ConfigManager::Inst().get_ip_adapter_config()->set_flag(IP_ADAPTER_CONFIG_IPV4_MCAST, true);

  ConnectivityMgr::inst().initialize( k_adapter_type_ip );

  std::chrono::nanoseconds secs;

  secs++;

  std::chrono::system_clock::now();
  std::chrono::steady_clock::time_point time_pt;

}
