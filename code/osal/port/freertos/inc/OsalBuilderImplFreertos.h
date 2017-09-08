/*
 * OsalBuilderImplFreertos.h
 *
 *  Created on: Sep 8, 2017
 *      Author: psammand
 */

#ifndef OSAL_PORT_FREERTOS_INC_OSALBUILDERIMPLFREERTOS_H_
#define OSAL_PORT_FREERTOS_INC_OSALBUILDERIMPLFREERTOS_H_

#ifdef _OS_FREERTOS_

#include <inc/OsalBuilder.h>

namespace ja_iot {
namespace osal {
class Condition;
class Mutex;
class Task;
} /* namespace osal */
} /* namespace ja_iot */

namespace ja_iot {
namespace osal {
class OsalBuilderImplFreertos : public OsalBuilder
{
  public:

    OsalBuilderImplFreertos ();

    virtual ~OsalBuilderImplFreertos ();

    void Init() override;

    Mutex* AllocateMutex() override;
    void   FreeMutex( Mutex *mutex ) override;

    Condition* CreateCondition() override;
    void       FreeCondition( Condition *condition ) override;

    Task* AllocateTask() override;
    void  FreeTask( Task *task ) override;

    Semaphore* alloc_semaphore()                      override;
    void       free_semaphore( Semaphore *semaphore ) override;

  private:
    Mutex * access_mutex = nullptr;
};
}
}

#endif /* _OS_FREERTOS_ */

#endif /* OSAL_PORT_FREERTOS_INC_OSALBUILDERIMPLFREERTOS_H_ */