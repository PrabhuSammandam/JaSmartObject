/*
 * Hash.h
 *
 *  Created on: Jul 25, 2017
 *      Author: psammand
 */

#ifndef HASH_H_
#define HASH_H_
#include <cstdint>

namespace ja_iot {
namespace base {
constexpr uint32_t HASH_INIT_VALUE = 0x811c9dc5;

class Hash {
public:
  static uint32_t get_hash(void *buf, uint32_t buf_len, uint32_t hash_value);
  static uint32_t get_hash(void *buf, uint32_t buf_len);
};
}
}

#endif /* HASH_H_ */
