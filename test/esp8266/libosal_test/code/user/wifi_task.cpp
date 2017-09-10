/*
 * wifi_task.cpp
 *
 *  Created on: Sep 10, 2017
 *      Author: psammand
 */

#include <stdio.h>
#include "wifi_task.h"
#include "OsalMgr.h"
#include "PtrMsgQ.h"

using namespace ja_iot::osal;
using namespace ja_iot::base;

PtrMsgQ<WIFI_TASK_MSG_Q_LENGTH> wifi_task_msg_q_;


bool WifiTask::create_task()
{
  wifi_task_ = OsalMgr::Inst()->AllocTask();

  if( wifi_task_ == nullptr )
  {
    printf( "Failed to create WIFI_TASK\n" );
    return ( false );
  }

  TaskMsgQParam task_msg_q_param;

  task_msg_q_param.msgQ = &wifi_task_msg_q_;
  task_msg_q_param.taskMsgHandler = ;

  wifi_task_->InitWithMsgQ(WIFI_TASK_NAME, WIFI_TASK_PRIORITY, WIFI_TASK_STACK_LENGTH, )

  return ( true );
}

bool WifiTask::send_msg( const WifiTaskMsg &msg )
{
  return ( true );
}

bool WifiTask::send_msg( WifiTaskMsgType msg_type, void *msg_param )
{
  return ( true );
}
