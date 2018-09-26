#pragma once

#include <vector>
#include <cstdint>
#include "Mutex.h"
#include "Sem.h"
#include "Task.h"

namespace ja_iot {
namespace osal {
class OsalTimer;
class OsalTimerMgr
{
  public:
    OsalError initialize();

    OsalError register_timer( OsalTimer *timer );
    OsalError unregister_timer( OsalTimer *timer );
    OsalError reset_timer( OsalTimer *timer );
    void      task_loop();
		uint64_t get_system_time();

  private:
    void sort_timers();
    bool trigger_timers();
    void update_now_time();

    std::vector<OsalTimer *>   _timers_list;
    Mutex *_list_mutex{};
    Semaphore *_queue_sem{};
    Task *_task{};
    bool _is_initialised{ false };
    bool _is_running{ false };
    uint64_t _now_time_us{};
    uint32_t _prev_time_us{};
};
}
}