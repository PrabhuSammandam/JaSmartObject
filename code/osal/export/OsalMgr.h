/*
 * OsalMgr.h
 *
 *  Created on: Jun 27, 2017
 *      Author: psammand
 */

#ifndef OSAL_EXPORT_OSALMGR_H_
#define OSAL_EXPORT_OSALMGR_H_

#include "PrimitiveDataTypes.h"
#include "OsalError.h"
#include "Mutex.h"
#include "Condition.h"
#include "Task.h"
#include "Sem.h"

namespace ja_iot {
namespace osal {
class OsalMgr
{
  public:
    static OsalMgr* Inst();

    void Init();

    Mutex* AllocMutex();
    void   FreeMutex( Mutex *mutex );

    Condition* AllocCondition();
    void       FreeCondition( Condition *condition );

    Task* AllocTask();
    void  FreeTask( Task *task );

    Semaphore* alloc_semaphore();
    void       free_semaphore( Semaphore * );

  private:

    OsalMgr ();
    static OsalMgr   _instance;
    bool             is_inited_ = false;
};
}
}

#endif /* OSAL_EXPORT_OSALMGR_H_ */