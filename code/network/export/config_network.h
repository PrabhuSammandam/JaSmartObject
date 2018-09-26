/*
 * network_config.h
 *
 *  Created on: Sep 12, 2017
 *      Author: psammand
 */

#ifndef NETWORK_EXPORT_CONFIG_NETWORK_H_
#define NETWORK_EXPORT_CONFIG_NETWORK_H_

#include <cstdint>

#if defined ( _OS_WINDOWS_ ) || defined ( _OS_LINUX_ )
constexpr uint16_t          COAP_MAX_PDU_SIZE = 1400;
#elif defined ( _OS_FREERTOS_ )
constexpr uint16_t          COAP_MAX_PDU_SIZE = 320;
#endif

constexpr uint16_t          MAX_NO_OF_ADAPTER_TYPES = 6;

constexpr uint16_t          MSG_PROCESSOR_RECEIVE_MAX_NO_MSGS = 10;

constexpr uint16_t          MSG_PROCESSOR_SENDER_TASK_MAX_NO_MSGS = 10;
constexpr uint16_t          MSG_PROCESSOR_SENDER_TASK_STACK_SIZE  = 0;
constexpr uint16_t          MSG_PROCESSOR_SENDER_TASK_PRIORITY    = 0;
constexpr const char *const MSG_PROCESSOR_SENDER_TASK_NAME        = "msg_prossr";

constexpr uint16_t          IP_ADAPTER_SENDER_TASK_MAX_NO_MSGS = 10;
constexpr uint16_t          IP_ADAPTER_SENDER_TASK_STACK_SIZE  = 0;
constexpr uint16_t          IP_ADAPTER_SENDER_TASK_PRIORITY    = 0;
constexpr const char *const IP_ADAPTER_SENDER_TASK_NAME        = "ip_s_task";

constexpr uint16_t          IP_ADAPTER_RECEIVER_TASK_MAX_NO_MSGS = 10;
constexpr uint16_t          IP_ADAPTER_RECEIVER_TASK_STACK_SIZE  = 0;
constexpr uint16_t          IP_ADAPTER_RECEIVER_TASK_PRIORITY    = 0;
constexpr const char *const IP_ADAPTER_RECEIVER_TASK_NAME        = "ip_r_task";

constexpr uint16_t          JA_COAP_CONFIG_MAX_PAYLOAD_SIZE = 512;
constexpr uint16_t          JA_COAP_CONFIG_MAX_OPTION_SIZE  = 1034;

constexpr uint16_t          k_resource_uri_path_max_size = 128;


#define ENABLE_IPV4_UNICAST_SECURE
#define ENABLE_IPV4_MULTICAST
#define ENABLE_IPV4_MULTICAST_SECURE

#define ENABLE_IPV6_UNICAST
#define ENABLE_IPV6_UNICAST_SECURE
#define ENABLE_IPV6_MULTICAST
#define ENABLE_IPV6_MULTICAST_SECURE

#endif /* NETWORK_EXPORT_CONFIG_NETWORK_H_ */