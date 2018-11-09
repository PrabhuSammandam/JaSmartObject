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

#include "../../../resources/common/inc/test/BigCONResponse.h"
#include "../../../resources/common/inc/test/BigNONResponse.h"
#include "../../../resources/common/inc/test/BigPbResponse.h"
#include "../../../resources/common/inc/test/SmallNonResponseResource.h"
#include "../../../resources/common/inc/test/SmallPiggyBackResource.h"
#include "../../../resources/common/inc/test/SmallSlowResponseResource.h"

namespace ja_iot {
namespace stack {
using namespace ja_iot::base;

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
  add_resource( new DeviceResource{} );
  add_resource( new WellKnownResource{} );

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
  return ( (uint16_t) _cz_resources_list.size() );
}

std::vector<IResource *> & ResourceMgr::get_resources_list()
{
  return ( _cz_resources_list );
}

ErrCode ResourceMgr::add_resource( IResource *pcz_resource )
{
  pcz_resource->set_unique_id( std::to_string( _u32_resource_id++ ) );
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