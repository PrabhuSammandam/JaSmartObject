#pragma once
#include "ErrCode.h"
#include "packet.h"

namespace ja_iot {
namespace ocfstack {
class ServerRequest;
class OcfStackImpl;

class OcfStackRequestHandler
{
  public:
    OcfStackRequestHandler( OcfStackImpl *p_stack_impl ) : _p_stack_impl{ p_stack_impl } {}
    ~OcfStackRequestHandler () { _p_stack_impl = nullptr; }

    base::ErrCode process_request(network::RequestPacket *pcz_request_packet);
    base::ErrCode process_server_request( ServerRequest &rcz_server_request );
    base::ErrCode process_default_device_request( ServerRequest &rcz_server_request ) const;

  private:
    OcfStackImpl * _p_stack_impl;
    OcfStackRequestHandler( const OcfStackRequestHandler &other )                   = delete;
    OcfStackRequestHandler( OcfStackRequestHandler &&other ) noexcept               = delete;
    OcfStackRequestHandler & operator = ( const OcfStackRequestHandler &other )     = delete;
    OcfStackRequestHandler & operator = ( OcfStackRequestHandler &&other ) noexcept = delete;
};
}
}