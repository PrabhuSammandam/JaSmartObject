/*
 * task_test.cpp
 *
 *  Created on: 07-Sep-2017
 *      Author: prabhu
 */

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

class ConsumerThreadMsgHandler : public ITaskMsgHandler
{
  public:
    void HandleMsg( void *msg ) override
    {
      printf( "Got msg %d\n", *((uint16_t*)msg) );
    }

    void DeleteMsg( void *msg ) override
    {
    }
};

PtrMsgQ<10>              gs_thread_consumer_msg_q;
ConsumerThreadMsgHandler consumer_thread_msg_q_handler;

class ProducerThread : public ITaskRoutine
{
  public:
    void Run( void *arg ) override
    {
    	uint16_t count = 0;
      Task *consumer_thread = (Task *) arg;

      while( true )
      {
        sleep( 1 );
        consumer_thread->SendMsg(&count);
        count++;
      }
    }
};

ProducerThread producer_thread_routine;

int main()
{
  OsalMgr::Inst()->Init();

  auto consumer_thread = OsalMgr::Inst()->AllocTask();

  if( consumer_thread == nullptr )
  {
    printf( "Failed to allocate the consumer task\n" );
    return ( 1 );
  }

  TaskMsgQParam consumer_task_msg_q_param;

  consumer_task_msg_q_param.msgQ           = &gs_thread_consumer_msg_q;
  consumer_task_msg_q_param.taskMsgHandler = &consumer_thread_msg_q_handler;

  consumer_thread->InitWithMsgQ( (uint8_t *) "consumer", 0, 0, &consumer_task_msg_q_param, nullptr );
  consumer_thread->Start();

  auto producer_thread = OsalMgr::Inst()->AllocTask();

  if( producer_thread == nullptr )
  {
    printf( "Failed to allocate the producer task\n" );
    return ( 1 );
  }

  producer_thread->Init( (uint8_t *) "producer", 0, 0, &producer_thread_routine, consumer_thread );
  producer_thread->Start();

  Semaphore *sema = OsalMgr::Inst()->alloc_semaphore();

  sema->Init( 0, 0 );
  sema->Wait();

  return ( 0 );
}
