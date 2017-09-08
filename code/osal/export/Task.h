/*
 * Task.h
 *
 *  Created on: Jun 27, 2017
 *      Author: psammand
 */

#ifndef OSAL_EXPORT_TASK_H_
#define OSAL_EXPORT_TASK_H_

#include "PrimitiveDataTypes.h"
#include "OsalError.h"
#include "MsgQ.h"
#include <cstdint>

namespace ja_iot {
namespace osal {
constexpr uint8_t kTASK_NAME_MAX_LENGTH = 8;

class ITaskRoutine
{
  public:

    virtual ~ITaskRoutine () {}
    virtual void Run( void *arg ) = 0;
};

class ITaskMsgHandler
{
  public:

    virtual ~ITaskMsgHandler () {}
    virtual void HandleMsg( void *msg ) = 0;
    virtual void DeleteMsg( void *msg ) = 0;
};

struct TaskMsgQParam
{
  ja_iot::base::MsgQ * msgQ;
  ITaskMsgHandler *    taskMsgHandler;
};

class Task
{
  public:

    virtual ~Task () {}

    virtual OsalError InitWithMsgQ( uint8_t *taskName, uint32_t taskPriority, uint32_t stackSize, TaskMsgQParam *taskMsgQParam, void *taskArg ) = 0;
    virtual OsalError Init( uint8_t *taskName, uint32_t taskPriority, uint32_t stackSize, ITaskRoutine *taskRoutine, void *taskArg )            = 0;

    virtual OsalError Start()   = 0;
    virtual OsalError Stop()    = 0;
    virtual OsalError Destroy() = 0;
    virtual OsalError Wait()    = 0;

    virtual OsalError SendMsg( void *msgMem ) = 0;
};
}
}

#endif /* OSAL_EXPORT_TASK_H_ */
