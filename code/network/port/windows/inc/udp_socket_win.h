/*
 * SocketImplWindows.h
 *
 *  Created on: Jul 15, 2017
 *      Author: psammand
 */

#pragma once

#ifdef _OS_WINDOWS_

#include <i_udp_socket.h>
#include <winsock2.h>

namespace ja_iot
{
  namespace network
  {
    class UdpSocketImplWindows : public IUdpSocket
    {
    public:

      UdpSocketImplWindows();

      SocketError OpenSocket(base::IpAddrFamily ip_addr_family = base::IpAddrFamily::IPv4) override;
      SocketError BindSocket(base::IpAddress& ip_address, uint16_t port = 0) override;
      SocketError CloseSocket() override;

      SocketError JoinMulticastGroup(base::IpAddress& group_address, uint32_t if_index) override;
      SocketError LeaveMulticastGroup(base::IpAddress& group_address, uint32_t if_index) override;

      SocketError SelectMulticastInterface(base::IpAddress& group_address, uint32_t if_index) override;

      SocketError ReceiveData(base::IpAddress& remote_addr, uint16_t& port, uint8_t* data, int16_t& data_length)
      override;
      SocketError SendData(base::IpAddress& remote_addr, uint16_t port, uint8_t* data, uint16_t data_length) override;

      SocketError EnableMulticastLoopback(bool is_enabled) override;
      SocketError EnableReuseAddr(bool is_enabled) override;
      SocketError EnableIpv6Only(bool is_enabled) override;
      SocketError EnablePacketInfo(bool is_enabled) override;

      SocketError SetBlocking(bool is_blocked) override;

      uint16_t GetLocalPort() override;

      base::IpAddrFamily GetAddrFamily() override;


      SOCKET get_socket() const { return _u32_socket_fd; }
      void set_socket(const SOCKET socket_fd) { _u32_socket_fd = socket_fd; }

    private:
      SOCKET _u32_socket_fd = INVALID_SOCKET;
      base::IpAddrFamily _e_addr_family = base::IpAddrFamily::IPv4;
    };
  }
}
#endif //#ifdef _OS_WINDOWS_
