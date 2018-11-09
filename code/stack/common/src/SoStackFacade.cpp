/*
 * SoStackFacade.cpp
 *
 *  Created on: Oct 30, 2018
 *      Author: psammand
 */

#include "SoStackFacade.h"
#include "IMemAllocator.h"
#include "OsalMgr.h"
#include "i_nwk_platform_factory.h"
#include "config_mgr.h"
#include "adapter_mgr.h"
#include "MsgStack.h"
#include "config_mgr.h"
#include "ResourceMgr.h"
#include "common/inc/DeviceResource.h"

#define __FILE_NAME__ "SoStackFacade"

#ifdef __GNUC__
#define DEBUG_FUNCTION_LABEL __FUNCTION__
#else
#define DEBUG_FUNCTION_LABEL __func__
#endif

#define DBG_INFO( format, ... ) printf( "INF:%s::%s:%d# " format "\n", __FILE_NAME__, DEBUG_FUNCTION_LABEL, __LINE__, ## __VA_ARGS__ )
#define DBG_WARN( format, ... ) printf( "WRN:%s::%s:%d# " format "\n", __FILE_NAME__, DEBUG_FUNCTION_LABEL, __LINE__, ## __VA_ARGS__ )
#define DBG_ERROR( format, ... ) printf( "ERR:%s::%s:%d# " format "\n", __FILE_NAME__, DEBUG_FUNCTION_LABEL, __LINE__, ## __VA_ARGS__ )
#define DBG_FATAL( format, ... ) printf( "FTL:%s::%s:%d# " format "\n", __FILE_NAME__, DEBUG_FUNCTION_LABEL, __LINE__, ## __VA_ARGS__ )

namespace ja_iot {
namespace so_stack {
using namespace ja_iot::base;
using namespace ja_iot::network;
using namespace ja_iot::osal;
using namespace ja_iot::memory;
using namespace ja_iot::stack;

SoStackFacade *SoStackFacade::_p_instance{ nullptr };
SoStackFacade::SoStackFacade ()
{
}

ErrCode SoStackFacade::initialize()
{
  MemAlloctorType mem_alloctor_type;
  NetworkPlatform network_platform;

#ifdef _OS_LINUX_
  mem_alloctor_type = MemAlloctorType::kLinux;
  network_platform  = NetworkPlatform::kLinux;
#endif
#ifdef _OS_WINDOWS_
  mem_alloctor_type = MemAlloctorType::kWindows;
  network_platform  = NetworkPlatform::kWindows;
#endif
#ifdef _OS_FREERTOS_
  mem_alloctor_type = MemAlloctorType::kFreeRTOS;
  network_platform  = NetworkPlatform::kFreeRTOS;
#endif

  const auto mem_allocator = MemAllocatorFactory::create_set_mem_allocator( mem_alloctor_type );

  if( mem_allocator == nullptr )
  {
    DBG_ERROR( "Failed allocating MemAllocator" );
    return ( ErrCode::OUT_OF_MEM );
  }

  OsalMgr::Inst()->Init();

  const auto platform_factory = INetworkPlatformFactory::create_set_factory( network_platform );

  if( platform_factory == nullptr )
  {
    DBG_ERROR( "Failed allocating INetworkPlatformFactory" );
    return ( ErrCode::OUT_OF_MEM );
  }

  ResourceMgr::inst().init_default_resources();

  return ( ErrCode::OK );
}

ja_iot::base::ErrCode SoStackFacade::set_device_info( DeviceInfo &device_info )
{
  ResourceMgr::inst().set_device_info( device_info );
  return ( ErrCode::OK );
}
SoStackFacade::~SoStackFacade ()
{
}

SoStackFacade & SoStackFacade::inst()
{
  if( _p_instance == nullptr )
  {
    static SoStackFacade _instance{};
    _p_instance = &_instance;
  }

  return ( *_p_instance );
}

ErrCode SoStackFacade::start()
{
  MsgStack::inst().initialize( k_adapter_type_ip );
  return ( ErrCode::OK );
}

SoStackFacade & SoStackFacade::enable_ipv4( bool is_enabled )
{
  ConfigManager::Inst().get_ip_adapter_config()->set_flag( IP_ADAPTER_CONFIG_IPV4_UCAST, true );
  return ( *this );
}
SoStackFacade & SoStackFacade::enable_ipv4_mcast( bool is_enabled )
{
  ConfigManager::Inst().get_ip_adapter_config()->set_flag( IP_ADAPTER_CONFIG_IPV4_MCAST, true );
  return ( *this );
}
SoStackFacade & SoStackFacade::enable_ipv6( bool is_enabled )
{
  ConfigManager::Inst().get_ip_adapter_config()->set_flag( IP_ADAPTER_CONFIG_IPV6_UCAST, true );
  return ( *this );
}
SoStackFacade & SoStackFacade::enable_ipv6_mcast( bool is_enabled )
{
  ConfigManager::Inst().get_ip_adapter_config()->set_flag( IP_ADAPTER_CONFIG_IPV6_MCAST, true );
  return ( *this );
}
}
}
