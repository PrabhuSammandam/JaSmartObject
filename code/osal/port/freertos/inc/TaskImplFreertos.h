/*
 * TaskImplFreertos.h
 *
 *  Created on: Sep 8, 2017
 *      Author: psammand
 */

#ifndef OSAL_PORT_FREERTOS_INC_TASKIMPLFREERTOS_H_
#define OSAL_PORT_FREERTOS_INC_TASKIMPLFREERTOS_H_

#ifdef _OS_FREERTOS_

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "OsalError.h"
#include "common/inc/TaskBase.h"

namespace ja_iot {
namespace osal {
class TaskImplFreertos : public TaskBase
{
  public:

	TaskImplFreertos ();

    virtual ~TaskImplFreertos ();

    OsalError PortCreateTask() override;
    OsalError PortDeleteTask() override;
    OsalError Wait() override;

  private:
    xTaskHandle   task_handle_ = 0;
};
}
}

#endif /* _OS_FREERTOS_ */

#endif /* OSAL_PORT_FREERTOS_INC_TASKIMPLFREERTOS_H_ */
