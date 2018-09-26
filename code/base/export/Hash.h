/*
 * Hash.h
 *
 *  Created on: Jul 25, 2017
 *      Author: psammand
 */

#pragma once

#include <cstdint>
#include <cstddef>
#include <functional>

namespace ja_iot {
namespace base {
constexpr uint32_t HASH_INIT_VALUE = 0x811c9dc5;

class Hash
{
  public:
    static uint32_t get_hash( void *buf, uint32_t buf_len, uint32_t hash_value );
    static uint32_t get_hash( void *buf, uint32_t buf_len );
};

template<typename T, typename... Rest>
inline void hash_combine( std::size_t &seed, const T &v, Rest... rest )
{
  std::hash<T> hasher;
  seed ^= hasher( v ) + 0x9e3779b9 + ( seed << 6 ) + ( seed >> 2 );
  hash_combine( seed, rest... );
}
}
}