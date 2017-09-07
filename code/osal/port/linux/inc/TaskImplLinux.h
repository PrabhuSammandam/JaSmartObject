/*
 * TaskImplLinux.h
 *
 *  Created on: 05-Sep-2017
 *      Author: prabhu
 */

#ifndef OSAL_PORT_LINUX_INC_TASKIMPLLINUX_H_
#define OSAL_PORT_LINUX_INC_TASKIMPLLINUX_H_

#ifdef _OS_LINUX_

#include <pthread.h>
#include "OsalError.h"
#include "common/inc/TaskBase.h"

namespace ja_iot {
namespace osal {
class TaskImplLinux : public TaskBase
{
  public:

    TaskImplLinux ();

    virtual ~TaskImplLinux ();

    OsalError PortCreateTask() override;
    OsalError PortDeleteTask() override;
    OsalError Wait() override;

  private:
    pthread_t            task_handle_      = 0;
};
}
}

#endif /* _OS_LINUX_ */

#endif /* OSAL_PORT_LINUX_INC_TASKIMPLLINUX_H_ */
