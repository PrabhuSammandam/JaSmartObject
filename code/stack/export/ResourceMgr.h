#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include "ErrCode.h"
#include "IResource.h"
#include "DeviceInfo.h"  

namespace ja_iot {
namespace stack {
class ResourceMgr
{
  public:
    static ResourceMgr& inst();

  public:
    base::ErrCode           init_default_resources();
    IResource*              find_resource_by_uri( const std::string &res_uri );
    uint16_t                get_no_of_resources();
    std::vector<IResource *>& get_resources_list();

    base::ErrCode add_resource( IResource *pcz_resource );
    base::ErrCode remove_resource( IResource *pcz_resource );
    base::ErrCode remove_all_resources();
    DeviceInfo& get_device_info() { return ( _device_info ); }
		void      set_device_info(DeviceInfo &device_info) { _device_info = std::move(device_info); }

  private:
    ResourceMgr ();
    ~ResourceMgr ();
		  
  private:
    static ResourceMgr * _p_instance;
    std::vector<IResource *> _cz_resources_list{};
    ResourceMgr( const ResourceMgr &other )                   = delete;
    ResourceMgr( ResourceMgr &&other ) noexcept               = delete;
    ResourceMgr & operator = ( const ResourceMgr &other )     = delete;
    ResourceMgr & operator = ( ResourceMgr &&other ) noexcept = delete;

    DeviceInfo _device_info;
};
}
}