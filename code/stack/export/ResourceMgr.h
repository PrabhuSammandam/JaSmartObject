#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include "ErrCode.h"
#include "IResource.h"

namespace ja_iot {
namespace stack {

class ResourceMgr
{
  public:
    static ResourceMgr& inst();

  public:
    base::ErrCode           init_default_resources() ;
    IResource*              find_resource_by_uri( const std::string &res_uri ) ;
    uint16_t                get_no_of_resources() ;
    std::vector<IResource *>& get_resources_list() ;

    base::ErrCode add_resource( IResource *pcz_resource ) ;
    base::ErrCode remove_resource( IResource *pcz_resource ) ;
    base::ErrCode remove_all_resources() ;

  private:
    ResourceMgr ();
    ~ResourceMgr ();

  private:
    static ResourceMgr *               _p_instance;
    std::vector<IResource *> _cz_resources_list{};
    ResourceMgr( const ResourceMgr &other )                   = delete;
    ResourceMgr( ResourceMgr &&other ) noexcept               = delete;
    ResourceMgr & operator = ( const ResourceMgr &other )     = delete;
    ResourceMgr & operator = ( ResourceMgr &&other ) noexcept = delete;
};
}
}
