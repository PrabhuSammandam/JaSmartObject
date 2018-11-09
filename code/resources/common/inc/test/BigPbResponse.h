#ifdef COMPILE_TEST_RESOURCES

#pragma once

#include <string>
#include "BaseResource.h"
#include "Exchange.h"

namespace ja_iot::resources {
using namespace stack;
class BigPbResponse : public BaseResource
{
  public:
    BigPbResponse ();

    uint8_t handle_request( Interaction *interaction ) override;

  private:
    void init();
};
}
#endif
