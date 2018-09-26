/*
 * coap_consts.h
 *
 *  Created on: Nov 14, 2017
 *      Author: psammand
 */

#pragma once

#include <cstdint>

constexpr uint8_t COAP_MSG_TYPE_CON = 0;
constexpr uint8_t COAP_MSG_TYPE_NON = 1;
constexpr uint8_t COAP_MSG_TYPE_ACK = 2;
constexpr uint8_t COAP_MSG_TYPE_RST = 3;
constexpr uint8_t COAP_MSG_TYPE_NONE = 4;

constexpr uint8_t COAP_MSG_CODE_EMPTY = 0;
constexpr uint8_t COAP_MSG_CODE_GET = 1;
constexpr uint8_t COAP_MSG_CODE_POST = 2;
constexpr uint8_t COAP_MSG_CODE_PUT = 3;
constexpr uint8_t COAP_MSG_CODE_DEL = 4;
constexpr uint8_t COAP_MSG_CODE_CREATED_201 = 65;
constexpr uint8_t COAP_MSG_CODE_DELETED_202 = 66;
constexpr uint8_t COAP_MSG_CODE_VALID_203 = 67;
constexpr uint8_t COAP_MSG_CODE_CHANGED_204 = 68;
constexpr uint8_t COAP_MSG_CODE_CONTENT_205 = 69;
constexpr uint8_t COAP_MSG_CODE_CONTINUE_231 = 95;
constexpr uint8_t COAP_MSG_CODE_BAD_REQUEST_400 = 128;
constexpr uint8_t COAP_MSG_CODE_UNAUTHORIZED_401 = 129;
constexpr uint8_t COAP_MSG_CODE_BAD_OPTION_402 = 130;
constexpr uint8_t COAP_MSG_CODE_FORBIDDEN_403 = 131;
constexpr uint8_t COAP_MSG_CODE_NOT_FOUND_404 = 132;
constexpr uint8_t COAP_MSG_CODE_METHOD_NOT_ALLOWED_405 = 133;
constexpr uint8_t COAP_MSG_CODE_NOT_ACCEPTABLE_406 = 134;
constexpr uint8_t COAP_MSG_CODE_REQUEST_ENTITY_INCOMPLETE_408 = 136;
constexpr uint8_t COAP_MSG_CODE_PRECONDITION_FAILED_412 = 140;
constexpr uint8_t COAP_MSG_CODE_REQUEST_ENTITY_TOO_LARGE_413 = 141;
constexpr uint8_t COAP_MSG_CODE_UNSUPPORTED_CONTENT_FORMAT_415 = 143;
constexpr uint8_t COAP_MSG_CODE_INTERNAL_SERVER_ERROR_500 = 160;
constexpr uint8_t COAP_MSG_CODE_NOT_IMPLEMENTED_501 = 161;
constexpr uint8_t COAP_MSG_CODE_BAD_GATEWAY_502 = 162;
constexpr uint8_t COAP_MSG_CODE_SERVICE_UNAVAILABLE_503 = 163;
constexpr uint8_t COAP_MSG_CODE_GATEWAY_TIMEOUT_504 = 164;
constexpr uint8_t COAP_MSG_CODE_PROXYING_NOT_SUPPORTED_505 = 165;


constexpr uint16_t COAP_CONTENT_FORMAT_NONE = 0xFFFF;
constexpr uint16_t COAP_CONTENT_FORMAT_PLAIN = 0;
constexpr uint16_t COAP_CONTENT_FORMAT_LINK_FORMAT = 40;
constexpr uint16_t COAP_CONTENT_FORMAT_XML = 41;
constexpr uint16_t COAP_CONTENT_FORMAT_OCTET_STREAM = 42;
constexpr uint16_t COAP_CONTENT_FORMAT_EXI = 47;
constexpr uint16_t COAP_CONTENT_FORMAT_JSON = 50;
constexpr uint16_t COAP_CONTENT_FORMAT_CBOR = 60;
constexpr uint16_t COAP_CONTENT_FORMAT_VND_OCF_CBOR = 10000;

constexpr uint16_t COAP_OPTION_IF_MATCH = 1;
constexpr uint16_t COAP_OPTION_URI_HOST = 3;
constexpr uint16_t COAP_OPTION_ETAG = 4;
constexpr uint16_t COAP_OPTION_IF_NONE_MATCH = 5;
constexpr uint16_t COAP_OPTION_OBSERVE = 6;
constexpr uint16_t COAP_OPTION_URI_PORT = 7;
constexpr uint16_t COAP_OPTION_LOCATION_PATH = 8;
constexpr uint16_t COAP_OPTION_URI_PATH = 11;
constexpr uint16_t COAP_OPTION_CONTENT_FORMAT = 12;
constexpr uint16_t COAP_OPTION_MAX_AGE = 14;
constexpr uint16_t COAP_OPTION_URI_QUERY = 15;
constexpr uint16_t COAP_OPTION_ACCEPT = 17;
constexpr uint16_t COAP_OPTION_LOCATION_QUERY = 20;
constexpr uint16_t COAP_OPTION_BLOCK_2 = 23;
constexpr uint16_t COAP_OPTION_BLOCK_1 = 27;
constexpr uint16_t COAP_OPTION_SIZE2 = 28;
constexpr uint16_t COAP_OPTION_PROXY_URI = 35;
constexpr uint16_t COAP_OPTION_PROXU_SCHEME = 39;
constexpr uint16_t COAP_OPTION_SIZE1 = 60;
constexpr uint16_t COAP_OPTION_ACCEPT_VERSION = 2049;
constexpr uint16_t COAP_OPTION_CONTENT_VERSION = 2053;
