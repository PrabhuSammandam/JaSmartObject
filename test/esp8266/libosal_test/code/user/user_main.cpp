#include <string>
#include <functional>
#include <algorithm>
#include <vector>
#include <c_types.h>
#include <esp_misc.h>
#include <esp_system.h>
#include <OsalMgr.h>
#include <stdio.h>
#include <wifi_task.h>

using namespace ja_iot::osal;

const char *const FlashSizeMap[] = { "512 KB (256 KB + 256 KB)",     // 0x00
                                     "256 KB",                       // 0x01
                                     "1024 KB (512 KB + 512 KB)",    // 0x02
                                     "2048 KB (512 KB + 512 KB)",     // 0x03
                                     "4096 KB (512 KB + 512 KB)",     // 0x04
                                     "2048 KB (1024 KB + 1024 KB)",   // 0x05
                                     "4096 KB (1024 KB + 1024 KB)",   // 0x06
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
  printf( "==== System info: ====\n" );
  printf( "SDK version:%s rom %d\n", system_get_sdk_version(), system_upgrade_userbin_check() );
  printf( "Time = %d\n", system_get_time() );
  printf( "Chip id = 0x%x\n", system_get_chip_id() );
  printf( "CPU freq = %d MHz\n", system_get_cpu_freq() );
  printf( "Flash size map = %s\n", FlashSizeMap[system_get_flash_size_map()] );
  printf( "Free heap size = %d\n", system_get_free_heap_size() );
  printf( "==== End System info ====\n" );
}


/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
 *******************************************************************************/

WifiTask gs_wifi_task;

#include <experimental/optional>

typedef std::function<void ( const int *interface_event )> InterfaceMonitorCallback;

void test_function()
{
  InterfaceMonitorCallback callback;
  int                      sum = 0;
  std::string str("test");

  std::vector<int> int_vector{};

  int_vector.push_back( 10 );

  std::for_each( int_vector.cbegin(), int_vector.cend(), [&] ( int i ) {
    sum += i;
  } );
}

extern "C" void user_init( void )
{
  test_function();

  OsalMgr::Inst()->Init();
  print_system_info();

  for( int var = 0; var < 1000; ++var )
  {
    os_delay_us( 1000 );
  }

  gs_wifi_task.create_task();
}
