/*
 * OsalBuilderImplWindows.h
 *
 *  Created on: Jun 27, 2017
 *      Author: psammand
 */

#ifndef OSAL_WINDOWS_INC_OSALBUILDERIMPLWINDOWS_H_
#define OSAL_WINDOWS_INC_OSALBUILDERIMPLWINDOWS_H_

#include "PrimitiveDataTypes.h"
#include "Condition.h"
#include "OsalError.h"
#include "inc/OsalBuilder.h"
#include "Task.h"

namespace ja_iot
{
namespace osal
{
class OsalBuilderImplWindows : public OsalBuilder
{
public:
    OsalBuilderImplWindows ();
    virtual ~OsalBuilderImplWindows ();

    void Init() override;

    Mutex* AllocateMutex() override;
    void FreeMutex( Mutex *mutex ) override;

    Condition* CreateCondition() override;
    void     FreeCondition( Condition *condition ) override;

    Task * AllocateTask() override;
    void FreeTask( Task *task ) override;

private:
    Mutex *_accessMutex{ nullptr };
};
}
}

#endif /* OSAL_WINDOWS_INC_OSALBUILDERIMPLWINDOWS_H_ */
