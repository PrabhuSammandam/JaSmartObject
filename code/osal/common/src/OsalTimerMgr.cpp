#include <algorithm>
#include <chrono>
#include "common/inc/OsalTimerMgr.h"
#include "OsalMgr.h"
#include "OsTime.h"
#include "ScopedMutex.h"
#include "OsalTimer.h"

namespace ja_iot {
namespace osal {
constexpr uint16_t          OSAL_TIMER_MGR_TASK_STACK_SIZE = 512;
constexpr uint16_t          OSAL_TIMER_MGR_TASK_PRIORITY   = 2;
constexpr const char *const OSAL_TIMER_MGR_TASK_NAME       = "OSAL_TMR";
constexpr uint8_t           OSAL_TIMER_MGR_MAX_TIMERS      = 10;

void task_run_cb( void *pv_task_arg );
bool timer_compare( OsalTimer *timer1, OsalTimer *timer2 );

#define CHECK_NULL( __VAR__, __RET__ ) if( __VAR__ == nullptr ){ return __RET__; \
}
#define CHECK_FOR_ERROR( __VAR__, __ERR__, __RET__ ) if( __VAR__ != __ERR__ ){ return __RET__; \
}

OsalError OsalTimerMgr::initialize()
{
  if( _is_initialised == true )
  {
    return ( OsalError::OK );
  }

  _list_mutex = OsalMgr::Inst()->AllocMutex();
  CHECK_NULL( _list_mutex, OsalError::OUT_OF_MEMORY );

  _queue_sem = OsalMgr::Inst()->alloc_semaphore();
  CHECK_NULL( _queue_sem, OsalError::OUT_OF_MEMORY );

  auto osal_error = _queue_sem->Init( 0, OSAL_TIMER_MGR_MAX_TIMERS );
  CHECK_FOR_ERROR( osal_error, OsalError::OK, osal_error );

  _task = OsalMgr::Inst()->AllocTask();
  CHECK_NULL( _task, OsalError::OUT_OF_MEMORY );

  osal_error = _task->Init( { OSAL_TIMER_MGR_TASK_NAME, OSAL_TIMER_MGR_TASK_PRIORITY,
                              OSAL_TIMER_MGR_TASK_STACK_SIZE, task_run_cb, this } );
  CHECK_FOR_ERROR( osal_error, OsalError::OK, osal_error );

  _is_running = true;
  osal_error  = _task->Start();
  CHECK_FOR_ERROR( osal_error, OsalError::OK, osal_error );

  _is_initialised = true;
  _now_time_us    = OsTime::get_current_time_us();
  _prev_time_us   = (uint32_t) _now_time_us;

  return ( osal_error );
}

OsalError OsalTimerMgr::register_timer( OsalTimer *timer )
{
  CHECK_NULL( timer, OsalError::INVALID_ARGS );

  ScopedMutex lock{ _list_mutex };

  update_now_time();
  timer->_trigger_time_us = _now_time_us + timer->_duration_us;

  _timers_list.push_back( timer );

  sort_timers();

  _queue_sem->Post();

  return ( OsalError::OK );
}

/* this function will not call delete for the passed timer object. Users need to delete the timer object */
OsalError OsalTimerMgr::unregister_timer( OsalTimer *timer )
{
  CHECK_NULL( timer, OsalError::INVALID_ARGS );

  ScopedMutex lock{ _list_mutex };

  bool found = false;
  int  i     = 0;

  for( auto &loop_timer : _timers_list )
  {
    if( timer == loop_timer )
    {
      _timers_list.erase( _timers_list.cbegin() + i );
      found = true;
      break;
    }

    i++;
  }

  if( found )
  {
    _queue_sem->Post();
  }

  return ( OsalError::OK );
}

OsalError OsalTimerMgr::reset_timer( OsalTimer *timer )
{
  CHECK_NULL( timer, OsalError::INVALID_ARGS );

  ScopedMutex lock{ _list_mutex };

  bool found = false;

  for( auto &loop_timer : _timers_list )
  {
    if( timer == loop_timer )
    {
      found = true;
      break;
    }
  }

  if( found )
  {
    update_now_time();
    timer->_trigger_time_us = _now_time_us + timer->_duration_us;
    sort_timers();

    _queue_sem->Post();

    return ( OsalError::OK );
  }

  return ( OsalError::ERR );
}
void OsalTimerMgr::task_loop()
{
  uint32_t sem_wait_time_ms = 1000;

  while( _is_running )
  {
    update_now_time();

    if( trigger_timers() )
    {
      continue;
    }

    if( _timers_list.size() > 0 )
    {
      sem_wait_time_ms = 1000;
    }
    else
    {
      sem_wait_time_ms = 10000;
    }

    _queue_sem->Wait( sem_wait_time_ms );
  }
}

uint64_t OsalTimerMgr::get_system_time()
{
  update_now_time();
  return ( _now_time_us );
}

void OsalTimerMgr::sort_timers()
{
  std::sort( _timers_list.begin(), _timers_list.end(), timer_compare );
}

bool OsalTimerMgr::trigger_timers()
{
  ScopedMutex lock{ _list_mutex };

  if( _timers_list.size() == 0 )
  {
    return ( false );
  }

  auto timer = _timers_list[0];

  if( timer->_trigger_time_us <= _now_time_us )
  {
    if( timer->_is_one_shot == false )
    {
      timer->_trigger_time_us = _now_time_us + timer->_duration_us;
    }
    else
    {
      _timers_list.erase( _timers_list.begin() );
    }

    if( timer->_timer_cb != nullptr )
    {
      timer->_timer_cb( timer->_user_param1, timer->_user_param2 );
    }

    if( timer->_is_one_shot == false )
    {
      sort_timers();
    }

    return ( true );
  }

  return ( false );
}

void OsalTimerMgr::update_now_time()
{
  uint32_t current_time_us = OsTime::get_current_time_us();

  _now_time_us += ( current_time_us - _prev_time_us );
  _prev_time_us = current_time_us;
}

bool timer_compare( OsalTimer *timer1, OsalTimer *timer2 )
{
  return ( timer1->_trigger_time_us < timer2->_trigger_time_us );
}

void task_run_cb( void *pv_task_arg )
{
  if( pv_task_arg != nullptr )
  {
    ( (OsalTimerMgr *) pv_task_arg )->task_loop();
  }
}
}
}
