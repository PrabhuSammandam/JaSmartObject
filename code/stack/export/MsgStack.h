#pragma once

#include <PtrMsgQ.h>
#include <cstdint>
#include <string>

namespace ja_iot {
namespace osal {
class OsalTimer;
class Task;
} /* namespace osal */
} /* namespace ja_iot */

namespace ja_iot {
namespace stack {
class StackEvent;
}
}
constexpr uint16_t SIMPLE_STACK_TASK_MAX_NO_MSGS = 10;
constexpr uint16_t SIMPLE_STACK_TASK_STACK_SIZE  = 512;
constexpr uint16_t SIMPLE_STACK_TASK_PRIORITY    = 5;
const std::string  SIMPLE_STACK_TASK_NAME        = "MSG_STK";

namespace ja_iot {
namespace stack {
class MsgStack
{
  public:
    static MsgStack& inst();

    void initialize( uint16_t adapter_type );
    void send_stack_event( StackEvent *stack_msg );

  private:
    ja_iot::osal::OsalTimer *                              _heart_beat_timer = nullptr;
    ja_iot::osal::Task *                                   _task             = nullptr;
    ja_iot::base::PtrMsgQ<SIMPLE_STACK_TASK_MAX_NO_MSGS>   _task_msg_q;

  private:
    MsgStack ();
    ~MsgStack ();
    static MsgStack * _p_instance;
    MsgStack( const MsgStack &other )                   = delete;
    MsgStack( MsgStack &&other ) noexcept               = delete;
    MsgStack & operator = ( const MsgStack &other )     = delete;
    MsgStack & operator = ( MsgStack &&other ) noexcept = delete;
};
}
}