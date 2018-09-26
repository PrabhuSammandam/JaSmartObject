/*
 * OsalTimer.h
 *
 *  Created on: Dec 19, 2017
 *      Author: psammand
 */

#pragma once

#include <cstdint>
#include <OsalError.h>

namespace ja_iot {
namespace osal {
typedef void ( *OsalTimerCb ) ( void *user_param1, void *user_param2 );
class OsalTimerMgr;

class OsalTimer
{
  public:
    OsalTimer(uint64_t duration_us, OsalTimerCb timer_cb, bool is_one_shot = false );
    OsalTimer(uint64_t duration_us, OsalTimerCb timer_cb, void *user_param1, bool is_one_shot = false );
    OsalTimer(uint64_t duration_us, OsalTimerCb timer_cb, void *user_param1, void *user_param2, bool is_one_shot = false );
		~OsalTimer();

    OsalError start();
    OsalError stop();
		OsalError restart(uint64_t duration);

		static uint64_t get_system_time();

  private:
		uint64_t _duration_us{};
		uint64_t _trigger_time_us{};
    OsalTimerCb _timer_cb{};
    void *_user_param1{};
    void *_user_param2{};
    bool _is_one_shot{ false };

		friend OsalTimerMgr;
		friend bool timer_compare(OsalTimer* timer1, OsalTimer* timer2);
};
}
}  // namespace ja_iot