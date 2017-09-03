/*
 * SimpleMsgQ.h
 *
 *  Created on: Jun 27, 2017
 *      Author: psammand
 */

#ifndef OSAL_EXPORT_SIMPLEMSGQ_H_
#define OSAL_EXPORT_SIMPLEMSGQ_H_

#include "cstdint"
#include "MsgQ.h"

namespace ja_iot
{
namespace base
{
template<uint16_t noOfMsg>
class PtrMsgQ : public MsgQ
{
public:
    PtrMsgQ ()
    {
    }
    ~PtrMsgQ ()
    {
    }

    bool IsFull() override
    {
        if( ( back + 1 ) % noOfMsg == front )
        {
            return ( true );
        }
        else
        {
            return ( false );
        }
    }
    bool IsEmpty() override
    {
        if( back == front )        // is empty
        {
            return ( true );
        }
        else
        {
            return ( false );         // is not empty
        }
    }

    void* Dequeue() override
    {
        void *val = nullptr;

        if( !IsEmpty() )
        {
            val   = _msgList[front];
            front = ( front + 1 ) % noOfMsg;
        }

        return ( val );
    }

    bool Enqueue( void *msg ) override
    {
        bool b{ false };

        if( !IsFull() )
        {
            _msgList[back] = msg;
            back           = ( back + 1 ) % noOfMsg;
            b              = true;
        }

        return ( b );
    }

private:
    void        *_msgList[noOfMsg];
    uint16_t    front = 0;
    uint16_t    back  = 0;
};
}
}

#endif /* OSAL_EXPORT_SIMPLEMSGQ_H_ */
