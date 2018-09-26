#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <unordered_set>
#include "timer_impl.hpp"

namespace ThreadPoolTimer {
class TimerRunLoop
{
  public:
    TimerRunLoop ();
    ~TimerRunLoop ();
    static TimerRunLoop& getSingleton();

    bool getRunning();
    void stop();
    void start();

    void registerTimer( TimerImpl &timer );
    void unRegisterTimer( TimerImpl &timer );

  private:
    std::thread * _thread;

    std::vector<TimerImpl *>   _timers;
    std::mutex                 _timersMutex;
    std::condition_variable    _timersCondition;

    std::mutex                _stopMutex;
    std::condition_variable   _stopCondition;

    std::chrono::steady_clock::time_point   _nowTime;
    void _sortTimers();
    void _updateNowTime();
    void _resetTimer( TimerImpl &timer );

    void _triggerTimers();

    void _loopTask();
    void _start();
    void _stop();
    bool   _running;
};
}