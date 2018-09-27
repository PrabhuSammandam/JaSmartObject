/*
 * IpAdapterImplLinux.h
 *
 *  Created on: 17-Sep-2017
 *      Author: prabhu
 */

#pragma once

#ifdef _OS_LINUX_

#include <common/inc/ip_adapter_base.h>
#include <config_network.h>
#include <ErrCode.h>
#include <interface_addr.h>
#include <stdint.h>
#include <vector>

namespace ja_iot {
namespace network {
class UdpSocketImplLinux;
} /* namespace network */
} /* namespace ja_iot */

namespace ja_iot {
namespace network {
constexpr uint16_t MAX_NO_SOCKET_FD = 10;

class IpAdapterImplLinux : public IpAdapterBase
{
  public:
    IpAdapterImplLinux ();
    ~IpAdapterImplLinux ();

    void          do_init_fast_shutdown_mechanism() override;
    void          do_init_address_change_notify_mechanism() override;
    void          do_un_init_address_change_notify_mechanism() override;
    void          do_handle_send_msg( IpAdapterQMsg *ip_adapter_q_msg ) override;
    void          do_handle_receive() override;
    base::ErrCode do_pre_stop_server() override;
    base::ErrCode do_start_interface_monitor() override;
    base::ErrCode do_stop_interface_monitor() override;

    std::vector<InterfaceAddress *> get_interface_address_for_index( uint8_t u8_index ) override;
    void                            read_data() override {}

  private:
    void                            handle_received_socket_data( int &selected_fd, uint16_t &u16_network_flag );
    void                            send_data( UdpSocketImplLinux *pcz_udp_socket, Endpoint &rcz_endpoint, const uint8_t *pu8_data, const uint16_t u16_data_length ) const;
    std::vector<InterfaceAddress *> get_newly_found_interface_address();
    void							update_max_fd(int new_fd);

    int        netlink_fd      = -1;      /**< netlink */
    int        shutdown_fds[2] = { -1 };    /**< fds used to signal threads to stop */
    int        max_fd          = -1;
    int        socket_fd_array[MAX_NO_SOCKET_FD];
    uint16_t   socket_fd_count = 0;
    uint8_t    _pu8_receive_buffer[COAP_MAX_PDU_SIZE];

    osal::Mutex *                     _access_mutex = nullptr;
    std::vector<InterfaceAddress *>   _interface_addr_list;
};
}  // namespace network
}

#endif /* _OS_LINUX_ */
