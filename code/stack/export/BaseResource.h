#pragma once
#include <cstdint>
#include <vector>
#include "IResource.h"
#include "QueryContainer.h"
#include "common_typedefs.h"

namespace ja_iot::stack {
class OcfExchange;

class BaseResource : public IResource
{
  public:
    explicit BaseResource( std::string uri );

    uint8_t       handle_request( Interaction *interaction ) override;
    TypeArray     & get_types() override;
    void          add_type( const std::string &type ) override;
    InterfaceArray& get_interfaces() override;
    void          add_interface( const std::string &interface ) override;
    void          add_interfaces( InterfaceArray &interfaces );
    uint8_t       get_property() override;
    void          set_property( uint8_t property ) override;
    bool          is_collection() override;
    std::string   & get_uri() override;
    void          set_uri( const std::string &uri ) override;
    std::string   & get_unique_id() override;
    void          set_unique_id( const std::string &unique_id ) override;
    uint8_t       get_representation( ResInterfaceType interface_type, ResRepresentation &representation ) override;
    uint8_t       get_discovery_representation( ResRepresentation &representation ) override;

    virtual bool    is_method_supported( uint8_t method ) { return ( false ); }
    virtual uint8_t handle_get( QueryContainer &query_container, Interaction *interaction );
    virtual uint8_t handle_post( QueryContainer &query_container, Interaction *interaction );
    virtual uint8_t handle_put( QueryContainer &query_container, Interaction *interaction );
    virtual uint8_t handle_delete( QueryContainer &query_container, Interaction *interaction );
    uint8_t         set_response( Interaction *interaction, ResRepresentation &representation );

    bool    is_discoverable() { return ( ( _attribute & OCF_RESOURCE_PROP_DISCOVERABLE ) != 0 ); }
    uint8_t check_interface_query( QueryContainer &query_container );
    uint8_t check_type_query( QueryContainer &query_container );

    void                    add_property_name( const std::string &property_name ) { _property_name_list.push_back( property_name ); }
    std::vector<std::string>& get_property_name_list() { return ( _property_name_list ); }

  protected:
    void get_endpoint_list_representation( ResRepresentation &rcz_res_rep );

  protected:
    uint8_t                    _attribute = 0;
    std::string                _unique_id;
    std::string                _uri;
    TypeArray                  _types;
    InterfaceArray             _interfaces;
    std::vector<std::string>   _property_name_list;
};
}