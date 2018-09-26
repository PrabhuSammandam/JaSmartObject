#pragma once
#include <cstdint>
#include "OcfIResource.h"
#include <vector>

namespace ja_iot {
namespace ocfstack {
class OcfExchange;

class OcfResource : public OcfIResource
{
  public:
    explicit OcfResource( std::string uri );

    uint8_t                 handle_request( OcfExchange &exchange ) override;
    std::vector<std::string>& get_types() override;
    void                    add_type( const std::string &type ) override;
    std::vector<std::string>& get_interfaces() override;
    void                    add_interface( const std::string &interfaces ) override;
    uint8_t                 get_property() override;
    void                    set_property( uint8_t property ) override;
    bool                    is_collection() override;
    std::string             & get_uri() override;
    void                    set_uri( const std::string &uri ) override;
    std::string             & get_unique_id() override;
    void                    set_unique_id( const std::string &unique_id ) override;
    uint8_t                 get_representation( OcfResInterfaceType interface_type, ResRepresentation &representation ) override;
    uint8_t                 get_discovery_representation( ResRepresentation &representation ) override;

  protected:
    uint8_t                    _property = 0;
    std::string                _unique_id;
    std::string                _uri;
    std::vector<std::string>   _types;
    std::vector<std::string>   _interfaces;
};
}
}