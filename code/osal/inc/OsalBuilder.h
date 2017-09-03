/*
 * OsalBuilder.h
 *
 *  Created on: Jun 27, 2017
 *      Author: psammand
 */

#ifndef OSAL_INC_OSALBUILDER_H_
#define OSAL_INC_OSALBUILDER_H_

#include "Mutex.h"
#include "Condition.h"
#include "Task.h"

namespace ja_iot {
namespace osal {

class OsalBuilder
{
public:
	OsalBuilder(){}
	virtual ~OsalBuilder(){}

	virtual void Init() = 0;

	virtual Mutex* AllocateMutex() = 0;
	virtual void FreeMutex(Mutex* mutex) = 0;

	virtual Condition* CreateCondition() = 0;
	virtual void FreeCondition(Condition* condition) = 0;

	virtual Task* AllocateTask() = 0;
	virtual void FreeTask(Task* task) = 0;

};

OsalBuilder* OSAL_GetBuilder();

}
}
#endif /* OSAL_INC_OSALBUILDER_H_ */
