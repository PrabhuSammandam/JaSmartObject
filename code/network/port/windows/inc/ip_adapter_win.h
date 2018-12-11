/*
 * IpAdapterWindows.h
 *
 *  Created on: Jul 3, 2017
 *      Author: psammand
 */

#pragma once
#include <network_datatypes.h>

#ifdef _OS_WINDOWS_

#include <memory>
#include <vector>
#include <cstdint>

#include <i_adapter.h>
#include <config_network.h>
#include <common/inc/ip_adapter_base.h>
#include "port/windows/inc/udp_socket_win.h"

#ifdef __GNUC__
#include <winnt.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#include <ws2ipdef.h>

#include <mswsock.h>
#include <psdk_inc/_socket_types.h>
#endif /*__GNUC__*/

#ifdef _MSC_VER
#include <WS2tcpip.h>
#include <MSWSock.h>
#endif /*_MSC_VER*/

namespace ja_iot {
namespace network {
constexpr uint16_t MAX_NO_OF_WSA_HANDLES = 10;

class IpAdapterImplWindows : public IpAdapterBase
{
  public:
    IpAdapterImplWindows ();
    ~IpAdapterImplWindows ();

    void read_data() override;

  protected:
    base::ErrCode do_pre_intialize() override;
    base::ErrCode do_post_start_server() override;
    base::ErrCode do_pre_stop_server() override;

		void do_handle_send_msg(IpAdapterQMsg *msg) override;
    void do_handle_receive() override;
    void do_init_fast_shutdown_mechanism() override;

    void do_init_address_change_notify_mechanism() override;
    void do_un_init_address_change_notify_mechanism() override;

    InterfaceAddressList get_interface_address_for_index( uint8_t u8_index ) override;

  private:
    void                                  add_socket_to_event_array( UdpSocketImplWindows *ip_socket );
    void                                  handle_received_socket_data( SOCKET socket_fd, uint16_t network_flag );
    uint16_t                              get_network_flag_for_socket( SOCKET socket_fd ) const;
    base::ErrCode                         convert_name_to_addr( const char *host, uint16_t port, struct sockaddr_storage *sockaddr );
    void                                  send_data( UdpSocketImplWindows *udp_socket, Endpoint &endpoint, const uint8_t *data, const uint16_t data_length ) const;
//
    void                                  get_interface_address_list( std::vector<InterfaceAddress *> &interface_addr_list );

    LPFN_WSARECVMSG _pfn_wsa_recv_msg_cb{};   /**< Win32 function pointer to WSARecvMsg() */
    WSAEVENT _h_shutdown_event{};   /**< Event used to signal threads to stop */
    OVERLAPPED _interface_monitor_event{};
    SOCKET _interface_monitor_socket{};
    SOCKET   _ah_socket_fd_array[MAX_NO_OF_WSA_HANDLES];
    HANDLE   _ah_wsa_events_array[MAX_NO_OF_WSA_HANDLES];
    uint16_t _u16_wsa_events_count{};
    uint8_t   _pu8_receive_buffer[COAP_MAX_PDU_SIZE];
};
}
}
#endif // #ifdef _OS_WINDOWS_
