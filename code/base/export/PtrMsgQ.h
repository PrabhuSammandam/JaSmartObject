/*
 * SimpleMsgQ.h
 *
 *  Created on: Jun 27, 2017
 *      Author: psammand
 */

#ifndef __PTR_MSG_Q_H__
#define __PTR_MSG_Q_H__

#include "cstdint"
#include "MsgQ.h"

namespace ja_iot
{
  namespace base
  {
    /***
     * Message queue for only pointers with fixed no of messages. It enqueues and dequeues only pointer types.
     * It is derived from the MsgQ class. The MsgQ class is the base class which provides
     * the interface for queue operations.
     *
     * Why this class ?
     *
     * In standard C++ STL there is no option to give the statically allocated memory for containers.
     * This class is used to define the statically allocated memory for the containers and this container is used
     * by the clients with the MsgQ base class interface.
     */
    template <uint16_t noOfMsg>
    class PtrMsgQ : public MsgQ
    {
    public:

      PtrMsgQ()
      {
      }

      ~PtrMsgQ()
      {
      }

      uint16_t GetCapacity() override
      {
        return noOfMsg;
      }

      bool IsFull() override
      {
        if ((back + 1) % noOfMsg == front)
        {
          return true;
        }
        return false;
      }

      bool IsEmpty() override
      {
        if (back == front) // is empty
        {
          return true;
        }
        return false; // is not empty
      }

      void* Dequeue() override
      {
        void* val = nullptr;

        if (!IsEmpty())
        {
          val = _msgList[front];
          front = (front + 1) % noOfMsg;
        }

        return val;
      }

      bool Enqueue(void* msg) override
      {
        auto b{false};

        if (!IsFull())
        {
          _msgList[back] = msg;
          back = (back + 1) % noOfMsg;
          b = true;
        }

        return b;
      }

    private:
      void* _msgList[noOfMsg];
      uint16_t front = 0;
      uint16_t back = 0;
    };
  }
}

#endif /* __PTR_MSG_Q_H__ */
