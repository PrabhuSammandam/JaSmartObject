#include "timer_impl.hpp"
#include "tp_timer.hpp"

namespace ThreadPoolTimer {
Timer::Timer( std::function<void()> task )
{
  _impl = new TimerImpl( task );
}

Timer::~Timer ()
{
  delete _impl;
}

void Timer::set_interval( const std::chrono::nanoseconds &interval )
{
  _impl->setInterval( interval );
}

void Timer::set_enabled( bool enabled )
{
  _impl->setEnabled( enabled );
}

bool Timer::is_enabled()
{
  return ( _impl->getEnabled() );
}

void Timer::stopRunLoop()
{
  TimerImpl::stopRunLoop();
}

void Timer::startRunLoop()
{
  TimerImpl::startRunLoop();
}
}