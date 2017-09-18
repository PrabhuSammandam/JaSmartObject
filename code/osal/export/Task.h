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

#define DECLARE_TASK_MSG_HANDLER_CLASS( HNDLR_CLASS, HOST, HNDL_FUNC, DELETE_FUNC ) class HNDLR_CLASS : public ja_iot::osal::ITaskMsgHandler \
{ \
    public: \
      HNDLR_CLASS( HOST * host ) : host_{ host } {} \
      void HandleMsg( void *msg ) override{ host_->HNDL_FUNC( msg ); } \
      void DeleteMsg( void *msg ) override{ host_->DELETE_FUNC( msg ); } \
    private: \
      HOST * host_; \
}; \

#define DECLARE_TASK_ROUTINE_CLASS( ROUTINE_CLASS, HOST, RUN_FUNC ) class ROUTINE_CLASS : public ja_iot::osal::ITaskRoutine \
{ \
    public: \
      ROUTINE_CLASS( HOST * host ) : host_{ host } {} \
      void Run( void *arg ) override{ host_->RUN_FUNC( arg ); } \
    private: \
      HOST * host_; \
}; \

}
}

#endif /* OSAL_EXPORT_TASK_H_ */
