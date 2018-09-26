#pragma once

#include <string>
#include "BaseResource.h"
#include "Exchange.h"

namespace ja_iot {
namespace stack {
class BigCONResponse : public BaseResource
{
  public:
    BigCONResponse ();

    uint8_t handle_request( Interaction *interaction ) override;

  private:
    void init();
};
}
}