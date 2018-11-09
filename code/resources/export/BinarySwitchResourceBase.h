/*
 * BinarySwitchResourceBase.h
 *
 *  Created on: Nov 8, 2018
 *      Author: psammand
 */

#pragma once

#include "BaseResource.h"

namespace ja_iot::resources {
using namespace ja_iot::stack;

class BinarySwitchResourceBase : public BaseResource
{
  public:
    BinarySwitchResourceBase( std::string uri );

    bool         is_method_supported( uint8_t method ) override;
    uint8_t      get_discovery_representation( ResRepresentation &representation ) override;
    uint8_t      get_representation( ResInterfaceType interface_type, ResRepresentation &representation ) override;
    virtual void handle_property_change() {}

    bool getValue() { return ( _value ); }
    void setValue( bool value ) { _value = value; }

  private:
    uint8_t handle_get( QueryContainer &query_container, Interaction *interaction ) override;
    uint8_t handle_post( QueryContainer &query_container, Interaction *interaction ) override;
    void    init();

    bool   _value = false;
};
}