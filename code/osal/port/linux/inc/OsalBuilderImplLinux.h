/*
 * OsalBuilderImplLinux.h
 *
 *  Created on: 05-Sep-2017
 *      Author: prabhu
 */

#ifndef OSAL_PORT_LINUX_INC_OSALBUILDERIMPLLINUX_H_
#define OSAL_PORT_LINUX_INC_OSALBUILDERIMPLLINUX_H_

#ifdef _OS_LINUX_

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
class OsalBuilderImplLinux : public OsalBuilder
{
  public:

    OsalBuilderImplLinux ();

    virtual ~OsalBuilderImplLinux ();

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

#endif /* _OS_LINUX_ */

#endif /* OSAL_PORT_LINUX_INC_OSALBUILDERIMPLLINUX_H_ */
