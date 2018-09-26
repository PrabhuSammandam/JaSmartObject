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

extern void run_cbor_test_suite();

int main( int argc, char *argv[] )
{
  const auto mem_allocator = MemAllocatorFactory::create_mem_allocator( MemAlloctorType::kWindows );
  MemAllocatorFactory::set( mem_allocator );
  OsalMgr::Inst()->Init();

  run_cbor_test_suite();

  auto semaphore = OsalMgr::Inst()->alloc_semaphore();

  semaphore->Init( 0, 1 );
  semaphore->Wait();

  return ( 0 );
}