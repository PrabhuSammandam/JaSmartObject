#ifdef COMPILE_TEST_RESOURCES
/*
 * SmallPiggyBackResource.h
 *
 *  Created on: Feb 2, 2018
 *      Author: psammand
 */

#pragma once

#include <string>
#include "BaseResource.h"
#include "Exchange.h"

namespace ja_iot::resources {
using namespace stack;

class SmallPiggybackResource : public BaseResource
{
  public:
	SmallPiggybackResource ();

    uint8_t handle_request( Interaction *interaction ) override;

  private:
    void init();
};
}
#endif
