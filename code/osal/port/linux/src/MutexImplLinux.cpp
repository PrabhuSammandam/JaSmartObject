/*
 * MutexImplLinux.cpp
 *
 *  Created on: 04-Sep-2017
 *      Author: prabhu
 */

#ifdef _OS_LINUX_

#include <port/linux/inc/MutexImplLinux.h>

namespace ja_iot {
namespace osal {
MutexImplLinux::MutexImplLinux ()
{
}

MutexImplLinux::~MutexImplLinux ()
{
}

OsalError MutexImplLinux::Init()
{
	pthread_mutexattr_t mutex_attr;

	pthread_mutexattr_init(&mutex_attr);

	pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE);

  int ret_status = pthread_mutex_init( &mutex_impl_, &mutex_attr );

  if( ret_status == 0 )
  {
    return ( OsalError::OK );
  }

  return ( OsalError::ERR );
}

OsalError MutexImplLinux::Lock()
{
  int ret_status = pthread_mutex_lock( &mutex_impl_ );

  if( ret_status == 0 )
  {
    return ( OsalError::OK );
  }

  return ( OsalError::ERR );
}

OsalError MutexImplLinux::Unlock()
{
  int ret_status = pthread_mutex_unlock( &mutex_impl_ );

  if( ret_status == 0 )
  {
    return ( OsalError::OK );
  }

  return ( OsalError::ERR );
}

OsalError MutexImplLinux::Uninit()
{
  int ret_status = pthread_mutex_destroy( &mutex_impl_ );

  if( ret_status == 0 )
  {
    mutex_impl_ = PTHREAD_MUTEX_INITIALIZER;
    return ( OsalError::OK );
  }

  return ( OsalError::ERR );
}
}  // namespace osal
}  // namespace ja_iot

#endif /* _OS_LINUX_ */
