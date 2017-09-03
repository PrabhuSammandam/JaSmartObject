/*
 * Condition.h
 *
 *  Created on: Jun 27, 2017
 *      Author: psammand
 */

#ifndef OSAL_EXPORT_CONDITION_H_
#define OSAL_EXPORT_CONDITION_H_

#include "PrimitiveDataTypes.h"
#include "OsalError.h"
#include "Mutex.h"
#include <cstdint>

namespace ja_iot {
namespace osal {
class Condition
{
  public:

    Condition () {}

    virtual ~Condition () {}

    virtual OsalError Init()   = 0;
    virtual OsalError Uninit() = 0;

    virtual OsalError Wait( Mutex *mutex, uint64_t timeOut = 0 ) = 0;
    virtual void      Signal()                                   = 0;
    virtual void      Broadcast()                                = 0;
};
}
}



#endif /* OSAL_EXPORT_CONDITION_H_ */