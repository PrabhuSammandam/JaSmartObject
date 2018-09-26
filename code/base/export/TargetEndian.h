/*
 * Endian.h
 *
 *  Created on: Jul 25, 2017
 *      Author: psammand
 */

#ifndef TARGETENDIAN_H_
#define TARGETENDIAN_H_

// can be used for short, unsigned short, word, unsigned word (2-byte types)
template <typename T1>
constexpr auto BYTESWAP16(T1&& n)
-> decltype((((n & 0xFF00) >> 8) | ((n & 0x00FF) << 8)))
{
  return ((((n & 0xFF00) >> 8) | ((n & 0x00FF) << 8)));
}

// can be used for int or unsigned int or float (4-byte types)
template <typename T1>
constexpr auto BYTESWAP32(T1&& n)
-> decltype(((BYTESWAP16((n & 0xFFFF0000) >> 16)) |
  ((BYTESWAP16(n & 0x0000FFFF)) << 16)))
{
  return (((BYTESWAP16((n & 0xFFFF0000) >> 16)) |
    ((BYTESWAP16(n & 0x0000FFFF)) << 16)));
}

// can be used for unsigned long long or double (8-byte types)
template <typename T1>
constexpr auto BYTESWAP64(T1&& n)
-> decltype(((BYTESWAP32((n & 0xFFFFFFFF00000000) >> 32)) |
  ((BYTESWAP32(n & 0x00000000FFFFFFFF)) << 32)))
{
  return (((BYTESWAP32((n & 0xFFFFFFFF00000000) >> 32)) |
    ((BYTESWAP32(n & 0x00000000FFFFFFFF)) << 32)));
}

#ifdef _BIG_ENDIAN_
template <typename T1>
inline constexpr auto hton16(T1 &&__x__) -> decltype(__x__) {
  return (__x__);
}
template <typename T1>
inline constexpr auto hton32(T1 &&__x__) -> decltype(__x__) {
  return (__x__);
}
template <typename T1>
inline constexpr auto hton64(T1 &&__x__) -> decltype(__x__) {
  return (__x__);
}

template <typename T1>
inline constexpr auto ntoh16(T1 &&__x__) -> decltype(__x__) {
  return (__x__);
}
template <typename T1>
inline constexpr auto ntoh32(T1 &&__x__) -> decltype(__x__) {
  return (__x__);
}
template <typename T1>
inline constexpr auto ntoh64(T1 &&__x__) -> decltype(__x__) {
  return (__x__);
}
#else
#ifdef _LITTLE_ENDIAN_
template <typename T1>
constexpr auto hton16(T1&& __x__) -> decltype(BYTESWAP16(__x__))
{
  return (BYTESWAP16(__x__));
}

template <typename T1>
constexpr auto hton32(T1&& __x__) -> decltype(BYTESWAP32(__x__))
{
  return (BYTESWAP32(__x__));
}

template <typename T1>
constexpr auto hton64(T1&& __x__) -> decltype(BYTESWAP64(__x__))
{
  return (BYTESWAP64(__x__));
}

template <typename T1>
constexpr auto ntoh16(T1&& __x__) -> decltype(BYTESWAP16(__x__))
{
  return (BYTESWAP16(__x__));
}

template <typename T1>
constexpr auto ntoh32(T1&& __x__) -> decltype(BYTESWAP32(__x__))
{
  return (BYTESWAP32(__x__));
}

template <typename T1>
constexpr auto ntoh64(T1&& __x__) -> decltype(BYTESWAP64(__x__))
{
  return (BYTESWAP64(__x__));
}
#else
#err "type of endian not defined"
#endif
#endif

#endif /* TARGETENDIAN_H_ */
