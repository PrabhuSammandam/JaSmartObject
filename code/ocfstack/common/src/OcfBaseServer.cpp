#include <OcfBaseServer.h>
#include <connectivity_mgr.h>
#include <data_types.h>
#include <Mutex.h>
#include <OsalError.h>
#include <OsalMgr.h>
#include <OcfPlatformInfo.h>
#include <Task.h>
#include <algorithm>
#include <OcfPlatformConfig.h>
#include "ScopedMutex.h"
#include "OcfStack.h"

using namespace ja_iot::base;
using namespace ja_iot::osal;

namespace ja_iot {
namespace ocfstack {
constexpr uint16_t          BASE_SERVER_TASK_STACK_SIZE = 0;
constexpr uint16_t          BASE_SERVER_TASK_PRIORITY   = 0;
constexpr const char *const BASE_SERVER_TASK_NAME       = "base_srvr";

class BaseServerImpl
{
  public:
    BaseServerImpl () : _platform_cfg{ network::eDeviceType::CLIENT_SERVER } {}

    void task_function() const;

    OcfPlatformConfig   _platform_cfg;
    Mutex *_access_mutex{};
    Task *_task{};
    bool _is_stopped{ true };
};

void BaseServerImpl::task_function() const
{
  while( !_is_stopped )
  {
    OcfStack::inst().check_and_process_received_data();
    // sleep for some seconds
  }
}

void task_function_wrapper( void *arg )
{
  const auto base_server_impl = static_cast<BaseServerImpl *>( arg );

  if( base_server_impl )
  {
    base_server_impl->task_function();
  }
}

/**********************************************************************************************************************/
/**************                       OcfBaseServer DEFINITIONS            **************************************/
/**********************************************************************************************************************/
OcfBaseServer::OcfBaseServer( OcfPlatformConfig platform_cfg ) : _pimpl{ std::make_unique<BaseServerImpl>() }
{
  _pimpl->_platform_cfg = std::move( platform_cfg );
}
OcfBaseServer::~OcfBaseServer ()
{
}

ErrCode OcfBaseServer::start()
{
  if( !_pimpl->_is_stopped )
  {
    return ( ErrCode::ERR );
  }

  _pimpl->_access_mutex = OsalMgr::Inst()->AllocMutex();

  if( !_pimpl->_access_mutex )
  {
    return ( ErrCode::ERR );
  }

  _pimpl->_task = OsalMgr::Inst()->AllocTask();

  if( !_pimpl->_task )
  {
    OsalMgr::Inst()->FreeMutex( _pimpl->_access_mutex );
    return ( ErrCode::ERR );
  }

  task_creation_params_t st_task_creation_params = {};

  st_task_creation_params.cz_name        = BASE_SERVER_TASK_NAME;
  st_task_creation_params.u16_stack_size = BASE_SERVER_TASK_STACK_SIZE;
  st_task_creation_params.u32_priority   = BASE_SERVER_TASK_PRIORITY;
  st_task_creation_params.pfn_run_cb     = task_function_wrapper;
  st_task_creation_params.pv_task_arg    = this->_pimpl.get();

  auto osal_ret_status = _pimpl->_task->Init( &st_task_creation_params );

  if( osal_ret_status != OsalError::OK )
  {
    OsalMgr::Inst()->FreeTask( _pimpl->_task );
    OsalMgr::Inst()->FreeMutex( _pimpl->_access_mutex );
    return ( ErrCode::ERR );
  }

  osal_ret_status = _pimpl->_task->Start();

  if( osal_ret_status != OsalError::OK )
  {
    OsalMgr::Inst()->FreeTask( _pimpl->_task );
    OsalMgr::Inst()->FreeMutex( _pimpl->_access_mutex );
    return ( ErrCode::ERR );
  }

  _pimpl->_is_stopped = false;

  return ( ErrCode::OK );
}

ErrCode OcfBaseServer::stop()
{
  return ( ErrCode::OK );
}

ErrCode OcfBaseServer::register_resource( OcfIResource *pcz_resource )
{
  ScopedMutex lock{ _pimpl->_access_mutex };

  return ( ErrCode::OK );
}

ErrCode OcfBaseServer::unregister_resource( OcfIResource *pcz_resource )
{
  ScopedMutex lock{ _pimpl->_access_mutex };

  return ( ErrCode::OK );
}

ErrCode OcfBaseServer::get_supported_transport_schemes( uint16_t &u16_transport_schemes )
{
  ScopedMutex lock{ _pimpl->_access_mutex };

  return ( ErrCode::OK );
}

ErrCode OcfBaseServer::set_device_info( OcfDeviceInfo &device_info )
{
  ScopedMutex lock{ _pimpl->_access_mutex };

  return ( OcfStack::inst().set_device_info( device_info ) );
}

ErrCode OcfBaseServer::set_platform_info( OcfPlatformInfo &platform_info )
{
  ScopedMutex lock{ _pimpl->_access_mutex };

  return ( OcfStack::inst().set_platform_info( platform_info ) );
}
}
}