/*
 * SocketImplWindows.h
 *
 *  Created on: Jul 15, 2017
 *      Author: psammand
 */

#ifndef UDPSOCKETIMPLWINDOWS_H_
#define UDPSOCKETIMPLWINDOWS_H_

#ifdef _OS_WINDOWS_

#include <i_udp_socket.h>
#include <winsock2.h>

namespace ja_iot {
namespace network {
class UdpSocketImplWindows : public IUdpSocket
{
  public:

    UdpSocketImplWindows ();

    SocketError OpenSocket( ja_iot::base::IpAddrFamily ip_addr_family = ja_iot::base::IpAddrFamily::IPV4 ) override;
    SocketError BindSocket( IpAddress &ip_address, uint16_t port = 0 ) override;
    SocketError CloseSocket()                                            override;

    SocketError JoinMulticastGroup( IpAddress &group_address, uint32_t if_index )  override;
    SocketError LeaveMulticastGroup( IpAddress &group_address, uint32_t if_index ) override;

    SocketError SelectMulticastInterface( IpAddress &group_address, uint32_t if_index )  override;

    SocketError ReceiveData( IpAddress &remote_addr, uint16_t &port, uint8_t *data, int16_t &data_length ) override;
    SocketError SendData( IpAddress &remote_addr, uint16_t port, uint8_t *data, uint16_t data_length ) override;

    SocketError EnableMulticastLoopback(bool is_enabled)  override;
    SocketError EnableReuseAddr(bool is_enabled)  override;
    SocketError EnableIpv6Only(bool is_enabled)  override;
    SocketError EnablePacketInfo(bool is_enabled)  override;

    SocketError SetBlocking(bool is_blocked)override;

    uint16_t GetLocalPort() override;

    ja_iot::base::IpAddrFamily GetAddrFamily() override;


    SOCKET getSocket() { return ( socket_fd_ ); }
    void   setSocket( SOCKET socket_fd ) { socket_fd_ = socket_fd; }

  private:
    SOCKET         socket_fd_      = INVALID_SOCKET;
    ja_iot::base::IpAddrFamily   ip_addr_family_ = ja_iot::base::IpAddrFamily::IPV4;
};
}
}
#endif //#ifdef _OS_WINDOWS_
#endif /* UDPSOCKETIMPLWINDOWS_H_ */
