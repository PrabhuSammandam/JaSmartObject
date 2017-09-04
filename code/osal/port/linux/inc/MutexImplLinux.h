/*
 * MutexImplLinux.h
 *
 *  Created on: 04-Sep-2017
 *      Author: prabhu
 */

#ifndef OSAL_PORT_LINUX_INC_MUTEXIMPLLINUX_H_
#define OSAL_PORT_LINUX_INC_MUTEXIMPLLINUX_H_

#include <Mutex.h>
#include <OsalError.h>

namespace ja_iot {

namespace osal {

class MutexImplLinux: public Mutex {
public:
	MutexImplLinux();
	virtual ~MutexImplLinux();

	OsalError Init() override;
	OsalError Lock() override;
	OsalError Unlock() override;
	OsalError Uninit() override;

	void* GetMutexImpl() {
		return (mutex_impl_);
	}

private:
	void* mutex_impl_;

};

}

}  // namespace ja_iot

#endif /* OSAL_PORT_LINUX_INC_MUTEXIMPLLINUX_H_ */
