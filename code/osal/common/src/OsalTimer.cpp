#include "OsalTimer.h"
#include "common/inc/OsalTimerMgr.h"
#include "OsalMgr.h"

namespace ja_iot {
namespace osal {
OsalTimer::OsalTimer( uint64_t duration, OsalTimerCb timer_cb, bool is_one_shot ) : _duration_us{ duration }, _timer_cb{ timer_cb }, _is_one_shot{ is_one_shot }
{
}
OsalTimer::OsalTimer( uint64_t duration, OsalTimerCb timer_cb, void *user_param1, bool is_one_shot ) : _duration_us{ duration }, _timer_cb{ timer_cb }, _user_param1{ user_param1 }, _is_one_shot{ is_one_shot }
{
}
OsalTimer::OsalTimer( uint64_t duration, OsalTimerCb timer_cb, void *user_param1, void *user_param2, bool is_one_shot ) : _duration_us{ duration }, _timer_cb{ timer_cb }, _user_param1{ user_param1 }, _user_param2{ user_param2 }, _is_one_shot{ is_one_shot }
{
}
OsalTimer::~OsalTimer ()
{
  stop();
}
OsalError OsalTimer::start()
{
  return ( OsalMgr::Inst()->get_timer_mgr()->register_timer( this ) );
}
OsalError OsalTimer::stop()
{
  return ( OsalMgr::Inst()->get_timer_mgr()->unregister_timer( this ) );
}
OsalError OsalTimer::restart( uint64_t duration )
{
  _duration_us = duration;
  return ( OsalMgr::Inst()->get_timer_mgr()->reset_timer( this ) );
}
uint64_t OsalTimer::get_system_time()
{
	return OsalMgr::Inst()->get_timer_mgr()->get_system_time();
}
}
}