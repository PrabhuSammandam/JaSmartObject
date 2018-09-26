/*
 * task_test.cpp
 *
 *  Created on: 07-Sep-2017
 *      Author: prabhu
 */

#include <functional>
#include <iostream>
#include <OsalError.h>
#include <OsalMgr.h>
#include <PtrMsgQ.h>
#include <stdio.h>
#include <Sem.h>
#include <Task.h>
#include <unistd.h>
#include <cstdint>
#include <semaphore.h>

using namespace ja_iot::osal;
using namespace ja_iot::base;
using namespace std;

PtrMsgQ<10> gs_thread_consumer_msg_q;

void producer_thread_function( void *arg )
{
  uint16_t count           = 0;
  Task *   consumer_thread = (Task *) arg;

  while( true )
  {
    sleep( 1 );
    consumer_thread->SendMsg( &count );
    count++;
  }
}

void consumer_task_handle_msg_cb( void *pv_task_arg, void *pv_user_data )
{
	cout << "Got new message " << *( (uint16_t *) pv_task_arg ) << endl;
}
void consumer_task_delete_msg_cb( void *pv_task_arg, void *pv_user_data )
{
}

int main()
{
  OsalMgr::Inst()->Init();

  auto consumer_thread = OsalMgr::Inst()->AllocTask();

  if( consumer_thread == nullptr )
  {
    printf( "Failed to allocate the consumer task\n" );
    return ( 1 );
  }

  consumer_thread->Init( { "consumer", 0, 0, &gs_thread_consumer_msg_q,
                           consumer_task_handle_msg_cb, nullptr, consumer_task_delete_msg_cb, nullptr } );

  consumer_thread->Start();

  auto producer_thread = OsalMgr::Inst()->AllocTask();

  if( producer_thread == nullptr )
  {
    printf( "Failed to allocate the producer task\n" );
    return ( 1 );
  }

  producer_thread->Init({"producer", 0, 0, producer_thread_function, consumer_thread});
  producer_thread->Start();

  Semaphore *sema = OsalMgr::Inst()->alloc_semaphore();

  sema->Init( 0, 1 );
  sema->Wait();

  while( true )
  {
    sleep( 10 );
  }

  return ( 0 );
}
