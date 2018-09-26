#pragma once
#include <chrono>
#include <mutex>
#include <iostream>
#include <functional>

namespace ThreadPoolTimer {
class TimerRunLoop;

class TimerImpl
{
  public:
    TimerImpl( std::function<void()> task );
    ~TimerImpl ();

    static void stopRunLoop();
    static void startRunLoop();

    void setInterval( const std::chrono::nanoseconds &interval )
    {
      if( !_enabled )
      {
        _interval = interval;
      }
    }
    const std::chrono::nanoseconds& getInterval() { return ( _interval ); }

    void setEnabled( bool enabled );
    bool getEnabled() { return ( _enabled ); }

    void                                 setNextTrigger( const std::chrono::steady_clock::time_point &trigger ) { _nextTrigger = trigger; }
    std::chrono::steady_clock::time_point& getNextTrigger() { return ( _nextTrigger ); }

    void trigger() { _task(); }

  private:
    std::function<void()>   _task;

    TimerRunLoop   &_runLoop;

    std::chrono::nanoseconds                _interval;
    std::chrono::steady_clock::time_point   _nextTrigger;

    bool   _enabled;
};
}