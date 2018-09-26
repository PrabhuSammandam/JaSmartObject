/*
 * OsalMgr.h
 *
 *  Created on: Jun 27, 2017
 *      Author: psammand
 */

#pragma once

#include "Mutex.h"
#include "Condition.h"
#include "Task.h"
#include "Sem.h"

namespace ja_iot {
namespace osal {
class OsalTimerMgr;

class OsalMgr
{
  public:
    static OsalMgr* Inst();

    void Init();

    Mutex* AllocMutex() const;
    void   FreeMutex( Mutex *mutex );

    Condition* AllocCondition();
    void       FreeCondition( Condition *condition );

    Task* AllocTask();
    void  FreeTask( Task *task );

    Semaphore* alloc_semaphore();
    void       free_semaphore( Semaphore * );

		OsalTimerMgr* get_timer_mgr();

  private:
    OsalMgr ();
    static OsalMgr   _instance;
    bool             is_inited_ = false;
    OsalMgr( const OsalMgr &other )               = delete;
    OsalMgr( OsalMgr &&other )                    = delete;
    OsalMgr & operator = ( const OsalMgr &other ) = delete;
    OsalMgr & operator = ( OsalMgr &&other )      = delete;
};
}
}