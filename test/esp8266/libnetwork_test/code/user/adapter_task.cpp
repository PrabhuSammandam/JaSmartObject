/*
 * adapter_task.cpp
 *
 *  Created on: Sep 13, 2017
 *      Author: psammand
 */

#include <FreeRTOS.h>
#include <iostream>
#include <chrono>
#include <task.h>
#include <adapter_task.h>
#include <adapter_mgr.h>
#include <OsalMgr.h>
#include <connectivity_mgr.h>
#include "esp_system.h"
#include "OsalTimer.h"
#include "OsalRandom.h"

using namespace std;
using namespace ja_iot::osal;
using namespace ja_iot::base;
using namespace ja_iot::network;

#define PSTR( s ) ( __extension__( { static const char __c[] __attribute__( ( section( ".irom0.text" ) ) ) = ( s ); &__c[0]; } ) )

#define dbg( format, ... ) printf( PSTR( format "\n" ), ## __VA_ARGS__ )
#define DBG_INFO( format, ... ) printf( PSTR( format "\n" ), ## __VA_ARGS__ )
#define DBG_ERROR( format, ... ) printf( PSTR( format "\n" ), ## __VA_ARGS__ )
AdapterTask::AdapterTask ()
{
}

bool AdapterTask::create_task()
{
  OsalError osal_status;

  DBG_INFO( "AdapterTask::create_task:%d# creating task", __LINE__ );
  adapter_task_ = OsalMgr::Inst()->AllocTask();

  if( adapter_task_ == nullptr )
  {
    DBG_ERROR( "AdapterTask::create_task:%d# Alloc failed for task", __LINE__ );
    return ( false );
  }

  task_creation_params_t st_task_creation_params = {};

  st_task_creation_params.cz_name        = "adap_task";
  st_task_creation_params.u16_stack_size = 512;
  st_task_creation_params.u32_priority   = 2;
  st_task_creation_params.pfn_run_cb     = Run;
  st_task_creation_params.pv_task_arg    = this;

  osal_status = adapter_task_->Init( &st_task_creation_params );

  if( osal_status != OsalError::OK )
  {
    DBG_ERROR( "AdapterTask::create_task:%d# Task Init failed", __LINE__ );

    OsalMgr::Inst()->FreeTask( adapter_task_ );
    adapter_task_ = nullptr;

    return ( false );
  }

  osal_status = adapter_task_->Start();

  if( osal_status != OsalError::OK )
  {
    DBG_ERROR( "AdapterTask::create_task:%d# Task Start failed", __LINE__ );

    OsalMgr::Inst()->FreeTask( adapter_task_ );
    adapter_task_ = nullptr;

    return ( false );
  }

  return ( true );
}

int seconds  = 1;
int cb_count = 0;
OsalTimer *timer1{};

void osal_timer_cb( void *user_param1, void *user_param2 )
{
	auto rand_no = OsalRandom::get_random_range(2000, 2000*1.5f);

	printf("random no %d\n", rand_no);
//  printf( "Callback called cb_count[%d], duration[%d]\n", cb_count, seconds );
//
//  if( cb_count++ > 10 )
//  {
//    cb_count = 0;
//    seconds++;
//
//    timer1->restart( seconds * 1000000 );
//  }
}

void AdapterTask::Run( void *arg )
{
  ConnectivityMgr::inst().select_network( k_adapter_type_ip );
  ConnectivityMgr::inst().start_listening_server();
  timer1 = new OsalTimer{ 1000000, osal_timer_cb };

  timer1->start();

  while( 1 )
  {
    ConnectivityMgr::inst().handle_request_response();
    // AdapterManager::Inst().ReadData();
    // DBG_INFO("AdapterTask::Run:%d# Running", __LINE__);
    vTaskDelay( 1000 / portTICK_RATE_MS );

    // cout << PSTR("FreeHeapSize = ") << system_get_free_heap_size() << endl;
    // auto time = std::chrono::system_clock::now();
    // printf( "FreeHeapSize=%d Time = %d Size = %d\n", system_get_free_heap_size(), system_get_time(), sizeof(unsigned long) );
  }
}
