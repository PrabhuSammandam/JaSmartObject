#include "OcfFacade.h"
#include "OcfBaseServer.h"
#include "Mutex.h"
#include "OsalMgr.h"
#include "data_types.h"
#include "OcfPlatformConfig.h"
#include "IMemAllocator.h"
#include "i_nwk_platform_factory.h"
#include "OcfStack.h"
#include "ScopedMutex.h"

using namespace ja_iot::base;
using namespace ja_iot::osal;
using namespace ja_iot::memory;
using namespace ja_iot::network;

namespace ja_iot {
namespace ocfstack {
OcfFacade *OcfFacade::_p_instance{};

class OcfFacadeImpl
{
  public:
    OcfFacadeImpl () : _platform_cfg{ network::eDeviceType::CLIENT_SERVER }
    {
    }

    ErrCode platform_init();
    ErrCode initialise();
    ErrCode start();
    ErrCode stop();

    OcfPlatformConfig   _platform_cfg;
    std::unique_ptr<OcfBaseServer> _server{};
    Mutex *_facade_mutex{};

    bool _is_started{ false };
    bool _is_initialised{ false };
    bool _is_platform_inited{ false };
};

ErrCode OcfFacadeImpl::platform_init()
{
  if( _is_platform_inited )
  {
    return ( ErrCode::OK );
  }

  const auto mem_allocator = MemAllocatorFactory::create_mem_allocator( MemAlloctorType::kWindows );

  if( mem_allocator == nullptr )
  {
    return ( ErrCode::ERR );
  }

  MemAllocatorFactory::set( mem_allocator );

  OsalMgr::Inst()->Init();

  const auto platform_factory = INetworkPlatformFactory::create_factory( NetworkPlatform::kWindows );

  if( platform_factory == nullptr )
  {
    return ( ErrCode::ERR );
  }

  INetworkPlatformFactory::set_curr_factory( platform_factory );

  _is_platform_inited = true;

  return ( ErrCode::OK );
}

ErrCode OcfFacadeImpl::initialise()
{
  if( platform_init() != ErrCode::OK )
  {
    return ( ErrCode::ERR );
  }

  _facade_mutex = OsalMgr::Inst()->AllocMutex();

  if( !_facade_mutex )
  {
    return ( ErrCode::ERR );
  }

  if( _platform_cfg.is_server_enabled() )
  {
    _server = std::make_unique<OcfBaseServer>( _platform_cfg );

    if( !_server )
    {
      return ( ErrCode::ERR );
    }
  }

  _is_initialised = true;

  return ( ErrCode::OK );
}

ErrCode OcfFacadeImpl::start()
{
  if( !_is_initialised )
  {
    return ( ErrCode::ERR );
  }

  if( _is_started )
  {
    return ( ErrCode::OK );
  }

  ScopedMutex lock{ _facade_mutex };

  auto ret_status = ErrCode::OK;

  ret_status = OcfStack::inst().initialise( _platform_cfg );

  if( ret_status != ErrCode::OK )
  {
    return ( ret_status );
  }

  if( _platform_cfg.is_server_enabled() )
  {
    ret_status = _server->start();

    if( ret_status != ErrCode::OK )
    {
      return ( ret_status );
    }
  }

  _is_started = true;

  return ( ret_status );
}

ErrCode OcfFacadeImpl::stop()
{
  ScopedMutex lock{ _facade_mutex };
  auto ret_status = ErrCode::OK;

  if( _platform_cfg.is_server_enabled() )
  {
    ret_status = _server->stop();

    if( ret_status != ErrCode::OK )
    {
      return ( ret_status );
    }
  }

  _is_started = false;
  return ( ErrCode::OK );
}

OcfFacade & OcfFacade::inst()
{
  if( !_p_instance )
  {
    static OcfFacade static_instance{};
    _p_instance = &static_instance;
  }

  return ( *_p_instance );
}

ErrCode OcfFacade::initialise( const OcfPlatformConfig platform_cfg ) const
{
  _pimpl->_platform_cfg = platform_cfg;
  return ( _pimpl->initialise() );
}

ErrCode OcfFacade::start() const
{
  return ( _pimpl->start() );
}

ErrCode OcfFacade::stop() const
{
  return ( _pimpl->stop() );
}

ErrCode OcfFacade::set_device_info( OcfDeviceInfo &device_info ) const
{
  auto ret_status = ErrCode::ERR;

  if( _pimpl->_server )
  {
    ret_status = _pimpl->_server->set_device_info( device_info );
  }

  return ( ret_status );
}

base::ErrCode OcfFacade::set_platform_info( OcfPlatformInfo &platform_info ) const
{
  auto ret_status = ErrCode::ERR;

  if( _pimpl->_server )
  {
    ret_status = _pimpl->_server->set_platform_info( platform_info );
  }

  return ( ret_status );
}
OcfFacade::OcfFacade ()
{
  _pimpl = std::make_unique<OcfFacadeImpl>();
}
OcfFacade::~OcfFacade ()
{
}
}
}