#include <iostream>
#include <OsalRandom.h>
#include "OsalMgr.h"
#include "OsalTimer.h"
#include "IMemAllocator.h"
#include "umm_malloc.h"
#include <vector>
#include "SimpleAllocator.h"
#include "ResPropValue.h"

using namespace std;
using namespace ja_iot::base;
using namespace ja_iot::osal;
using namespace ja_iot::memory;
using namespace JaCoAP;
using namespace ja_iot::stack;

extern void test_uuid();

int main()
{
	test_uuid();
	return 0;
}

int main__( int argc, char *argv[] )
{
  const auto mem_allocator = MemAllocatorFactory::create_mem_allocator( MemAlloctorType::kWindows );
  MemAllocatorFactory::set( mem_allocator );
  OsalMgr::Inst()->Init();

  auto semaphore = OsalMgr::Inst()->alloc_semaphore();

  semaphore->Init( 0, 1 );
  semaphore->Wait();

  return ( 0 );
}