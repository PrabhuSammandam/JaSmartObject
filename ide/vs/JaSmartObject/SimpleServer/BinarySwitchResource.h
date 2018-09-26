#pragma once
#include <string>
#include "BaseResource.h"
#include "QueryContainer.h"

class BinarySwitchResource : public ja_iot::stack::BaseResource
{
  public:
    BinarySwitchResource ();
		BinarySwitchResource(std::string uri);

    bool    is_method_supported( uint8_t method ) override;
    uint8_t get_representation( ja_iot::stack::ResInterfaceType interface_type, ja_iot::stack::ResRepresentation &representation ) override;

    uint8_t handle_get( ja_iot::stack::QueryContainer &query_container, ja_iot::stack::Interaction *interaction ) override;
    uint8_t handle_post( ja_iot::stack::QueryContainer &query_container, ja_iot::stack::Interaction *interaction ) override;

  private:
    void init();

    bool   _value = false;
};