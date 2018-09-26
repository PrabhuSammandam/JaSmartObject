#pragma once

#include <cstdint>
#include "OcfExchange.h"
#include <OcfResourceConsts.h>

namespace ja_iot {
namespace ocfstack {
class OcfIResource
{
  public:
    virtual ~OcfIResource () = default;

    virtual uint8_t handle_request( OcfExchange &exchange ) = 0;

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

    virtual uint8_t get_representation( OcfResInterfaceType interface_type, ResRepresentation &representation ) = 0;
    virtual uint8_t get_discovery_representation( ResRepresentation &representation )                           = 0;
};
}
}