/*
 * ConditionImplWindows.h
 *
 *  Created on: Jun 27, 2017
 *      Author: psammand
 */

#ifndef OSAL_WINDOWS_INC_CONDITIONIMPLWINDOWS_H_
#define OSAL_WINDOWS_INC_CONDITIONIMPLWINDOWS_H_

#include "PrimitiveDataTypes.h"
#include "Condition.h"
#include "OsalError.h"
#include <windows.h>

namespace ja_iot {
namespace osal {

class ConditionImplWindows: public Condition {
public:
	ConditionImplWindows();
	virtual ~ConditionImplWindows();

	OsalError Init() override;
	OsalError Uninit() override;

	OsalError Wait(Mutex* mutex, u64 timeOutInMicroSec = 0) override;

	void Signal() override;
	void Broadcast() override;

private:
	CONDITION_VARIABLE _conditionVariable;
};

}
}

#endif /* OSAL_WINDOWS_INC_CONDITIONIMPLWINDOWS_H_ */
