#ifdef _OS_FREERTOS_

#include <cstddef>
#include <esp_libc.h>
#include <common/inc/OsalRandomImpl.h>

namespace ja_iot {
namespace osal {
OsalError OsalRandomImpl::get_random_bytes( uint8_t *random_buffer, uint16_t buffer_len )
{
  if( ( random_buffer == nullptr ) || ( buffer_len == 0 ) )
  {
    return ( OsalError::ERR );
  }

  /* os_get_random api returns 0 on success, -1 on error  */
  auto ret_status = os_get_random( (unsigned char *) random_buffer, (size_t) buffer_len );

  return ( ( !ret_status ) ? OsalError::OK : OsalError::ERR );
}
}
}
#endif /* _OS_FREERTOS_ */
