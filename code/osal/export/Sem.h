/*
 * Semaphore.h
 *
 *  Created on: 05-Sep-2017
 *      Author: prabhu
 */

#ifndef OSAL_EXPORT_SEM_H_
#define OSAL_EXPORT_SEM_H_

#include <OsalError.h>
#include <cstdint>

namespace ja_iot {
namespace osal {
class Semaphore
{
  public:

    Semaphore () {}

    virtual ~Semaphore ();

    virtual OsalError Init( uint32_t access_count ) = 0;
    virtual OsalError Uninit()                      = 0;

    virtual OsalError Wait() = 0;
    virtual OsalError Post() = 0;
};
}
}


#endif /* OSAL_EXPORT_SEM_H_ */