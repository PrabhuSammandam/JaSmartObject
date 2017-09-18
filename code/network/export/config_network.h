/*
 * network_config.h
 *
 *  Created on: Sep 12, 2017
 *      Author: psammand
 */

#ifndef NETWORK_EXPORT_CONFIG_NETWORK_H_
#define NETWORK_EXPORT_CONFIG_NETWORK_H_

#if defined ( _OS_WINDOWS_ ) || defined ( _OS_LINUX_ )
constexpr uint16_t COAP_MAX_PDU_SIZE = 1400;
#elif defined ( _OS_FREERTOS_ )
constexpr uint16_t COAP_MAX_PDU_SIZE = 320;
#endif

constexpr uint16_t MAX_NO_OF_ADAPTER_TYPES = 6;

constexpr uint16_t MSG_PROCESSOR_TASK_MAX_NO_MSGS = 10;
#define MSG_PROCESSOR_TASK_STACK_SIZE 0
#define MSG_PROCESSOR_TASK_PRIORITY 0
#define MSG_PROCESSOR_TASK_NAME "msg_prossr"

#define ENABLE_IPV4_UNICAST_SECURE
#define ENABLE_IPV4_MULTICAST
#define ENABLE_IPV4_MULTICAST_SECURE

#define ENABLE_IPV6_UNICAST
#define ENABLE_IPV6_UNICAST_SECURE
#define ENABLE_IPV6_MULTICAST
#define ENABLE_IPV6_MULTICAST_SECURE

#endif /* NETWORK_EXPORT_CONFIG_NETWORK_H_ */
