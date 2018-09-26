#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include "Exchange.h"
#include "ResPropValue.h"
#include "ResourceConsts.h"
#include "Interaction.h"

namespace ja_iot {
namespace stack {
class IResource
{
  public:
    virtual ~IResource () = default;

    virtual uint8_t handle_request( Interaction *interaction ) = 0;

    virtual std::string             & get_uri()                                    = 0;
    virtual void                    set_uri( const std::string &uri )              = 0;
    virtual std::string             & get_unique_id()                              = 0;
    virtual void                    set_unique_id( const std::string &unique_id )  = 0;
    virtual std::vector<std::string>& get_types()                                  = 0;
    virtual void                    add_type( const std::string &type )            = 0;
    virtual std::vector<std::string>& get_interfaces()                             = 0;
    virtual void                    add_interface( const std::string &interfaces ) = 0;
    virtual uint8_t                 get_property()                                 = 0;
    virtual void                    set_property( uint8_t property )               = 0;
    virtual bool                    is_collection()                                = 0;

    virtual uint8_t get_representation( ResInterfaceType interface_type, ResRepresentation &representation ) = 0;
    virtual uint8_t get_discovery_representation( ResRepresentation &representation )                        = 0;
};
}
}