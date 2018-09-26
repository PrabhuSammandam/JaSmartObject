/*
 * wifi_task.cpp
 *
 *  Created on: Sep 10, 2017
 *      Author: psammand
 */

#include <cstdint>
#include "esp_common.h"
#include <OsalError.h>
#include <OsalMgr.h>
#include <PtrMsgQ.h>
#include <stdio.h>
#include <string.h>
#include <ScopedMutex.h>
#include <wifi_task.h>

using namespace ja_iot::osal;
using namespace ja_iot::base;

#define dbg( format, ... ) printf( format "\n", ## __VA_ARGS__ )

static void wifi_task_cb( System_Event_t *evt );

extern WifiTask                 gs_wifi_task;
PtrMsgQ<WIFI_TASK_MSG_Q_LENGTH> wifi_task_msg_q_;

bool WifiTask::create_task()
{
  OsalError osal_status;

  dbg( "WifiTask::create_task=>Creating task\n" );
  wifi_task_ = OsalMgr::Inst()->AllocTask();

  if( wifi_task_ == nullptr )
  {
    dbg( "Failed to create WIFI_TASK" );
    return ( false );
  }

  dbg( "WifiTask::create_task=>Creating mutex\n" );
  access_mutex_ = OsalMgr::Inst()->AllocMutex();

  if( access_mutex_ == nullptr )
  {
    dbg( "Failed to create access mutex" );
    return ( false );
  }

  task_creation_params_t st_task_creation_params = {};

  st_task_creation_params.cz_name               = WIFI_TASK_NAME;
  st_task_creation_params.u16_stack_size        = WIFI_TASK_STACK_LENGTH;
  st_task_creation_params.u32_priority          = WIFI_TASK_PRIORITY;
  st_task_creation_params.pcz_msg_queue         = &wifi_task_msg_q_;
  st_task_creation_params.pfn_handle_msg        = task_handle_msg_cb;
  st_task_creation_params.pv_handle_msg_cb_data = this;
  st_task_creation_params.pfn_delete_msg        = task_delete_msg_cb;
  st_task_creation_params.pv_delete_msg_cb_data = this;

  osal_status = wifi_task_->Init( &st_task_creation_params );

  if( osal_status != OsalError::OK )
  {
    dbg( "Failed to init task" );
    return ( false );
  }

  wifi_task_->Start();

  dbg( "WifiTask::create_task=>Connecting as station\n" );
  wifi_set_event_handler_cb( wifi_task_cb );
  wifi_set_opmode( STATION_MODE );
  wifi_station_connect();

  return ( true );
}

bool WifiTask::send_msg( WifiTaskMsg *msg )
{
  ScopedMutex lock{ access_mutex_ };

  auto new_msg = new WifiTaskMsg();// msg_list_.Alloc();

  if( new_msg == nullptr )
  {
    dbg( "Failed to alloc msg" );
    return ( false );
  }

  memcpy( new_msg, (const void *) msg, sizeof( WifiTaskMsg ) );

  dbg( "%s=>Sending msg", __FUNCTION__ );

  OsalError osal_status = wifi_task_->SendMsg( new_msg );

  if( osal_status != OsalError::OK )
  {
    dbg( "Failed to send msg" );
    return ( false );
  }

  dbg( "%s=>Sending msg success", __FUNCTION__ );

  return ( true );
}

bool WifiTask::send_msg( WifiTaskMsgType msg_type, void *msg_param )
{
  WifiTaskMsg msg{ msg_type,
                   msg_param };

  return ( send_msg( &msg ) );
}

void WifiTask::handle_msg( WifiTaskMsg *msg )
{
  dbg( "Got msg, type %d", (int) msg->msgType );
}

void WifiTask::delete_msg( WifiTaskMsg *msg )
{
  if( msg != nullptr )
  {
    delete ( msg );
  }
}

static void wifi_task_cb( System_Event_t *evt )
{
  WifiTaskMsg msg{};

  dbg( "WIFI cb event [%x]", evt->event_id );

  switch( evt->event_id )
  {
    case EVENT_STAMODE_SCAN_DONE:
    {
      dbg( "Scan done :" );
    }
    break;

    case EVENT_STAMODE_CONNECTED:
    {
      dbg( "connected to ssid [%s], channel [%d]", evt->event_info.connected.ssid, evt->event_info.connected.channel );
    }
    break;

    case EVENT_STAMODE_DISCONNECTED:
    {
      dbg( "disconnect from ssid [%s], reason [%d]", evt->event_info.disconnected.ssid, evt->event_info.disconnected.reason );
    }
    break;

    case EVENT_STAMODE_AUTHMODE_CHANGE:
    {
      dbg( "mode: %d -> %d", evt->event_info.auth_change.old_mode, evt->event_info.auth_change.new_mode );
    }
    break;

    case EVENT_STAMODE_GOT_IP:
    {
      dbg( "ip:" IPSTR ",mask:" IPSTR ",gw:" IPSTR, IP2STR( &evt->event_info.got_ip.ip ), IP2STR( &evt->event_info.got_ip.mask ), IP2STR( &evt->event_info.got_ip.gw ) );
      dbg( "" );
      msg.msgType = WifiTaskMsgType::STATION_GOT_IP;
      msg.msgData = NULL;

      dbg( "%s=>Sending msg", __FUNCTION__ );
      gs_wifi_task.send_msg( &msg );
    }
    break;

    case EVENT_SOFTAPMODE_STACONNECTED:
    {
      dbg( "station: " MACSTR "join, AID = %d", MAC2STR( evt->event_info.sta_connected.mac ), evt->event_info.sta_connected.aid );
    }
    break;

    case EVENT_SOFTAPMODE_STADISCONNECTED:
    {
      dbg( "station: " MACSTR "leave, AID = %d", MAC2STR( evt->event_info.sta_disconnected.mac ), evt->event_info.sta_disconnected.aid );
    }
    break;

    default:
    {
    }
    break;
  }
}
void WifiTask::task_handle_msg_cb( void *pv_msg, void *pv_user_data )
{
  static_cast<WifiTask *>( pv_user_data )->handle_msg( (WifiTaskMsg *) pv_msg );
}

void WifiTask::task_delete_msg_cb( void *pv_msg, void *pv_user_data )
{
  static_cast<WifiTask *>( pv_user_data )->delete_msg( (WifiTaskMsg *) pv_msg );
}