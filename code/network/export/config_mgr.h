/*
 * config_mgr.h
 *
 *  Created on: Oct 6, 2017
 *      Author: psammand
 */

#pragma once

#include <ip_adapter_config.h>

namespace ja_iot
{
  namespace network
  {
    class ConfigManager
    {
    public:
      static ConfigManager& Inst();

      IpAdapterConfig* get_ip_adapter_config() { return &_cz_ip_adapter_config; }

    private:

      ConfigManager();

      ~ConfigManager();

      static ConfigManager* _pcz_instance;
      IpAdapterConfig _cz_ip_adapter_config;
    };
  }
}
