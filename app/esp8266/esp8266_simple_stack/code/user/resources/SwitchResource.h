/*
 * SwitchResource.h
 *
 *  Created on: Feb 19, 2018
 *      Author: psammand
 */

#pragma once

#include <string>
#include "BaseResource.h"
#include "Exchange.h"

class SwitchResource : public ja_iot::stack::BaseResource
{
  public:
    SwitchResource ();

    bool    is_method_supported( uint8_t method ) override;
    uint8_t get_representation( ja_iot::stack::ResInterfaceType interface_type, ja_iot::stack::ResRepresentation &representation ) override;

    uint8_t handle_get( ja_iot::stack::QueryContainer &query_container, ja_iot::stack::Interaction *interaction ) override;
    uint8_t handle_post( ja_iot::stack::QueryContainer &query_container, ja_iot::stack::Interaction *interaction ) override;

  private:
    void init();

    bool   _value = 0;
};