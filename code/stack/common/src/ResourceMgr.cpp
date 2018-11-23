/*
 * ResourceMgr.cpp
 *
 *  Created on: Oct 8, 2017
 *      Author: psammand
 */
#include <vector>
#include <algorithm>
#include "ResourceMgr.h"
#include "ResModuleInterface.h"

namespace ja_iot {
using namespace resources;
namespace stack {
using namespace ja_iot::base;

IResource *     _device_res   = nullptr;
IResource *     _platform_res = nullptr;

static uint32_t _u32_resource_id = 1;
ResourceMgr *ResourceMgr::_p_instance{ nullptr };
ResourceMgr::ResourceMgr ()
{
}
ResourceMgr::~ResourceMgr ()
{
}

ResourceMgr & ResourceMgr::inst()
{
  if( _p_instance == nullptr )
  {
    static ResourceMgr _instance{};
    _p_instance = &_instance;
  }

  return ( *_p_instance );
}

ErrCode ResourceMgr::init_default_resources()
{
  resource_module_init_core_resources();
  resource_module_init_secure_virtual_resources();
  return ( ErrCode::OK );
}

IResource * ResourceMgr::find_resource_by_uri( const std::string &rstr_res_uri )
{
  if( rstr_res_uri.empty() || ( _cz_resources_list.empty() == true ) )
  {
    return ( nullptr );
  }

  for( auto &pcz_loop_resource : this->_cz_resources_list )
  {
    if( pcz_loop_resource->get_uri() == rstr_res_uri )
    {
      return ( pcz_loop_resource );
    }
  }

  return ( nullptr );
}

#if 0
IResource * ResourceMgr::find_resource_by_uri( const std::string_view res_uri )
{
  if( res_uri.empty() || ( _cz_resources_list.empty() == true ) )
  {
    return ( nullptr );
  }

  for( auto &pcz_loop_resource : this->_cz_resources_list )
  {
    if( pcz_loop_resource->get_uri() == res_uri )
    {
      return ( pcz_loop_resource );
    }
  }

  return ( nullptr );
}
#endif

uint16_t ResourceMgr::get_no_of_resources()
{
  return ( (uint16_t) _cz_resources_list.size() );
}

std::vector<IResource *> & ResourceMgr::get_resources_list()
{
  return ( _cz_resources_list );
}

ErrCode ResourceMgr::add_resource( IResource *pcz_resource )
{
  char buf[30];

  sprintf( &buf[0], "%d", _u32_resource_id++ );
  pcz_resource->set_unique_id( std::string( &buf[0] ) );
  _cz_resources_list.push_back( pcz_resource );
  return ( ErrCode::OK );
}

ErrCode ResourceMgr::remove_resource( IResource *pcz_resource )
{
  const auto find_result = std::find( _cz_resources_list.cbegin(), _cz_resources_list.cend(), pcz_resource );

  if( find_result != _cz_resources_list.cend() )
  {
    delete *find_result;
    _cz_resources_list.erase( find_result );
    return ( ErrCode::OK );
  }

  return ( ErrCode::ERR );
}

ErrCode ResourceMgr::remove_all_resources()
{
  for( auto &pcz_loop_resource : _cz_resources_list )
  {
    delete pcz_loop_resource;
  }

  _cz_resources_list.clear();

  return ( ErrCode::OK );
}

#define CHECK_NULL_OR_FIND_AND_RETURN( _RES_PTR_, _URI_ ) \
  static IResource *_RES_PTR_; \
  if( _RES_PTR_ == nullptr ) \
  { \
    _RES_PTR_ = find_resource_by_uri( _URI_ ); \
  } return _RES_PTR_;
IResource * ResourceMgr::get_device_owner_xfer_method_res()
{
  CHECK_NULL_OR_FIND_AND_RETURN( _doxm_res, "/oic/sec/doxm" );
}
IResource * ResourceMgr::get_access_control_list_res()
{
  CHECK_NULL_OR_FIND_AND_RETURN( _acl_res, "/oic/sec/acl" );
}
IResource * ResourceMgr::get_provisioning_status_res()
{
  CHECK_NULL_OR_FIND_AND_RETURN( _pstat_res, "/oic/sec/pstat" );
}
IResource * ResourceMgr::get_credential_res()
{
  CHECK_NULL_OR_FIND_AND_RETURN( _cred_res, "/oic/sec/cred" );
}
}
}
