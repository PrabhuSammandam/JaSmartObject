/*
 * UdpSocketImplEsp8266.h
 *
 *  Created on: Sep 11, 2017
 *      Author: psammand
 */

#ifdef _OS_FREERTOS_
#pragma once

#include <lwip/sockets.h>
#include <ip_addr.h>
#include <cstdint>
#include <i_udp_socket.h>

namespace ja_iot {
namespace network {
class UdpSocketImplEsp8266 : public IUdpSocket
{
  public:

	UdpSocketImplEsp8266 ();

    SocketError OpenSocket( base::IpAddrFamily ip_addr_family = base::IpAddrFamily::IPv4 ) override;
    SocketError BindSocket( base::IpAddress &ip_address, uint16_t port = 0 ) override;
    SocketError CloseSocket()                                            override;

    SocketError JoinMulticastGroup( base::IpAddress &group_address, uint32_t if_index )  override;
    SocketError LeaveMulticastGroup( base::IpAddress &group_address, uint32_t if_index ) override;

    SocketError SelectMulticastInterface( base::IpAddress &group_address, uint32_t if_index )  override;

    SocketError ReceiveData( base::IpAddress &remote_addr, uint16_t &port, uint8_t *data, int16_t &data_length );
    SocketError SendData( base::IpAddress &remote_addr, uint16_t port, uint8_t *data, uint16_t data_length ) override;

    SocketError EnableMulticastLoopback(bool is_enabled)  override;
    SocketError EnableReuseAddr(bool is_enabled)  override;
    SocketError EnableIpv6Only(bool is_enabled)  override;
    SocketError EnablePacketInfo(bool is_enabled)  override;

    SocketError SetBlocking(bool is_blocked) override;

    uint16_t GetLocalPort() override;

    base::IpAddrFamily GetAddrFamily() override;


    int  getSocket() { return ( socket_fd_ ); }
    void setSocket( int socket_fd ) { socket_fd_ = socket_fd; }

  private:
    int            socket_fd_      = -1;
    base::IpAddrFamily   ip_addr_family_ = base::IpAddrFamily::IPv4;
};
}
}
#endif /* _OS_FREERTOS_ */
