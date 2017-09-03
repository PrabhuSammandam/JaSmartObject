/*
 * ConditionImplWindows.cpp
 *
 *  Created on: Jun 27, 2017
 *      Author: psammand
 */

#ifdef _OS_WINDOWS_

#include "windows/inc/ConditionImplWindows.h"
#include "windows/inc/MutexImplWindows.h"

namespace ja_iot {
namespace osal {

ConditionImplWindows::ConditionImplWindows() {
}

ConditionImplWindows::~ConditionImplWindows() {
}

OsalError ConditionImplWindows::Init() {
	InitializeConditionVariable(&_conditionVariable);
	return OsalError::OK;
}

OsalError ConditionImplWindows::Uninit() {
	return OsalError::OK;
}

OsalError ConditionImplWindows::Wait(Mutex* mutex, u64 timeOutInMicroSec) {
	MutexImplWindows* mutexImpl = (MutexImplWindows*) mutex;
	OsalError retValue = OsalError::OK;
	DWORD milli = 0;

	if (timeOutInMicroSec > 0) {
		milli = (DWORD) (timeOutInMicroSec / 1000);
	} else {
		milli = INFINITE;
	}
	// Wait for the given time
	if (!SleepConditionVariableCS(&_conditionVariable,
			mutexImpl->GetMutexImpl(), milli)) {
		if (GetLastError() == ERROR_TIMEOUT) {
			retValue = OsalError::CONDITION_WAIT_TIMEOUT;
		} else {
			retValue = OsalError::CONDITION_WAIT_INVALID;
		}
	}

	return retValue;
}

void ConditionImplWindows::Signal() {
	WakeConditionVariable(&_conditionVariable);
}

void ConditionImplWindows::Broadcast() {
	WakeAllConditionVariable(&_conditionVariable);
}

}
}

#endif /*_OS_WINDOWS_*/
