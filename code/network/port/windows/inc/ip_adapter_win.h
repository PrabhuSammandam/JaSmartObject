/*
 * IpAdapterWindows.h
 *
 *  Created on: Jul 3, 2017
 *      Author: psammand
 */

#ifndef NETWORK_PORT_WINDOWS_INC_IP_ADAPTER_WIN_H_
#define NETWORK_PORT_WINDOWS_INC_IP_ADAPTER_WIN_H_

#ifdef _OS_WINDOWS_

#include <i_adapter.h>
#include <common/inc/ip_adapter_base.h>
#include <mswsock.h>
#include <psdk_inc/_socket_types.h>
#include <data_types.h>
#include <winnt.h>
#include <winsock2.h>
#include <cstdint>

namespace ja_iot {
namespace network {
class UdpSocketImplWindows;
} /* namespace network */
} /* namespace ja_iot */

namespace ja_iot {
namespace network {
constexpr uint16_t MAX_NO_OF_WSA_HANDLES = 10;

class IpAdapterImplWindows : public IpAdapterBase
{
  public:

    IpAdapterImplWindows ();

    ~IpAdapterImplWindows ();

    void ReadData() override;

  protected:
    ErrCode DoPreIntialize() override;
    ErrCode DoPostStartServer() override;
    ErrCode DoPreStopServer();

    void DoHandleSendMsg( IpAdapterQMsg *msg ) override;
    void DoHandleReceive() override;
    void DoInitFastShutdownMechanism() override;

    void DoInitAddressChangeNotifyMechanism() override;
    void DoUnInitAddressChangeNotifyMechanism() override;

    ErrCode DoHandleInterfaceEvent( InterfaceEvent *interface_event ) override;

  private:
    void     AddSocketToEventArray( UdpSocketImplWindows *ip_socket );
    void     HandleAddressChangeEvent();
    void     HandleReceivedSocketData( SOCKET socket_fd, uint16_t network_flag );
    uint16_t GetNetworkFlagForSocket( SOCKET socket_fd );
    ErrCode  ConvertNameToAddr( const char *host, uint16_t port, struct sockaddr_storage *sockaddr );
    void     SendData( UdpSocketImplWindows *udp_socket, Endpoint &endpoint, const uint8_t *data, const uint16_t data_length );

    bool IsBitSetInNetworkFlag( uint16_t value, uint16_t mask )
    {
      return ( ( (uint16_t) value & (uint16_t) mask ) != 0 );
    }

  private: // variables
    LPFN_WSARECVMSG   wsa_recv_msg_ = nullptr;   /**< Win32 function pointer to WSARecvMsg() */
    WSAEVENT          addr_change_event_; /**< Event used to signal address changes */
    WSAEVENT          shutdown_event_; /**< Event used to signal threads to stop */
    SOCKET            socket_fd_array_[MAX_NO_OF_WSA_HANDLES];
    HANDLE            wsa_events_array_[MAX_NO_OF_WSA_HANDLES];
    uint16_t          wsa_events_count_ = 0;
};
}
}
#endif // #ifdef _OS_WINDOWS_
#endif /* NETWORK_PORT_WINDOWS_INC_IP_ADAPTER_WIN_H_ */
