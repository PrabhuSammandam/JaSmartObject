#ifdef _OS_WINDOWS_
#include "windows.h"
#include "time.h"
#include "OsTime.h"

namespace ja_iot {
namespace osal {
#if defined ( _MSC_VER ) || defined ( _MSC_EXTENSIONS )
#define DELTA_EPOCH_IN_MICROSECS 11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS 11644473600000000ULL
#endif

struct timezone
{
  int   tz_minuteswest;              /* minutes W of Greenwich */
  int   tz_dsttime;                  /* type of dst correction */
};

int gettimeofday( struct timeval *tv, struct timezone *tz );

uint32_t OsTime::get_current_time_us()
{
  struct timeval currentTime;

  gettimeofday( &currentTime, NULL );
  return ( currentTime.tv_sec * (int) 1e6 + currentTime.tv_usec );

#if 0
  uint64_t             currentTime = 0;
  static LARGE_INTEGER frequency   = { 0 };

  if( !frequency.QuadPart )
  {
    QueryPerformanceFrequency( &frequency );
  }

  LARGE_INTEGER count = { 0 };
  QueryPerformanceCounter( &count );

  /*
   * T = 1 / f;
   *
   * So (1 / frequency.QuadPart) gives the time period for CPU execution.
   *
   * QueryPerformanceCounter returns the no of time periods passed from the system boot.
   *
   * So to the current time then multiply the returned no of periods with the frequency.
   *
   * time = counter * T; this is in microseconds as per the web docs. Need to still search for correct explanation.
   *
   */

  currentTime = count.QuadPart / ( frequency.QuadPart );
  return ( currentTime );
#endif
}

int gettimeofday( struct timeval *tv, struct timezone *tz )
{
  FILETIME         ft;
  unsigned __int64 tmpres = 0;
  static int       tzflag = 0;

  if( NULL != tv )
  {
    GetSystemTimeAsFileTime( &ft );

    tmpres  |= ft.dwHighDateTime;
    tmpres <<= 32;
    tmpres  |= ft.dwLowDateTime;

    tmpres /= 10;              /*convert into microseconds*/
    /*converting file time to unix epoch*/
    tmpres     -= DELTA_EPOCH_IN_MICROSECS;
    tv->tv_sec  = (long) ( tmpres / 1000000UL );
    tv->tv_usec = (long) ( tmpres % 1000000UL );
  }

  if( NULL != tz )
  {
    if( !tzflag )
    {
      _tzset();
      tzflag++;
    }

    tz->tz_minuteswest = _timezone / 60;
    tz->tz_dsttime     = _daylight;
  }

  return ( 0 );
}
}
}

#endif /* _OS_WINDOWS_ */
