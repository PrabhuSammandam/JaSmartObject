/*
 * OsalBuilderImplWindows.h
 *
 *  Created on: Jun 27, 2017
 *      Author: psammand
 */

#pragma once

#ifdef _OS_WINDOWS_

#include "Condition.h"
#include "common/inc/OsalBuilder.h"
#include "Task.h"

namespace ja_iot
{
  namespace osal
  {
    class OsalBuilderImplWindows : public OsalBuilder
    {
    public:
      OsalBuilderImplWindows();
      virtual ~OsalBuilderImplWindows();

      void Init() override;

      Mutex* AllocateMutex() override;
      void FreeMutex(Mutex* mutex) override;

      Condition* CreateCondition() override;
      void FreeCondition(Condition* condition) override;

      Task* AllocateTask() override;
      void FreeTask(Task* task) override;

      Semaphore* alloc_semaphore() override;
      void free_semaphore(Semaphore* semaphore) override;

    private:
      Mutex* _access_mutex{nullptr};
    };
  }
}

#endif /* _OS_WINDOWS_ */
