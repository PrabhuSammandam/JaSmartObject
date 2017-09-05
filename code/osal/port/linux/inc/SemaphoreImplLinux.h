/*
 * SemaphoreImplLinux.h
 *
 *  Created on: 05-Sep-2017
 *      Author: prabhu
 */

#ifndef OSAL_PORT_LINUX_INC_SEMAPHOREIMPLLINUX_H_
#define OSAL_PORT_LINUX_INC_SEMAPHOREIMPLLINUX_H_

#ifdef _OS_LINUX_

#include <OsalError.h>
#include <Sem.h>
#include <cstdint>
#include <semaphore.h>

namespace ja_iot {
namespace osal {

class SemaphoreImplLinux : public Semaphore
{
public:
	SemaphoreImplLinux();
	virtual ~SemaphoreImplLinux();

	OsalError Init(uint32_t access_count) override;
	OsalError Uninit() override;

	OsalError Wait() override;
	OsalError Post() override;

	sem_t* get_semaphore_impl()
	{
		return &semaphore_handle_;
	}

private:
	sem_t semaphore_handle_;
};

}
}

#endif /* _OS_LINUX_ */
#endif /* OSAL_PORT_LINUX_INC_SEMAPHOREIMPLLINUX_H_ */
