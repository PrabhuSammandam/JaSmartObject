/*
 * TaskImplLinux.h
 *
 *  Created on: 05-Sep-2017
 *      Author: prabhu
 */

#pragma once

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

    OsalError port_create_task() override;
    OsalError port_delete_task() override;
    OsalError Wait() override;

  private:
    pthread_t            task_handle_      = 0;
};
}
}

#endif /* _OS_LINUX_ */
