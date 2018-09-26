#pragma once
#include <cstdint>
#include <vector>
#include "IResource.h"
#include "QueryContainer.h"

namespace ja_iot {
namespace stack {
class OcfExchange;

class BaseResource : public IResource
{
  public:
    explicit BaseResource( std::string uri );

    uint8_t                 handle_request( Interaction *interaction ) override;
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
    uint8_t                 get_representation( ResInterfaceType interface_type, ResRepresentation &representation ) override;
    uint8_t                 get_discovery_representation( ResRepresentation &representation ) override;
    void                    add_interfaces( std::vector<std::string> &interfaces );

    virtual bool    is_method_supported( uint8_t method ) { return ( false ); }
    virtual uint8_t handle_get( QueryContainer &query_container, Interaction *interaction );
    virtual uint8_t handle_post( QueryContainer &query_container, Interaction *interaction );
    virtual uint8_t handle_put( QueryContainer &query_container, Interaction *interaction );
    virtual uint8_t handle_delete( QueryContainer &query_container, Interaction *interaction );
    uint8_t         set_response( Interaction *interaction, ResRepresentation &representation );

    bool    is_discoverable() { return ( ( _property & OCF_RESOURCE_PROP_DISCOVERABLE ) != 0 ); }
    uint8_t check_interface_query( QueryContainer &query_container );
    uint8_t check_type_query( QueryContainer &query_container );

  protected:
    uint8_t                    _property = 0;
    std::string                _unique_id;
    std::string                _uri;
    std::vector<std::string>   _types;
    std::vector<std::string>   _interfaces;
};
}
}