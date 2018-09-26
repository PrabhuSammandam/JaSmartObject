#pragma once
#include <cstdint>

constexpr uint64_t CON_EXCHANGE_LIFETIME_US = 10000000;         // 227 seconds - temporary
constexpr uint64_t NON_EXCHANGE_LIFETIME_US = 10000000;         // 140 seconds - temporary

/*
* The maximum size of a resource body (in bytes) that will be accepted
* as the payload of a POST/PUT or the response to a GET request in a
* transparent blockwise transfer.
*/
constexpr uint16_t MAX_RESOURCE_BODY_SIZE = 1024;
/*
* The maximum payload size (in bytes) that can be transferred in a
* single message, i.e. without requiring a blockwise transfer.
*/
constexpr uint16_t MAX_MESSAGE_SIZE = 512;
/**
* The block size (number of bytes) to use when doing a blockwise
* transfer. This value serves as the upper limit for block size in
* blockwise transfers.
*/
constexpr uint16_t PREFERRED_BLOCK_SIZE = 256;
