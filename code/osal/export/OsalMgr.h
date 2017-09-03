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

  private:

    OsalMgr ();
    static OsalMgr   _instance;
};
}
}

#endif /* OSAL_EXPORT_OSALMGR_H_ */