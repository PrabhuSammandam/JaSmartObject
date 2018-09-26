/*
 * ISocket.h
 *
 *  Created on: Jul 15, 2017
 *      Author: psammand
 */

#pragma once

#include <stdint.h>
#include <ip_addr.h>

namespace ja_iot
{
  namespace network
  {
    enum class SocketError
    {
      OK,
      ERR,
      BIND_FAILED,
      SEND_FAILED,
      OPTION_SET_FAILED,
      SOCKET_NOT_VALID
    };

    /**
     * Base interface for the UDP socket. Each platform need to implement the virtual functions.
     */
    class IUdpSocket
    {
    public:
      IUdpSocket()
      {
      }

      virtual ~IUdpSocket()
      {
      }

      virtual SocketError OpenSocket(base::IpAddrFamily ip_addr_family = base::IpAddrFamily::IPv4) = 0;
      virtual SocketError BindSocket(base::IpAddress& ip_address, uint16_t port = 0) = 0;
      virtual SocketError CloseSocket() = 0;

      virtual SocketError JoinMulticastGroup(base::IpAddress& group_address, uint32_t if_index) = 0;
      virtual SocketError LeaveMulticastGroup(base::IpAddress& group_address, uint32_t if_index) = 0;

      virtual SocketError SelectMulticastInterface(base::IpAddress& group_address, uint32_t if_index) = 0;

      virtual SocketError EnableMulticastLoopback(bool is_enabled) = 0;
      virtual SocketError EnableReuseAddr(bool is_enabled) = 0;
      virtual SocketError EnableIpv6Only(bool is_enabled) = 0;
      virtual SocketError EnablePacketInfo(bool is_enabled) = 0;

      virtual SocketError SetBlocking(bool is_blocked) = 0;

      virtual SocketError ReceiveData(base::IpAddress& remote_addr, uint16_t& port, uint8_t* data,
                                      int16_t& data_length) = 0;

      /**
       * Send the msg_data through this socket. The remote address need to be passed.
       * @param remote_addr	- the remote address to send the msg_data. The address should be in numeric form
       * @param port			- remote port
       * @param data			- msg_data to send
       * @param data_length	- length of msg_data to send
       * @return
       */
      virtual SocketError SendData(base::IpAddress& remote_addr, uint16_t port, uint8_t* data,
                                   uint16_t data_length) = 0;

      /**
       * Get the local port which this socket is bind. During bind if the port passed is 0 then the
       * system will allocated the unoccupied port to the socket. This function will be used in that
       * situation to get the automatically allocated port.
       * @return
       */
      virtual uint16_t GetLocalPort() = 0;

      /**
       * Returns the ip address family this socket was created. All sockets should contain the ip address
       * which will be set during the construction of UDP socket. The valid address family are IPV4 & IPV6.
       * @return
       */
      virtual base::IpAddrFamily GetAddrFamily() = 0;

      uint16_t get_flags() const { return _flags; }
      void set_flags(const uint16_t flags) { this->_flags = flags; }

    private:
      uint16_t _flags = 0;
    };
  }
}
