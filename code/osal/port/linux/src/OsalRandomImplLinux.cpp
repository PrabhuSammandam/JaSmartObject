/*
 * OsalRandomImplLinux.cpp
 *
 *  Created on: Jan 26, 2018
 *      Author: psammand
 */
#include <cstdio>

#ifdef _OS_LINUX_

#include "common/inc/OsalRandomImpl.h"

namespace ja_iot {
namespace osal {
OsalError OsalRandomImpl::get_random_bytes( uint8_t *random_buffer, uint16_t buffer_len )
{
  if( ( random_buffer == nullptr ) || ( buffer_len == 0 ) )
  {
    return ( OsalError::ERR );
  }

  FILE *urandom = fopen( "/dev/urandom", "r" );

  if( urandom == NULL )
  {
    return ( OsalError::ERR );
  }

  if( fread( random_buffer, sizeof( uint8_t ), buffer_len, urandom ) != buffer_len )
  {
    fclose( urandom );
    return ( OsalError::ERR );
  }

  fclose( urandom );

  return ( OsalError::OK );
}
}
}
#endif /* _OS_WINDOWS_ */