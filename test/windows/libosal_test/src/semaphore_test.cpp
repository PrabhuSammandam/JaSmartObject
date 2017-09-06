#include "semaphore_test.h"
#include "cute.h"
#include "OsalMgr.h"
#include "Sem.h"

using namespace ja_iot::osal;

void semaphore_test_1()
{
  OsalMgr::Inst()->Init();

  auto semaphore = OsalMgr::Inst()->alloc_semaphore();

  ASSERT( semaphore != nullptr );

  auto ret_status = semaphore->Init(0, 1);

  ASSERT( ret_status == OsalError::OK );

  ret_status  = semaphore->Post();
  ASSERT( ret_status == OsalError::OK );

  ret_status  = semaphore->Wait();
  ASSERT( ret_status == OsalError::OK );

  ret_status  = semaphore->Wait();
  ASSERT( ret_status == OsalError::OK );
}

cute::suite make_suite_semaphore_test()
{
  cute::suite s{};
  s.push_back( CUTE( semaphore_test_1 ) );
  return ( s );
}
