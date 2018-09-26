/*
 * resource_mgr.h
 *
 *  Created on: Oct 8, 2017
 *      Author: psammand
 */

#pragma once

#include <ErrCode.h>
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include "OcfIResource.h"

namespace ja_iot {
namespace ocfstack {
class ResourceManagerImpl;

class ResourceManager
{
  public:
    static ResourceManager& inst();

  public:
    base::ErrCode              init_default_resources() const;
    OcfIResource*              find_resource_by_uri( const std::string &res_uri ) const;
    uint16_t                   get_no_of_resources() const;
    std::vector<OcfIResource *>& get_resources_list() const;

    base::ErrCode add_resource( OcfIResource *pcz_resource ) const;
    base::ErrCode remove_resource( OcfIResource *pcz_resource ) const;
    base::ErrCode remove_all_resources() const;

  private:
    ResourceManager ();
    ~ResourceManager ();

  private:
    static ResourceManager *               _p_instance;
    std::unique_ptr<ResourceManagerImpl>   _pcz_impl = nullptr;
};
}
}