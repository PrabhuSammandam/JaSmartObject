/*
 * adapter_task.cpp
 *
 *  Created on: Sep 13, 2017
 *      Author: psammand
 */

#include <FreeRTOS.h>
#include <task.h>
#include <adapter_task.h>
#include <adapter_mgr.h>
#include <OsalMgr.h>
#include <connectivity_mgr.h>

using namespace ja_iot::osal;
using namespace ja_iot::base;
using namespace ja_iot::network;

#define dbg( format, ... ) printf( format "\n", ## __VA_ARGS__ )
#define DBG_INFO( format, ... ) printf( format "\n", ## __VA_ARGS__ )
#define DBG_ERROR( format, ... ) printf( format "\n", ## __VA_ARGS__ )

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

  osal_status = adapter_task_->Init( (uint8_t *) "adap_task", 2, 256, (ITaskRoutine *) this, this );

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

void AdapterTask::Run( void *arg )
{
  ConnectivityManager::Inst().select_network( kAdapterType_ip );
  ConnectivityManager::Inst().start_listening_server();

  while( 1 )
  {
    ConnectivityManager::Inst().handle_request_response();
//	  AdapterManager::Inst().ReadData();
//    DBG_INFO("AdapterTask::Run:%d# Running", __LINE__);
    vTaskDelay( 200 / portTICK_RATE_MS );
  }
}
