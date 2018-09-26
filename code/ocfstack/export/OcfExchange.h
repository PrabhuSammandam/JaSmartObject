#pragma once
#include "OcfRequest.h"
#include "OcfResponse.h"

namespace ja_iot {
namespace ocfstack {
class OcfExchange
{
  public:
    uint8_t    respond() { return 0; }
    OcfRequest & get_request() { return ( _request ); }
    OcfResponse& get_response() { return ( _response ); }

  private:
    OcfRequest    _request;
    OcfResponse   _response;
};
}
}