// LightServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "OcfFacade.h"
#include "data_types.h"
#include "OcfPlatformConfig.h"
#include "OsalMgr.h"
#include "OcfDeviceInfo.h"
#include "OcfPlatformInfo.h"

using namespace ja_iot::ocfstack;
using namespace ja_iot::network;
using namespace ja_iot::base;
using namespace std;

int main()
{
  const uint16_t server_flags = k_network_flag_ipv4 | k_network_flag_ipv6;
  const OcfPlatformConfig platform_cfg{ ja_iot::network::eDeviceType::SERVER, server_flags, k_network_flag_none, k_adapter_type_ip };

  auto           &ocf_facade = OcfFacade::inst();

  auto           ret_status = ocf_facade.initialise( platform_cfg );

  if( ret_status != ErrCode::OK )
  {
    cout << "OcfFacde initialise failed " << endl;
  }

  ret_status = ocf_facade.start();

  if( ret_status != ErrCode::OK )
  {
    cout << "OcfFacde start failed " << endl;
  }

  OcfDeviceInfo device_info{ "IoTivity Simple Server", "fa008167-3bbf-4c9d-8604-c9bcb96cb712", "ocf.1.1.0", "ocf.res.1.1.0" };

  ocf_facade.set_device_info( device_info );

  OcfPlatformInfo platform_info{ "0A3E0D6F-DBF5-404E-8719-D6880042463A", "GLT" };

  ocf_facade.set_platform_info( platform_info );

  auto semaphore = ja_iot::osal::OsalMgr::Inst()->alloc_semaphore();

  semaphore->Init( 0, 1 );
  semaphore->Wait();

  return ( 0 );
}