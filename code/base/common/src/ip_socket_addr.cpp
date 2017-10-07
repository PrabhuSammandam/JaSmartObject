#include <ip_socket_addr.h>
#include <Hash.h>
#include <stdio.h>

namespace ja_iot {
namespace base {
IpSocketAddr::IpSocketAddr () : _ipAddr{}
{
}

IpSocketAddr::IpSocketAddr( const IpAddress &ipAddr, uint16_t port ) : _ipAddr{ ipAddr }, _port{ port }
{
}

IpSocketAddr::IpSocketAddr( const uint8_t *ipAddr, uint16_t port ) : _ipAddr{ (const char *)( ipAddr ) }, _port{ port }
{
}

IpSocketAddr::~IpSocketAddr ()
{
    _ipAddr.~IpAddress ();
    _port = 0;
}

bool IpSocketAddr::operator == ( const IpSocketAddr &other )
{
    return ( ( _port == other._port ) && ( _ipAddr == other._ipAddr ) );
}

void IpSocketAddr::Print()
{
    printf( "IP:" );
//    _ipAddr.Print();
    printf( "Port:%u\n", _port );
}

uint32_t IpSocketAddr::GetHashValue()
{
    auto hashVal = Hash::get_hash( _ipAddr.get_addr(), sizeof( IpAddress ) );

    hashVal = Hash::get_hash( &_port, sizeof( uint16_t ), hashVal );

    return ( hashVal );
}
}
}
