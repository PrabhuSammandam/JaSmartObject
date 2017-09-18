/*
 * EndPoint.h
 *
 *  Created on: Jun 30, 2017
 *      Author: psammand
 */

#ifndef NETWORK_EXPORT_END_POINT_H_
#define NETWORK_EXPORT_END_POINT_H_

#include <string.h>
#include <data_types.h>
#include <cstdint>

constexpr uint16_t kEndpointAddressMaxSize = 66;

namespace ja_iot {
namespace network {
class Endpoint
{
  public:

    Endpoint () {}

    Endpoint( const Endpoint &other ) { *this = other; }

    Endpoint & operator = ( const Endpoint &other );

    AdapterType getAdapterType() const { return ( adapter_type_ ); }
    void        setAdapterType( AdapterType adapter_type ) { adapter_type_ = adapter_type; }
    uint32_t    getIfIndex() const { return ( if_index_ ); }
    void        setIfIndex( uint32_t if_index ) { if_index_ = if_index; }
    NetworkFlag getNetworkFlags() const { return ( network_flags_ ); }
    void        setNetworkFlags( NetworkFlag network_flags ) { network_flags_ = network_flags; }
    uint16_t    getPort() const { return ( port_ ); }
    void        setPort( uint16_t port ) { port_ = port; }

    uint8_t* getAddr() { return ( &addr_[0] ); }
    void     setAddr( const char *addr );

    bool SetIpv6AddrByScope( NetworkFlag ipv6_scope );

  private:
    AdapterType   adapter_type_                  = AdapterType::TYPE_DEFAULT;
    NetworkFlag   network_flags_                 = NetworkFlag::FLAG_DEFAULT;
    uint16_t      port_                          = 0;
    uint32_t      if_index_                      = 0;
    uint8_t       addr_[kEndpointAddressMaxSize] = { 0 };
};
}
}

#endif /* NETWORK_EXPORT_END_POINT_H_ */
