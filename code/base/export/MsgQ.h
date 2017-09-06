/*
 * MsgQ.h
 *
 *  Created on: Jun 27, 2017
 *      Author: psammand
 */

#ifndef OSAL_EXPORT_MSGQ_H_
#define OSAL_EXPORT_MSGQ_H_

#include <cstdint>

namespace ja_iot {
namespace base {
class MsgQ {
public:
  virtual ~MsgQ() {}

  virtual uint16_t GetCapacity() = 0;

  virtual bool IsFull() = 0;
  virtual bool IsEmpty() = 0;

  virtual void *Dequeue() = 0;
  virtual bool Enqueue(void *msg) = 0;
};
}
}

#endif /* OSAL_EXPORT_MSGQ_H_ */
