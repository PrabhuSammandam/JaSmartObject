/*
 * config_mgr.cpp
 *
 *  Created on: Oct 6, 2017
 *      Author: psammand
 */

#include <config_mgr.h>

namespace ja_iot {
namespace network {
ConfigManager *ConfigManager::p_instance_{ nullptr };

ConfigManager & ConfigManager::Inst()
{
  if( p_instance_ == nullptr )
  {
    static ConfigManager _instance{};
    p_instance_ = &_instance;
  }

  return ( *p_instance_ );
}

ConfigManager::ConfigManager ()
{
}

ConfigManager::~ConfigManager ()
{
}
}
}