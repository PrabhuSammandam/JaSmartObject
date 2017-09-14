/*
 * IpAdapterImplEsp8266.h
 *
 *  Created on: Sep 11, 2017
 *      Author: psammand
 */

#ifndef NETWORK_PORT_ESP8266_INC_IPADAPTERIMPLESP8266_H_
#define NETWORK_PORT_ESP8266_INC_IPADAPTERIMPLESP8266_H_

#include <IpAdapterBase.h>
#include <IUdpSocket.h>

#ifdef _OS_FREERTOS_

namespace ja_iot {
namespace network {
class IpAdapterImplEsp8266 : public IAdapter
{
  public:

    IpAdapterImplEsp8266 ();

    ~IpAdapterImplEsp8266 ();

    ErrCode Initialize() override;
    ErrCode Terminate()  override;

    ErrCode StartAdapter() override;
    ErrCode StopAdapter()  override;

    ErrCode StartServer() override;
    ErrCode StopServer()  override;

    ErrCode StartListening() override;
    ErrCode StopListening()  override;

    int32_t SendUnicastData( Endpoint  &end_point, const uint8_t *data, uint16_t data_length ) override;

    int32_t SendMulticastData( Endpoint  &end_point, const uint8_t *data, uint16_t data_length ) override;

    void ReadData() override;

    AdapterType GetType() override;

    void SetAdapterHandler( IAdapterEventHandler *adapter_event_handler ) override;

  private:
    ErrCode OpenSocket( IpAddrFamily ip_addr_family, bool is_multicast, IUdpSocket *udp_socket, uint16_t port );
    ErrCode OpenSocket2( IpAddrFamily ip_addr_family, bool is_multicast, IUdpSocket *udp_socket, uint16_t &port );
    int32_t send_data( Endpoint &end_point, const uint8_t *data, uint16_t data_length, bool is_multicast ) ;
  private:
    uint16_t               ipv4_ucast_port_   = 0;
    IAdapterEventHandler * p_adapter_handler_ = nullptr;
    IUdpSocket *           ipv4_ucast_socket_ = nullptr;
    IUdpSocket *           ipv4_mcast_socket_ = nullptr;
};
}  // namespace network
}  // namespace ja_iot

#endif /* _OS_FREERTOS_ */

#endif /* NETWORK_PORT_ESP8266_INC_IPADAPTERIMPLESP8266_H_ */
