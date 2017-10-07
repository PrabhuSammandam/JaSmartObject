/*
 * konstants.h
 *
 *  Created on: Jul 10, 2017
 *      Author: psammand
 */

#ifndef KONSTANTS_H_
#define KONSTANTS_H_

namespace ja_iot {
namespace network {
constexpr uint16_t          COAP_PORT        = 5683;
constexpr uint16_t          COAP_SECURE_PORT = 5684;

constexpr const char *const IPV4_MULTICAST_ADDRESS                    = "224.0.1.187";
constexpr const char *const IPV6_MULTICAST_ADDRESS_INTERFACE_LOCAL    = "ff01::158";
constexpr const char *const IPV6_MULTICAST_ADDRESS_LINK_LOCAL         = "ff02::158";
constexpr const char *const IPV6_MULTICAST_ADDRESS_REALM_LOCAL        = "ff03::158";
constexpr const char *const IPV6_MULTICAST_ADDRESS_ADMIN_LOCAL        = "ff04::158";
constexpr const char *const IPV6_MULTICAST_ADDRESS_SITE_LOCAL         = "ff05::158";
constexpr const char *const IPV6_MULTICAST_ADDRESS_ORGANIZATION_LOCAL = "ff08::158";
constexpr const char *const IPV6_MULTICAST_ADDRESS_GLOBAL             = "ff0e::158";

constexpr uint16_t JA_COAP_CONFIG_MAX_PAYLOAD_SIZE = 256;
constexpr uint16_t JA_COAP_CONFIG_MAX_OPTION_SIZE = (1034);

constexpr uint16_t k_resource_uri_path_max_size = 128;
}
}




#endif /* KONSTANTS_H_ */
