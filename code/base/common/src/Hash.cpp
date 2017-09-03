/*
 * Hash.cpp
 *
 *  Created on: Jul 25, 2017
 *      Author: psammand
 */

#include "Hash.h"

namespace ja_iot {
namespace base {
constexpr uint32_t FNV_32_PRIME = 0x01000193;

uint32_t Hash::get_hash(void *buf, uint32_t buf_len, uint32_t hash_value) {
  auto bp = static_cast<unsigned char *>(buf); /* start of buffer */
  auto be = bp + buf_len;                      /* beyond end of buffer */

  /*FNV-1 hash each octet in the buffer */
  while (bp < be) {
    /* xor the bottom with the current octet */
    hash_value ^= uint32_t(*bp++);

/* multiply by the 32 bit FNV magic prime mod 2^32 */
#if defined(NO_FNV_GCC_OPTIMIZATION)
    hash_value *= FNV_32_PRIME;
#else
    hash_value += (hash_value << 1) + (hash_value << 4) + (hash_value << 7) +
                  (hash_value << 8) + (hash_value << 24);
#endif
  }

  /* return our new hash value */
  return (hash_value);
}

uint32_t Hash::get_hash(void *buf, uint32_t buf_len) {
  return (Hash::get_hash(buf, buf_len, HASH_INIT_VALUE));
}
}
}
