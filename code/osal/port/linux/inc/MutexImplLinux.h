/*
 * MutexImplLinux.h
 *
 *  Created on: 04-Sep-2017
 *      Author: prabhu
 */

#ifndef OSAL_PORT_LINUX_INC_MUTEXIMPLLINUX_H_
#define OSAL_PORT_LINUX_INC_MUTEXIMPLLINUX_H_

#ifdef _OS_LINUX_

#include <pthread.h>
#include <Mutex.h>
#include <OsalError.h>

namespace ja_iot {

namespace osal {

class MutexImplLinux: public Mutex {
public:
	MutexImplLinux();
	virtual ~MutexImplLinux();

	OsalError Init() override;
	OsalError Uninit() override;

	OsalError Lock() override;
	OsalError Unlock() override;

	pthread_mutex_t* GetMutexImpl() {
		return (&mutex_impl_);
	}

private:
	pthread_mutex_t mutex_impl_ = PTHREAD_MUTEX_INITIALIZER;

};
}
}  // namespace ja_iot
#endif /*_OS_LINUX_*/

#endif /* OSAL_PORT_LINUX_INC_MUTEXIMPLLINUX_H_ */
