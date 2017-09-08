/*
 * TaskBase.h
 *
 *  Created on: Sep 7, 2017
 *      Author: psammand
 */

#ifndef OSAL_COMMON_INC_TASKBASE_H_
#define OSAL_COMMON_INC_TASKBASE_H_

#include <OsalError.h>
#include <Task.h>
#include <cstdint>

namespace ja_iot {
namespace osal {
class Mutex;
class Semaphore;
} /* namespace osal */
} /* namespace ja_iot */

namespace ja_iot {
namespace osal {
class TaskBase : public Task
{
  public:

    TaskBase ()
    {
      for( int i = 0; i < kTASK_NAME_MAX_LENGTH; ++i )
      {
        task_name_[i] = 0;
      }
    }

    virtual ~TaskBase () {}

    OsalError         Init( uint8_t *taskName, uint32_t taskPriority, uint32_t stackSize, ITaskRoutine *taskRoutine, void *taskArg ) override;
    OsalError         InitWithMsgQ( uint8_t *taskName, uint32_t taskPriority, uint32_t stackSize, TaskMsgQParam *taskMsgQParam, void *taskArg ) override;
    OsalError         Start() override;
    OsalError         Stop() override;
    OsalError         Destroy() override;
    OsalError         SendMsg( void *msgMem ) override;
    void              Run();
    virtual OsalError PortCreateTask() = 0;
    virtual OsalError PortDeleteTask() = 0;

  protected:
    Mutex *              msg_q_mutex_      = nullptr;
    Semaphore *          msg_q_semaphore_  = nullptr;
    uint32_t             task_priority_    = 0;
    uint32_t             stack_size_       = 0;
    bool                 is_to_stop_       = true;
    ja_iot::base::MsgQ * msg_q_            = nullptr;
    ITaskRoutine *       task_routine_     = nullptr;
    ITaskMsgHandler *    task_msg_handler_ = nullptr;
    bool                 is_msg_q_enabled_ = false;
    void *               task_argument_    = nullptr;
    uint8_t              task_name_[kTASK_NAME_MAX_LENGTH];
};
}
}


#endif /* OSAL_COMMON_INC_TASKBASE_H_ */