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
#include "Task.h"
#include "OsalError.h"
#include "Mutex.h"
#include "Sem.h"

namespace ja_iot {
namespace osal {
class TaskImplLinux : public Task
{
  public:

    TaskImplLinux ();

    virtual ~TaskImplLinux ();

    OsalError InitWithMsgQ( uint8_t *taskName, uint32_t taskPriority, uint32_t stackSize, TaskMsgQParam *taskMsgQParam, void *taskArg ) override;
    OsalError Init( uint8_t *taskName, uint32_t taskPriority, uint32_t stackSize, ITaskRoutine *taskRoutine, void *taskArg )   override;

    OsalError Start() override;
    OsalError Stop() override;
    OsalError Destroy() override;
    OsalError Wait() override;

    OsalError SendMsg( void *msgMem ) override;

    void Run();

  private:
    pthread_t            task_handle_      = 0;
    Mutex *              msg_q_mutex_      = nullptr;
    Semaphore *          msg_q_semaphore_  = nullptr;
    uint32_t             task_priority_    = 0;
    uint32_t             stack_size_       = 0;
    bool                 is_to_stop_       = true;
    ja_iot::base::MsgQ * msg_q_            = nullptr;
    ITaskRoutine *       task_routine_     = nullptr;
    ITaskMsgHandler *    task_msg_handler_ = nullptr;
    bool                 is_msg_q_enabled_ = false;
    void *               task_argument_   = nullptr;
};
}
}

#endif /* _OS_LINUX_ */

#endif /* OSAL_PORT_LINUX_INC_TASKIMPLLINUX_H_ */