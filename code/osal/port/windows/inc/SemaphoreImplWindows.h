/*
 * SemaphoreImplWindows.h
 *
 *  Created on: Sep 5, 2017
 *      Author: psammand
 */

#ifndef OSAL_PORT_WINDOWS_INC_SEMAPHOREIMPLWINDOWS_H_
#define OSAL_PORT_WINDOWS_INC_SEMAPHOREIMPLWINDOWS_H_

#include <Sem.h>
#include <windows.h>

#ifdef _OS_WINDOWS_

namespace ja_iot {
namespace osal {

class SemaphoreImplWindows: public Semaphore {
public:
	SemaphoreImplWindows();
	virtual ~SemaphoreImplWindows();

	OsalError Init(uint32_t initial_count, uint32_t access_count) override;
	OsalError Uninit() override;

	OsalError Wait() override;
	OsalError Post() override;

private:
	HANDLE semaphore_hndl_ = INVALID_HANDLE_VALUE;
};

}  // namespace osal
}  // namespace ja_iot

#endif /* _OS_WINDOWS_ */

#endif /* OSAL_PORT_WINDOWS_INC_SEMAPHOREIMPLWINDOWS_H_ */
