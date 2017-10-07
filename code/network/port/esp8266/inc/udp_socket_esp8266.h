/*
 * UdpSocketImplEsp8266.h
 *
 *  Created on: Sep 11, 2017
 *      Author: psammand
 */

#ifndef NETWORK_PORT_ESP8266_INC_UDPSOCKETIMPLESP8266_H_
#define NETWORK_PORT_ESP8266_INC_UDPSOCKETIMPLESP8266_H_

#ifdef _OS_FREERTOS_

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

    SocketError OpenSocket( ja_iot::base::IpAddrFamily ip_addr_family = ja_iot::base::IpAddrFamily::IPV4 ) override;
    SocketError BindSocket( IpAddress &ip_address, uint16_t port = 0 ) override;
    SocketError CloseSocket()                                            override;

    SocketError JoinMulticastGroup( IpAddress &group_address, uint32_t if_index )  override;
    SocketError LeaveMulticastGroup( IpAddress &group_address, uint32_t if_index ) override;

    SocketError SelectMulticastInterface( IpAddress &group_address, uint32_t if_index )  override;

    SocketError ReceiveData( IpAddress &remote_addr, uint16_t &port, uint8_t *data, int16_t &data_length );
    SocketError SendData( IpAddress &remote_addr, uint16_t port, uint8_t *data, uint16_t data_length ) override;

    SocketError EnableMulticastLoopback(bool is_enabled)  override;
    SocketError EnableReuseAddr(bool is_enabled)  override;
    SocketError EnableIpv6Only(bool is_enabled)  override;
    SocketError EnablePacketInfo(bool is_enabled)  override;

    SocketError SetBlocking(bool is_blocked) override;

    uint16_t GetLocalPort() override;

    ja_iot::base::IpAddrFamily GetAddrFamily() override;


    int  getSocket() { return ( socket_fd_ ); }
    void setSocket( int socket_fd ) { socket_fd_ = socket_fd; }

  private:
    int            socket_fd_      = -1;
    ja_iot::base::IpAddrFamily   ip_addr_family_ = ja_iot::base::IpAddrFamily::IPV4;
};
}
}
#endif /* _OS_FREERTOS_ */




#endif /* NETWORK_PORT_ESP8266_INC_UDPSOCKETIMPLESP8266_H_ */
