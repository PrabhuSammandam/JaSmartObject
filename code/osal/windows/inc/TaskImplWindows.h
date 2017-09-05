/*
 * TaskImplWindows.h
 *
 *  Created on: Jun 27, 2017
 *      Author: psammand
 */

#ifndef OSAL_WINDOWS_INC_TASKIMPLWINDOWS_H_
#define OSAL_WINDOWS_INC_TASKIMPLWINDOWS_H_

#ifdef _OS_WINDOWS_

#include "PrimitiveDataTypes.h"
#include "Task.h"
#include "OsalError.h"
#include "Mutex.h"
#include "Condition.h"
#include <windows.h>

namespace ja_iot {
namespace osal {
class TaskImplWindows : public Task
{
  public:

    TaskImplWindows ();

    ~TaskImplWindows ();
    OsalError InitWithMsgQ( pu8 taskName, u32 taskPriority, u32 stackSize, TaskMsgQParam *taskMsgQParam, pvoid taskArg ) override;
    OsalError Init( pu8 taskName, u32 taskPriority, u32 stackSize, ITaskRoutine *taskRoutine, pvoid taskArg )   override;

    OsalError Start() override;
    OsalError Stop() override;
    OsalError Destroy() override;
    OsalError Wait() override;

    OsalError SendMsg( pvoid msgMem ) override;

    void Run();

  private:

    HANDLE               _taskHandle     = 0;
    Mutex *              _mutex          = nullptr;
    Condition *          _condition      = nullptr;
    u32                  _taskPriority   = 0;
    u32                  _stackSize      = 0;
    bool                 _isToStop       = true;
    ja_iot::base::MsgQ * _msgQ           = nullptr;
    ITaskMsgHandler *    _taskMsgHandler = nullptr;
    ITaskRoutine *       _taskRoutine    = nullptr;
    bool                 _isMsgQTask     = false;
    pvoid                _taskArg        = nullptr;
};
}
}

#endif /* _OS_WINDOWS_ */

#endif /* OSAL_WINDOWS_INC_TASKIMPLWINDOWS_H_ */