/*
 * OsalBuilderImpl.h
 *
 *  Created on: Jun 27, 2017
 *      Author: psammand
 */

#ifndef OSAL_INC_OSALBUILDERIMPL_H_
#define OSAL_INC_OSALBUILDERIMPL_H_

#ifdef _OS_WINDOWS_
#include "windows/inc/OsalBuilderImplWindows.h"
#endif /* _OS_WINDOWS_ */

#ifdef _OS_LINUX_
#include "port/linux/inc/OsalBuilderImplLinux.h"
#endif /* _OS_LINUX_ */

#endif /* OSAL_INC_OSALBUILDERIMPL_H_ */
