#include "OsalRandom.h"
#include "common/inc/OsalRandomImpl.h"

namespace ja_iot {
namespace osal {
/* Return the number of leading zeroes in x.
 * Binary search algorithm from Section 5-3 of:
 *     H.S. Warren Jr. Hacker's Delight. Addison-Wesley. 2003.
 */
static int nlz( uint32_t x )
{
  if( x == 0 )
  {
    return ( 32 );
  }

  int n = 0;

  if( x <= 0x0000FFFF )
  {
    n = n + 16; x = x << 16;
  }

  if( x <= 0x00FFFFFF )
  {
    n = n + 8; x = x << 8;
  }

  if( x <= 0x0FFFFFFF )
  {
    n = n + 4; x = x << 4;
  }

  if( x <= 0x3FFFFFFF )
  {
    n = n + 2; x = x << 2;
  }

  if( x <= 0x7FFFFFFF )
  {
    n = n + 1;
  }

  return ( n );
}

uint32_t OsalRandom::get_random()
{
  uint32_t random{};

  get_random_bytes( (uint8_t *) &random, sizeof( uint32_t ) );

  return ( random );
}

uint32_t OsalRandom::get_random_range( uint32_t range_first, uint32_t range_second )
{
  if( range_first == range_second )
  {
    return ( range_second );
  }

  uint32_t rangeBase  = ( range_first < range_second ) ? range_first : range_second;
  uint32_t rangeWidth = ( range_first > range_second ) ? ( range_first - range_second ) : ( range_second - range_first );

  /*
   * Compute a random number between 0 and rangeWidth. Avoid using floating
   * point types to avoid overflow when rangeWidth is large. The condition
   * in the while loop will be false with probability at least 1/2.
   */
  uint32_t rangeMask = 0xFFFFFFFF >> nlz( rangeWidth );
  uint32_t offset    = 0;

  do
  {
    if( get_random_bytes( (uint8_t *) &offset, sizeof( offset ) ) != OsalError::OK )
    {
      return ( rangeBase );
    }

    offset = offset & rangeMask;
  } while( offset > rangeWidth );

  return ( rangeBase + offset );
}

OsalError OsalRandom::get_random_bytes(uint8_t *random_buffer, uint16_t buffer_len)
{
	return OsalRandomImpl::get_random_bytes(random_buffer, buffer_len);
}
}
}
