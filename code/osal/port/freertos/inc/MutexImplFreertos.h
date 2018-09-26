/*
 * MutexImplFreertos.h
 *
 *  Created on: Sep 8, 2017
 *      Author: psammand
 */

#pragma once

#ifdef _OS_FREERTOS_

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <Mutex.h>
#include <OsalError.h>


namespace ja_iot {
namespace osal {
class MutexImplFreertos : public Mutex
{
  public:

    MutexImplFreertos ();

    virtual ~MutexImplFreertos ();

    OsalError Init() override;
    OsalError Uninit() override;

    OsalError Lock() override;
    OsalError Unlock() override;

    xSemaphoreHandle GetMutexImpl()
    {
      return ( mutex_impl_ );
    }

  private:
    xSemaphoreHandle   mutex_impl_ = nullptr;
};
}  // namespace osal
}  // namespace ja_iot
#endif /* _OS_FREERTOS_ */
