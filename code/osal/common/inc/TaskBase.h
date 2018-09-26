/*
 * TaskBase.h
 *
 *  Created on: Sep 7, 2017
 *      Author: psammand
 */

#pragma once

#include <bitset>
#include "OsalError.h"
#include "Task.h"

namespace ja_iot {
namespace osal {
class Mutex;
class Semaphore;
}   /* namespace osal */
} /* namespace ja_iot */

namespace ja_iot {
namespace osal {
class TaskBase : public Task
{
  public:
    TaskBase () {}
    virtual ~TaskBase () {}

    OsalError Init( task_creation_params_t *pst_task_creation_params ) override;
    OsalError Init( const task_creation_params_t &task_creation_params ) override;
    OsalError Start() override;
    OsalError Stop() override;
    OsalError Destroy() override;
    OsalError SendMsg( void *pv_msg ) override;
    void      Run();

    virtual OsalError port_create_task() = 0;
    virtual OsalError port_delete_task() = 0;

  protected:
    Mutex *     _msg_q_mutex     = nullptr;
    Semaphore * _msg_q_semaphore = nullptr;
    task_creation_params_t _st_task_params{};
    std::bitset<8>   _bits;
};
}
}