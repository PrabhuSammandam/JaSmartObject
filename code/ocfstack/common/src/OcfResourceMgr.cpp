/*
 * ResourceManager.cpp
 *
 *  Created on: Oct 8, 2017
 *      Author: psammand
 */
#include <vector>
#include <algorithm>
#include <vector>
#include "OcfResourceMgr.h"
#include "OcfDeviceResource.h"
#include "OcfWellKnownResource.h"

namespace ja_iot {
namespace ocfstack {
using namespace ja_iot::base;

ResourceManager *ResourceManager::_p_instance{ nullptr };

class ResourceManagerImpl
{
  public:
    ResourceManagerImpl ()
    {
      _cz_resources_list.reserve( 10 );
    }
    ErrCode       init_default_resources();
    OcfIResource* find_resource_by_uri( const std::string &res_uri );
    ErrCode       add_resource( OcfIResource *pcz_resource );
    ErrCode       remove_resource( OcfIResource *pcz_resource );
    ErrCode       remove_all_resources();

  public:
    std::vector<OcfIResource *> _cz_resources_list{};
};

ResourceManager::ResourceManager ()
{
  _pcz_impl = std::make_unique<ResourceManagerImpl>();
}
ResourceManager::~ResourceManager ()
{
}

ResourceManager & ResourceManager::inst()
{
  if( _p_instance == nullptr )
  {
    static ResourceManager _instance{};
    _p_instance = &_instance;
  }

  return ( *_p_instance );
}

ErrCode ResourceManager::init_default_resources() const
{
  return ( _pcz_impl->init_default_resources() );
}

OcfIResource * ResourceManager::find_resource_by_uri( const std::string &res_uri ) const
{
  return ( _pcz_impl->find_resource_by_uri( res_uri ) );
}

uint16_t ResourceManager::get_no_of_resources() const
{
  return ( _pcz_impl->_cz_resources_list.size() );
}

std::vector<OcfIResource *> & ResourceManager::get_resources_list() const
{
  return ( _pcz_impl->_cz_resources_list );
}

ErrCode ResourceManager::add_resource( OcfIResource *pcz_resource ) const
{
  return ( _pcz_impl->add_resource( pcz_resource ) );
}

ErrCode ResourceManager::remove_resource( OcfIResource *pcz_resource ) const
{
  return ( _pcz_impl->remove_resource( pcz_resource ) );
}

ErrCode ResourceManager::remove_all_resources() const
{
  return ( _pcz_impl->remove_all_resources() );
}

/**********************************************************************************************************************/
/**************                       ResourceManagerImpl DEFINITIONS            **************************************/
/**********************************************************************************************************************/

ErrCode ResourceManagerImpl::add_resource( OcfIResource *pcz_resource )
{
  _cz_resources_list.push_back( pcz_resource );
  return ( ErrCode::OK );
}

ErrCode ResourceManagerImpl::remove_resource( OcfIResource *pcz_resource )
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

ErrCode ResourceManagerImpl::remove_all_resources()
{
  for( auto &resource : _cz_resources_list )
  {
    delete resource;
  }

  _cz_resources_list.clear();

  return ( ErrCode::OK );
}

ErrCode ResourceManagerImpl::init_default_resources()
{
  _cz_resources_list.push_back( new OcfDeviceResource{} );
  _cz_resources_list.push_back( new OcfWellKnownResource{} );

  return ( ErrCode::OK );
}

OcfIResource * ResourceManagerImpl::find_resource_by_uri( const std::string &res_uri )
{
  if( res_uri.empty() || ( _cz_resources_list.empty() == true ) )
  {
    return ( nullptr );
  }

  for( auto &resource : _cz_resources_list )
  {
    if( resource->get_uri() == res_uri )
    {
      return ( resource );
    }
  }

  return ( nullptr );
}
}
}