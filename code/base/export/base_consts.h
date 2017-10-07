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
constexpr uint16_t kNetworkFlag_none              = 0x0000;
constexpr uint16_t kNetworkFlag_scope_if_local    = 0x0001;
constexpr uint16_t kNetworkFlag_scope_link_local  = 0x0002;
constexpr uint16_t kNetworkFlag_scope_realm_local = 0x0003;
constexpr uint16_t kNetworkFlag_scope_admin_local = 0x0004;
constexpr uint16_t kNetworkFlag_scope_site_local  = 0x0005;
constexpr uint16_t kNetworkFlag_scope_org_local   = 0x0008;
constexpr uint16_t kNetworkFlag_scope_global      = 0x000E;
constexpr uint16_t kNetworkFlag_ipv4              = 0x0010;
constexpr uint16_t kNetworkFlag_ipv6              = 0x0020;
constexpr uint16_t kNetworkFlag_secure            = 0x0040;
constexpr uint16_t kNetworkFlag_multicast         = 0x0080;

constexpr uint16_t kAdapterType_default       = 0x0000;
constexpr uint16_t kAdapterType_ip            = 0x0001;
constexpr uint16_t kAdapterType_tcp           = 0x0002;
constexpr uint16_t kAdapterType_nfc           = 0x0004;
constexpr uint16_t kAdapterType_remote_access = 0x0010;
constexpr uint16_t kAdapterType_bt_le         = 0x0020;
constexpr uint16_t kAdapterType_bt_edr        = 0x0040;
constexpr uint16_t kAdapterType_all           = 0xFFFF;

constexpr uint16_t k_coap_port        = 5683;
constexpr uint16_t k_coap_secure_port = 5684;
}
}