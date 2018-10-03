#include "mutex_test.h"
#include "cute.h"
#include "Mutex.h"
#include "OsalMgr.h"

using namespace ja_iot::osal;

void mutex_test_1()
{
  OsalMgr::Inst()->Init();
  Mutex *mutex1 = OsalMgr::Inst()->AllocMutex();

  ASSERT( mutex1 != nullptr );

  auto ret_status = mutex1->Lock();
  ASSERT( ret_status == OsalError::OK );

  ret_status = mutex1->Unlock();
  ASSERT( ret_status == OsalError::OK );

  OsalMgr::Inst()->FreeMutex( mutex1 );
}

cute::suite make_suite_mutex_test()
{
  cute::suite s{};
  s.push_back( CUTE( mutex_test_1 ) );
  return ( s );
}