#pragma once
#include <string>
#include "BaseResource.h"
#include "QueryContainer.h"

using namespace ja_iot::base;
using namespace ja_iot::stack;

class CollectionResource : public BaseResource
{
public:
	CollectionResource();

	uint8_t handle_request(Interaction *interaction) override;
	uint8_t get_representation(ResInterfaceType interface_type, ResRepresentation &representation) override;
	uint8_t get_discovery_representation(ResRepresentation &representation) override;

private:
	uint8_t handle_get(Interaction *interaction);
	uint8_t handle_post(Interaction *interaction);
	void init();
};