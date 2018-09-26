/*
 * SemaphoreImplFreertos.h
 *
 *  Created on: Sep 8, 2017
 *      Author: psammand
 */

#ifndef OSAL_PORT_FREERTOS_INC_SEMAPHOREIMPLFREERTOS_H_
#define OSAL_PORT_FREERTOS_INC_SEMAPHOREIMPLFREERTOS_H_

#ifdef _OS_FREERTOS_

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <OsalError.h>
#include <Sem.h>
#include <cstdint>

namespace ja_iot {
namespace osal {

class SemaphoreImplFreertos : public Semaphore
{
public:
	SemaphoreImplFreertos();
	virtual ~SemaphoreImplFreertos();

	OsalError Init(uint32_t initial_count, uint32_t access_count) override;
	OsalError Uninit() override;

	OsalError Wait() override;
	OsalError Wait(uint32_t timeout_ms)override;
	OsalError Post() override;

	xSemaphoreHandle  get_semaphore_impl()
	{
		return semaphore_handle_;
	}

private:
	xSemaphoreHandle  semaphore_handle_ = nullptr;
};

}
}

#endif /* _OS_FREERTOS_ */

#endif /* OSAL_PORT_FREERTOS_INC_SEMAPHOREIMPLFREERTOS_H_ */
