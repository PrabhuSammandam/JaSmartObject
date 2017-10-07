/*
 * IpAddress.cpp
 *
 *  Created on: Jul 15, 2017
 *      Author: psammand
 */
#include <ip_addr.h>
#include <TargetEndian.h>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace ja_iot {
namespace base {
static int inet_pton4( const char *src, uint8_t *dst, int pton );
static int inet_pton6( const char *src, uint8_t *dst );

IpAddress::IpAddress( IpAddrFamily ip_address_family )
  : address_family_{ ip_address_family }
{
}

IpAddress::IpAddress( const char *ascii_addr, IpAddrFamily ip_address_family )
  : address_family_{ ip_address_family }
{
  if( ascii_addr != nullptr )
  {
    strcpy( (char *) &address_[0], ascii_addr );
  }
}

IpAddress::IpAddress( const IpAddress &other )
{
  this->address_family_ = other.address_family_;
  this->scope_id_       = other.scope_id_;

  for( int i = 0; i < 16; ++i )
  {
    address_[i] = other.address_[i];
  }
}

IpAddress::IpAddress( uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4 )
{
  address_[0] = byte1;
  address_[1] = byte2;
  address_[2] = byte3;
  address_[3] = byte4;
  address_[4] = '\0';

  address_family_ = IpAddrFamily::IPV4;
}

IpAddress::IpAddress( Ipv6AddrScope scope, uint16_t address ) : address_family_{ IpAddrFamily::IPv6 }, scope_id_{ scope }
{
  set_addr_by_scope( scope, address );
}

IpAddress::~IpAddress ()
{
}

bool IpAddress::is_broadcast()
{
  return ( false );
}

bool IpAddress::is_multicast()
{
  return ( false );
}

bool IpAddress::is_unicast()
{
  return ( false );
}

bool IpAddress::is_ipv4()
{
  return ( address_family_ == IpAddrFamily::IPV4 );
}

bool IpAddress::is_ipv6()
{
  return ( address_family_ == IpAddrFamily::IPv6 );
}

/***
 * Returns the IPV4 address in host endian.
 * @return
 */
uint32_t IpAddress::as_u32()
{
  return ( static_cast<uint32_t>( address_[0] << 24 | address_[1] << 16 | address_[2] << 8 | address_[3] ) );
}

void IpAddress::set_addr_by_scope( uint8_t scope_id, uint16_t address )
{
  for( int i = 0; i < 16; ++i )
  {
    address_[i] = 0;
  }

  address_[0]  = 0xFF;
  address_[1]  = ( scope_id & 0xFF );
  address_[14] = (uint8_t) ( ( address >> 8 ) & 0xFF );
  address_[15] = (uint8_t) address & 0xFF;
}

Ipv6AddrScope IpAddress::get_addr_scope( const char *ipv6_ascii_addr )
{
  if( ipv6_ascii_addr != nullptr )
  {
  }

  return ( Ipv6AddrScope::NONE );
}

bool IpAddress::from_string( const char *addr_string, IpAddrFamily ip_addr_family, IpAddress &ip_address )
{
  if( ip_addr_family == IpAddrFamily::IPV4 )
  {
    return ( inet_pton4( addr_string, ip_address.get_addr(), 1 ) == 1 );
  }
  else
  {
    return ( inet_pton6( addr_string, ip_address.get_addr() ) == 1 );
  }
}

/* int
 * inet_pton4(src, dst, pton)
 *      when last arg is 0: inet_aton(). with hexadecimal, octal and shorthand.
 *      when last arg is 1: inet_pton(). decimal dotted-quad only.
 * return:
 *      1 if `src' is a valid input, else 0.
 * notice:
 *      does not touch `dst' unless it's returning 1.
 * author:
 *      Paul Vixie, 1996.
 */
static int inet_pton4( const char *src, uint8_t *dst, int pton )
{
  uint32_t           val = 0;
  uint32_t           digit = 0;
  int                base, n;
  unsigned char      c;
  uint32_t           parts[4] = { 0 };
  register uint32_t *pp       = parts;

  c = *src;

  for(;; )
  {
    /*
     * Collect number up to ``.''.
     * Values are specified as for C:
     * 0x=hex, 0=octal, isdigit=decimal.
     */
    if( !isdigit( c ) )
    {
      return ( 0 );
    }

    val  = 0;
    base = 10;

    if( c == '0' )
    {
      c = *++src;

      if( ( c == 'x' ) || ( c == 'X' ) )
      {
        base = 16, c = *++src;
      }
      else if( isdigit( c ) && ( c != '9' ) )
      {
        base = 8;
      }
    }

    /* inet_pton() takes decimal only */
    if( pton && ( base != 10 ) )
    {
      return ( 0 );
    }

    for(;; )
    {
      if( isdigit( c ) )
      {
        digit = c - '0';

        if( (int) digit >= base )
        {
          break;
        }

        val = ( val * base ) + digit;
        c   = *++src;
      }
      else if( ( base == 16 ) && isxdigit( c ) )
      {
        digit = c + 10 - ( islower( c ) ? 'a' : 'A' );

        if( digit >= 16 )
        {
          break;
        }

        val = ( val << 4 ) | digit;
        c   = *++src;
      }
      else
      {
        break;
      }
    }

    if( c == '.' )
    {
      /*
       * Internet format:
       *      a.b.c.d
       *      a.b.c   (with c treated as 16 bits)
       *      a.b     (with b treated as 24 bits)
       *      a       (with a treated as 32 bits)
       */
      if( pp >= parts + 3 )
      {
        return ( 0 );
      }

      *pp++ = val;
      c     = *++src;
    }
    else
    {
      break;
    }
  }

  /*
   * Check for trailing characters.
   */
  if( ( c != '\0' ) && !isspace( c ) )
  {
    return ( 0 );
  }

  /*
   * Concoct the address according to
   * the number of parts specified.
   */
  n = pp - parts + 1;

  /* inet_pton() takes dotted-quad only.  it does not take shorthand. */
  if( pton && ( n != 4 ) )
  {
    return ( 0 );
  }

  switch( n )
  {
    case 0:
    {
      return ( 0 ); /* initial nondigit */
    }
    case 1: /* a -- 32 bits */
    {
    } break;

    case 2: /* a.b -- 8.24 bits */
    {
      if( ( parts[0] > 0xff ) || ( val > 0xffffff ) )
      {
        return ( 0 );
      }

      val |= parts[0] << 24;
    } break;

    case 3: /* a.b.c -- 8.8.16 bits */
    {
      if( ( ( parts[0] | parts[1] ) > 0xff ) || ( val > 0xffff ) )
      {
        return ( 0 );
      }

      val |= ( parts[0] << 24 ) | ( parts[1] << 16 );
    } break;

    case 4: /* a.b.c.d -- 8.8.8.8 bits */
    {
      if( ( parts[0] | parts[1] | parts[2] | val ) > 0xff )
      {
        return ( 0 );
      }

      val |= ( parts[0] << 24 ) | ( parts[1] << 16 ) | ( parts[2] << 8 );
    } break;
  }

  if( dst )
  {
    val = hton32( val );
    memcpy( dst, &val, 4 );
  }

  return ( 1 );
}

/* int
 * inet_pton6(src, dst)
 *      convert presentation level address to network order binary form.
 * return:
 *      1 if `src' is a valid [RFC1884 2.2] address, else 0.
 * notice:
 *      (1) does not touch `dst' unless it's returning 1.
 *      (2) :: in a full address is silently ignored.
 * credit:
 *      inspired by Mark Andrews.
 * author:
 *      Paul Vixie, 1996.
 */
static int inet_pton6( const char *src, uint8_t *dst )
{
  static const char xdigits_l[] = "0123456789abcdef";
  static const char xdigits_u[] = "0123456789ABCDEF";
  uint8_t           tmp[16], *tp, *endp, *colonp;
  const char *      xdigits, *curtok;
  int               ch, saw_xdigit;
  uint32_t          val;

  memset( ( tp = tmp ), '\0', 16 );
  endp   = tp + 16;
  colonp = nullptr;

  /* Leading :: requires some special handling. */
  if( *src == ':' )
  {
    if( *++src != ':' )
    {
      return ( 0 );
    }
  }

  curtok     = src;
  saw_xdigit = 0;
  val        = 0;

  while( ( ch = *src++ ) != '\0' )
  {
    const char *pch = nullptr;

    if( ( pch = strchr( ( xdigits = xdigits_l ), ch ) ) == nullptr )
    {
      pch = strchr( ( xdigits = xdigits_u ), ch );
    }

    if( pch != nullptr )
    {
      val <<= 4;
      val  |= ( pch - xdigits );

      if( val > 0xffff )
      {
        return ( 0 );
      }

      saw_xdigit = 1;
      continue;
    }

    if( ch == ':' )
    {
      curtok = src;

      if( !saw_xdigit )
      {
        if( colonp )
        {
          return ( 0 );
        }

        colonp = tp;
        continue;
      }
      else if( *src == '\0' )
      {
        return ( 0 );
      }

      if( tp + 2 > endp )
      {
        return ( 0 );
      }

      *tp++      = (uint8_t) ( val >> 8 ) & 0xff;
      *tp++      = (uint8_t) val & 0xff;
      saw_xdigit = 0;
      val        = 0;
      continue;
    }

    if( ( ch == '.' ) && ( ( tp + 4 ) <= endp ) && ( inet_pton4( curtok, tp, 1 ) > 0 ) )
    {
      tp        += 4;
      saw_xdigit = 0;
      break; /* '\0' was seen by inet_pton4(). */
    }

    return ( 0 );
  }

  if( saw_xdigit )
  {
    if( tp + 2 > endp )
    {
      return ( 0 );
    }

    *tp++ = (uint8_t) ( val >> 8 ) & 0xff;
    *tp++ = (uint8_t) val & 0xff;
  }

  if( colonp != nullptr )
  {
    /*
     * Since some memmove()'s erroneously fail to handle
     * overlapping regions, we'll do the shift by hand.
     */
    const int n = tp - colonp;
    int       i;

    if( tp == endp )
    {
      return ( 0 );
    }

    for( i = 1; i <= n; i++ )
    {
      endp[-i]      = colonp[n - i];
      colonp[n - i] = 0;
    }

    tp = endp;
  }

  if( tp != endp )
  {
    return ( 0 );
  }

  memcpy( dst, tmp, 16 );
  return ( 1 );
}

/* const char *
 * inet_ntop4(src, dst, size)
 *      format an IPv4 address, more or less like inet_ntoa()
 * return:
 *      `dst' (as a const)
 * notes:
 *      (1) uses no statics
 *      (2) takes a uint8_t* not an in_addr as input
 * author:
 *      Paul Vixie, 1996.
 */
static const char* inet_ntop4( const uint8_t *src, char *dst, size_t size )
{
  static const char fmt[]   = "%u.%u.%u.%u";
  char              tmp[15] = { 0 }; // 255.255.255.255

  if( sprintf( (char *) tmp, (const char *) fmt, src[0], src[1], src[2], src[3] ) >
    (int) size )
  {
    return ( nullptr );
  }

  strcpy( dst, tmp );
  return ( dst );
}

/* const char *
 * inet_ntop6(src, dst, size)
 *      convert IPv6 binary address into presentation (printable) format
 * author:
 *      Paul Vixie, 1996.
 */
static const char* inet_ntop6( const uint8_t *src, char *dst, size_t size )
{
  /*
   * Note that int32_t and int16_t need only be "at least" large enough
   * to contain a value of the specified size.  On some systems, like
   * Crays, there is no such thing as an integer variable with 16 bits.
   * Keep this in mind if you think this function should have been coded
   * to use pointer overlays.  All the world's not a VAX.
   */
  char tmp[sizeof "ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255"], *tp;

  struct
  {
    int   base, len;
  } best, cur;

  uint32_t words[16 / 2];
  int      i;

  /*
   * Preprocess:
   *      Copy the input (bytewise) array into a wordwise array.
   *      Find the longest run of 0x00's in src[] for :: shorthanding.
   */
  memset( words, '\0', sizeof words );

  for( i = 0; i < 16; i++ )
  {
    words[i / 2] |= ( src[i] << ( ( 1 - ( i % 2 ) ) << 3 ) );
  }

  best.base = -1;
  cur.base  = -1;

  for( i = 0; i < ( 16 / 2 ); i++ )
  {
    if( words[i] == 0 )
    {
      if( cur.base == -1 )
      {
        cur.base = i, cur.len = 1;
      }
      else
      {
        cur.len++;
      }
    }
    else
    {
      if( cur.base != -1 )
      {
        if( ( best.base == -1 ) || ( cur.len > best.len ) )
        {
          best = cur;
        }

        cur.base = -1;
      }
    }
  }

  if( cur.base != -1 )
  {
    if( ( best.base == -1 ) || ( cur.len > best.len ) )
    {
      best = cur;
    }
  }

  if( ( best.base != -1 ) && ( best.len < 2 ) )
  {
    best.base = -1;
  }

  /*
   * Format the result.
   */
  tp = tmp;

  for( i = 0; i < ( 16 / 2 ); i++ )
  {
    /* Are we inside the best run of 0x00's? */
    if( ( best.base != -1 ) && ( i >= best.base ) && ( i < ( best.base + best.len ) ) )
    {
      if( i == best.base )
      {
        *tp++ = ':';
      }

      continue;
    }

    /* Are we following an initial run of 0x00s or any real hex? */
    if( i != 0 )
    {
      *tp++ = ':';
    }

    /* Is this address an encapsulated IPv4? */
    if( ( i == 6 ) && ( best.base == 0 ) &&
      ( ( best.len == 6 ) || ( ( best.len == 5 ) && ( words[5] == 0xffff ) ) ) )
    {
      if( !inet_ntop4( src + 12, tp, sizeof tmp - ( tp - tmp ) ) )
      {
        return ( nullptr );
      }

      tp += strlen( tp );
      break;
    }

    tp += sprintf( tp, "%x", words[i] );
  }

  /* Was it a trailing run of 0x00's? */
  if( ( best.base != -1 ) && ( ( best.base + best.len ) == ( 16 / 2 ) ) )
  {
    *tp++ = ':';
  }

  *tp++ = '\0';

  /*
   * Check for overflow, copy, and we're done.
   */
  if( (size_t) ( tp - tmp ) > size )
  {
    return ( nullptr );
  }

  strcpy( dst, tmp );
  return ( dst );
}

void IpAddress::set_addr( _in_ uint32_t ipv4_addr )
{
  address_[0] = (uint8_t) ( ( ipv4_addr >> 24 ) & 0xFF );
  address_[1] = (uint8_t) ( ( ipv4_addr >> 16 ) & 0xFF );
  address_[2] = (uint8_t) ( ( ipv4_addr >> 8 ) & 0xFF );
  address_[3] = (uint8_t) ( ipv4_addr & 0xFF );
}

void IpAddress::set_addr( _in_ uint8_t *ip_addr, _in_ IpAddrFamily ip_addr_family )
{
  if( ip_addr_family == IpAddrFamily::IPV4 )
  {
    address_[0] = ip_addr[0];
    address_[1] = ip_addr[1];
    address_[2] = ip_addr[2];
    address_[3] = ip_addr[3];
  }
  else
  {
    memcpy( (void *) &address_[0], (void *) ip_addr, 16 );
  }
}

void IpAddress::to_string( uint8_t *buf, uint8_t buf_len )
{
  buf[0] = '\0';

  if( address_family_ == IpAddrFamily::IPV4 )
  {
    sprintf( (char *) buf, "%d.%d.%d.%d", address_[0], address_[1], address_[2], address_[3] );
  }
  else
  {
    sprintf( (char *) buf, "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x", address_[0], address_[1], address_[2], address_[3],
      address_[4], address_[5], address_[6], address_[7],
      address_[8], address_[9], address_[10], address_[11],
      address_[12], address_[13], address_[14], address_[15] );
  }
}
bool IpAddress::operator == ( const IpAddress &other )
{
  if( this->address_family_ != other.address_family_ )
  {
    return ( false );
  }

  for( auto i = 0; i < 16; i++ )
  {
    if( address_[i] != other.address_[i] )
    {
      return ( false );
    }
  }

  return ( true );
}
}
}