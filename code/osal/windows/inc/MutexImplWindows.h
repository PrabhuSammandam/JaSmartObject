/*
 * MutexImplWindows.h
 *
 *  Created on: Jun 27, 2017
 *      Author: psammand
 */

#ifndef OSAL_WINDOWS_INC_MUTEXIMPLWINDOWS_H_
#define OSAL_WINDOWS_INC_MUTEXIMPLWINDOWS_H_

#ifdef _OS_WINDOWS_

#include <windows.h>
#include "Mutex.h"

namespace ja_iot {
	namespace osal {
		class MutexImplWindows : public Mutex
		{
		public:

			MutexImplWindows ();

			virtual ~MutexImplWindows ();

			OsalError Init() override;
			OsalError Lock() override;
			OsalError Unlock() override;
			OsalError Uninit() override;

			CRITICAL_SECTION* GetMutexImpl()
			{
				return ( &_criticalSection );
			}

		private:
			CRITICAL_SECTION _criticalSection {};
		};
	}
}
#endif /*_OS_WINDOWS_ */
#endif /* OSAL_WINDOWS_INC_MUTEXIMPLWINDOWS_H_ */
