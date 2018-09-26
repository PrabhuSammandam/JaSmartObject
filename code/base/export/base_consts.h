/*
 * base_consts.h
 *
 *  Created on: Sep 25, 2017
 *      Author: psammand
 */

#pragma once

#include <cstdint>

namespace ja_iot {
namespace base {
constexpr uint16_t k_network_flag_none              = 0x0000;
constexpr uint16_t k_network_flag_scope_if_local    = 0x0001;
constexpr uint16_t k_network_flag_scope_link_local  = 0x0002;
constexpr uint16_t k_network_flag_scope_realm_local = 0x0003;
constexpr uint16_t k_network_flag_scope_admin_local = 0x0004;
constexpr uint16_t k_network_flag_scope_site_local  = 0x0005;
constexpr uint16_t k_network_flag_scope_org_local   = 0x0008;
constexpr uint16_t k_network_flag_scope_global      = 0x000E;
constexpr uint16_t k_network_flag_ipv4              = 0x0010;
constexpr uint16_t k_network_flag_ipv6              = 0x0020;
constexpr uint16_t k_network_flag_secure            = 0x0040;
constexpr uint16_t k_network_flag_multicast         = 0x0080;

constexpr uint16_t k_network_flag_ipv6_secure_mcast = k_network_flag_ipv6 | k_network_flag_secure | k_network_flag_multicast;
constexpr uint16_t k_network_flag_ipv6_secure_ucast = k_network_flag_ipv6 | k_network_flag_secure;
constexpr uint16_t k_network_flag_ipv6_mcast        = k_network_flag_ipv6 | k_network_flag_multicast;

constexpr uint16_t k_network_flag_ipv4_secure_mcast = k_network_flag_ipv4 | k_network_flag_secure | k_network_flag_multicast;
constexpr uint16_t k_network_flag_ipv4_secure_ucast = k_network_flag_ipv4 | k_network_flag_secure;
constexpr uint16_t k_network_flag_ipv4_mcast        = k_network_flag_ipv4 | k_network_flag_multicast;

constexpr uint16_t k_adapter_type_default       = 0x0000;
constexpr uint16_t k_adapter_type_ip            = 0x0001;
constexpr uint16_t k_adapter_type_tcp           = 0x0002;
constexpr uint16_t k_adapter_type_nfc           = 0x0004;
constexpr uint16_t k_adapter_type_remote_access = 0x0010;
constexpr uint16_t k_adapter_type_bt_le         = 0x0020;
constexpr uint16_t k_adapter_type_bt_edr        = 0x0040;
constexpr uint16_t k_adapter_type_all           = 0xFFFF;

constexpr uint16_t k_transport_scheme_none        = 0x0000;
constexpr uint16_t k_transport_scheme_coap        = 0x0001;
constexpr uint16_t k_transport_scheme_coaps       = 0x0002;
constexpr uint16_t k_transport_scheme_coap_tcp    = 0x0004;
constexpr uint16_t k_transport_scheme_coaps_tcp   = 0x0008;
constexpr uint16_t k_transport_scheme_http        = 0x0010;
constexpr uint16_t k_transport_scheme_https       = 0x0020;
constexpr uint16_t k_transport_scheme_coap_rfcomm = 0x0040;
constexpr uint16_t k_transport_scheme_coap_gatt   = 0x0080;
constexpr uint16_t k_transport_scheme_coap_nfc    = 0x0100;
constexpr uint16_t k_transport_scheme_coap_ra     = 0x0200;
constexpr uint16_t k_transport_scheme_all         = 0xFFFF;

constexpr uint16_t k_coap_port        = 5683;
constexpr uint16_t k_coap_secure_port = 5684;
}
}