/*
 * res_link.h
 *
 *  Created on: Nov 9, 2018
 *      Author: psammand
 */

#pragma once
#include <string>
#include <IResource.h>

class ResLink
{
  public:
    std::string                _target_uri;
    std::vector<std::string>   _relations;
    std::vector<std::string>   _types;
    std::vector<std::string>   _interfaces;
    std::string                _device_id;
    uint16_t                   _attributes;
    std::string                _anchor;
    uint16_t                   _instance_id;
    uint16_t                   _media_type;
    std::vector<std::string>   _endpoints;
};
