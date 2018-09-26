/*
 * config_mgr.cpp
 *
 *  Created on: Oct 6, 2017
 *      Author: psammand
 */

#include <config_mgr.h>

namespace ja_iot {
namespace network {
ConfigManager *ConfigManager::_pcz_instance{};

ConfigManager & ConfigManager::Inst()
{
  if( _pcz_instance == nullptr )
  {
    static ConfigManager scz_instance{};
    _pcz_instance = &scz_instance;
  }

  return ( *_pcz_instance );
}
ConfigManager::ConfigManager ()
{
}
ConfigManager::~ConfigManager ()
{
}
}
}