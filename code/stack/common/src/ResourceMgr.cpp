/*
 * ResourceMgr.cpp
 *
 *  Created on: Oct 8, 2017
 *      Author: psammand
 */
#include <vector>
#include <algorithm>
#include <vector>
#include "ResourceMgr.h"
#include "common/inc/DeviceResource.h"
#include "common/inc/WellKnownResource.h"
#include "common/inc/TestResource/SmallPiggyBackResource.h"
#include "common/inc/TestResource/SmallSlowResponseResource.h"
#include "common/inc/TestResource/SmallNonResponseResource.h"
#include "common/inc/TestResource/BigPbResponse.h"
#include "common/inc/TestResource/BigCONResponse.h"
#include "common/inc/TestResource/BigNONResponse.h"

namespace ja_iot {
namespace stack {
using namespace ja_iot::base;

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
//  _cz_resources_list.push_back( new DeviceResource{} );
  _cz_resources_list.push_back( new WellKnownResource{} );
  _cz_resources_list.push_back( new SmallPiggybackResource{} );
  _cz_resources_list.push_back( new SmallSlowResponseResource{} );
  _cz_resources_list.push_back( new SmallNonResponseResource{} );
	_cz_resources_list.push_back(new BigPbResponse{});
	_cz_resources_list.push_back(new BigCONResponse{});
	_cz_resources_list.push_back(new BigNONResponse{});

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

uint16_t ResourceMgr::get_no_of_resources()
{
  return ( (uint16_t)_cz_resources_list.size() );
}

std::vector<IResource *> & ResourceMgr::get_resources_list()
{
  return ( _cz_resources_list );
}

ErrCode ResourceMgr::add_resource( IResource *pcz_resource )
{
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
}
}
