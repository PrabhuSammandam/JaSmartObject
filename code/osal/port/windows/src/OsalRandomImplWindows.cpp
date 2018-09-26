#ifdef _OS_WINDOWS_

#include <windows.h>
#include <common/inc/OsalRandomImpl.h>

namespace ja_iot {
namespace osal {
OsalError OsalRandomImpl::get_random_bytes( uint8_t *random_buffer, uint16_t buffer_len )
{
  if( ( random_buffer == nullptr ) || ( buffer_len == 0 ) )
  {
    return ( OsalError::ERR );
  }

  NTSTATUS status = BCryptGenRandom( NULL, random_buffer, (ULONG) buffer_len, BCRYPT_USE_SYSTEM_PREFERRED_RNG );

  if( !BCRYPT_SUCCESS( status ) )
  {
    return ( OsalError::ERR );
  }

  return ( OsalError::OK );
}
}
}
#endif /* _OS_WINDOWS_ */