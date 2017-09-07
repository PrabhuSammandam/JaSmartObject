/*
 * TaskImplWindowsSem.h
 *
 *  Created on: Sep 7, 2017
 *      Author: psammand
 */

#ifndef OSAL_PORT_WINDOWS_INC_TASKIMPLWINDOWS_H_
#define OSAL_PORT_WINDOWS_INC_TASKIMPLWINDOWS_H_

#ifdef _OS_WINDOWS_

#include <windows.h>
#include "common/inc/TaskBase.h"
#include "OsalError.h"

namespace ja_iot {
namespace osal {
class TaskImplWindowsSem : public TaskBase
{
  public:

    TaskImplWindowsSem ();

    ~TaskImplWindowsSem ();

    OsalError PortCreateTask() override;
    OsalError PortDeleteTask() override;
    OsalError Wait() override;

  private:

    HANDLE   _taskHandle = 0;
};
}
}

#endif /* _OS_WINDOWS_ */




#endif /* OSAL_PORT_WINDOWS_INC_TASKIMPLWINDOWS_H_ */
