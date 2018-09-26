#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include "tp_timer.hpp"
#include "timer_impl.hpp"
#include "timer_run_loop.hpp"

namespace ThreadPoolTimer {
TimerImpl::TimerImpl( std::function<void()> task ) :
  _task( task ),
  _enabled( false ),
  _runLoop( TimerRunLoop::getSingleton() )
{
}
TimerImpl::~TimerImpl ()
{
}

void TimerImpl::stopRunLoop()
{
  TimerRunLoop::getSingleton().stop();
}

void TimerImpl::startRunLoop()
{
  TimerRunLoop::getSingleton().start();
}

void TimerImpl::setEnabled( bool enabled )
{
  if( enabled != _enabled )
  {
    _enabled = enabled;

    if( _enabled )
    {
      _runLoop.registerTimer( *this );
    }
    else
    {
      _runLoop.unRegisterTimer( *this );
    }
  }
}
}
