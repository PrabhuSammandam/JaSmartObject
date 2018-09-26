#pragma once
#include "OcfMsgHeader.h"
#include "OcfObserveInfo.h"
#include "ResPropValue.h"
#include "OcfQueryContainer.h"

namespace ja_iot {
namespace ocfstack {
constexpr uint8_t OCF_REQUEST_FLAG_NONE    = 0x00;
constexpr uint8_t OCF_REQUEST_FLAG_REQUEST = 0x01;
constexpr uint8_t OCF_REQUEST_FLAG_OBSERVE = 0x02;

class OcfRequest : public OcfMsgHeader
{
  public:
    OcfRequest ()
    {
      
    }
    OcfQueryContainer& get_query_container() { return ( _query_container ); }
    OcfObserveInfo   & get_observe_info() { return ( _observe_info ); }
    std::string      & get_uri() { return ( _uri ); }
    void             set_uri( const std::string &uri ) { _uri = uri; }

  private:
    std::string         _uri;
    OcfQueryContainer   _query_container;
    uint8_t             _flags = OCF_REQUEST_FLAG_NONE;
    OcfObserveInfo      _observe_info;
    ResRepresentation   _representation;
};
}
}