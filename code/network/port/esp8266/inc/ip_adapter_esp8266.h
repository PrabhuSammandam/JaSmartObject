/*
 * IpAdapterImplEsp8266.h
 *
 *  Created on: Sep 11, 2017
 *      Author: psammand
 */

#pragma once

#include <vector>
#include "i_adapter.h"
#include "config_network.h"
#include "i_udp_socket.h"
#include "Task.h"

#ifdef _OS_FREERTOS_

namespace ja_iot {
namespace network {
class IpAdapterImplEsp8266 : public IAdapter
{
  public:
    IpAdapterImplEsp8266 ();
    ~IpAdapterImplEsp8266 ();

    base::ErrCode initialize() override;
    base::ErrCode terminate()  override;

    base::ErrCode start_adapter() override;
    base::ErrCode stop_adapter()  override;

    base::ErrCode start_server() override;
    base::ErrCode stop_server()  override;

    base::ErrCode start_listening() override;
    base::ErrCode stop_listening()  override;

    int32_t send_unicast_data( Endpoint &end_point, const uint8_t *data, uint16_t data_length ) override;
    int32_t send_multicast_data( Endpoint &end_point, const uint8_t *data, uint16_t data_length ) override;

    void read_data() override;

    uint16_t get_type() override;

    void set_adapter_event_cb( const pfn_adapter_event_cb pfn_adapter_event_callback, void *pv_user_data ) override;

    base::ErrCode get_endpoints_list( std::vector<Endpoint *> &rcz_endpoint_list ) override;

    void receive_task_loop();

  private:
    base::ErrCode open_socket( base::IpAddrFamily ip_addr_family, bool is_multicast, IUdpSocket *udp_socket, uint16_t port );
    base::ErrCode open_socket2( base::IpAddrFamily ip_addr_family, bool is_multicast, IUdpSocket *udp_socket, uint16_t &port );
    int32_t       send_data( Endpoint &end_point, const uint8_t *data, uint16_t data_length, bool is_multicast );
    void          send_packet_received_adapter_event( uint8_t *received_data, int16_t data_length, uint16_t port, base::IpAddress remote_addr, bool is_mcast );

  private:
    pfn_adapter_event_cb   _adapter_event_cb      = nullptr;
    void *                 _adapter_event_cb_data = nullptr;
    IUdpSocket *           _ipv4_ucast_socket     = nullptr;
    IUdpSocket *           _ipv4_mcast_socket     = nullptr;
    ja_iot::osal::Task *   _receive_task          = nullptr;
    bool                   _is_server_started     = false;
    uint8_t                _receive_buffer[COAP_MAX_PDU_SIZE];
};
}  // namespace network
}  // namespace ja_iot

#endif /* _OS_FREERTOS_ */
