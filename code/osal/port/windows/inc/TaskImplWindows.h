/*
 * TaskImplWindows.h
 *
 *  Created on: Sep 7, 2017
 *      Author: psammand
 */

#pragma once

#ifdef _OS_WINDOWS_

#include <windows.h>
#include "common/inc/TaskBase.h"
#include "OsalError.h"

namespace ja_iot
{
  namespace osal
  {
    class TaskImplWindows : public TaskBase
    {
    public:

      TaskImplWindows();

      ~TaskImplWindows();

      OsalError port_create_task() override;
      OsalError port_delete_task() override;
      OsalError Wait() override;

    private:

      HANDLE _taskHandle = nullptr;
    };
  }
}


#endif /* OSAL_PORT_WINDOWS_INC_TASKIMPLWINDOWS_H_ */
