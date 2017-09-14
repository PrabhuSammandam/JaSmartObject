/*
 * adapter_task.h
 *
 *  Created on: Sep 13, 2017
 *      Author: psammand
 */

#ifndef CODE_INCLUDE_ADAPTER_TASK_H_
#define CODE_INCLUDE_ADAPTER_TASK_H_

#include <export/Task.h>

class AdapterTask : public ja_iot::osal::ITaskRoutine
{
  public:

    AdapterTask ();

    bool create_task();

    void Run( void *arg ) override;

  private:

    ja_iot::osal::Task * adapter_task_ = nullptr;
};


#endif /* CODE_INCLUDE_ADAPTER_TASK_H_ */
