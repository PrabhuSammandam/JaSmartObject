#pragma once
#include <chrono>
#include <functional>

namespace ThreadPoolTimer {
class TimerImpl;
class Timer
{
  public:
    Timer( std::function<void()> task );
    ~Timer ();

    void set_interval( const std::chrono::nanoseconds &interval );
    void set_enabled( bool enabled );
    bool is_enabled();

    static void stopRunLoop();
    static void startRunLoop();

  private:
    TimerImpl * _impl;
};
}