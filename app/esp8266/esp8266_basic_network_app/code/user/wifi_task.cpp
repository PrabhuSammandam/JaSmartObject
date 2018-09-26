/*
 * wifi_task.cpp
 *
 *  Created on: Sep 10, 2017
 *      Author: psammand
 */

#if 1
#include <cstdint>
#include "esp_common.h"
#include <OsalError.h>
#include <OsalMgr.h>
#include <PtrMsgQ.h>
#include <stdio.h>
#include <string.h>
#include <ScopedMutex.h>
#include <wifi_task.h>
#include <adapter_task.h>

using namespace ja_iot::osal;
using namespace ja_iot::base;

void task_handle_msg_cb(void *pv_msg, void *pv_user_data);
void task_delete_msg_cb(void *pv_msg, void *pv_user_data);

#define PSTR( s ) ( __extension__( { static const char __c[] __attribute__( ( section( ".irom0.text" ) ) ) = ( s ); &__c[0]; } ) )
// #define	PSTR(s) s

#define dbg( format, ... ) printf( PSTR( format "\n" ), ## __VA_ARGS__ )
#define DBG_INFO( format, ... ) printf( PSTR( format "\n" ), ## __VA_ARGS__ )
#define DBG_WARN( format, ... ) printf( PSTR( format "\n" ), ## __VA_ARGS__ )
#define DBG_ERROR( format, ... ) printf( PSTR( format "\n" ), ## __VA_ARGS__ )
#define DBG_FATAL( format, ... ) printf( PSTR( format "\n" ), ## __VA_ARGS__ )

static void wifi_task_cb( System_Event_t *evt );

AdapterTask                     gs_adapter_task;
extern WifiTask                 gs_wifi_task;
PtrMsgQ<WIFI_TASK_MSG_Q_LENGTH> wifi_task_msg_q_;

bool WifiTask::create_task()
{
  OsalError osal_status;

  DBG_INFO( "WifiTask::create_task:%d# ENTER", __LINE__ );
  wifi_task_ = OsalMgr::Inst()->AllocTask();

  if( wifi_task_ == nullptr )
  {
    DBG_ERROR( "WifiTask::create_task:%d# AllocTask FAILED", __LINE__ );
    return ( false );
  }

  access_mutex_ = OsalMgr::Inst()->AllocMutex();

  if( access_mutex_ == nullptr )
  {
    DBG_ERROR( "WifiTask::create_task:%d# AllocMutex FAILED", __LINE__ );
    return ( false );
  }

  access_mutex_->Lock();

  osal_status = wifi_task_->Init( {WIFI_TASK_NAME, WIFI_TASK_PRIORITY, WIFI_TASK_STACK_LENGTH,
	  &wifi_task_msg_q_, task_handle_msg_cb, this, task_delete_msg_cb, this});

  if( osal_status != OsalError::OK )
  {
    DBG_ERROR( "WifiTask::create_task:%d# Task Init FAILED", __LINE__ );
    return ( false );
  }

  wifi_task_->Start();

  access_mutex_->Unlock();

  DBG_ERROR( "WifiTask::create_task:%d# connecting as station", __LINE__ );
  wifi_set_event_handler_cb( wifi_task_cb );
  wifi_set_opmode( STATION_MODE );

  struct station_config config;
  bzero( &config, sizeof( struct station_config ) );
  // sprintf((char*)config.ssid, (char*)"Cisco17377");
  sprintf( (char *) config.ssid, (char *) "JinjuAmla" );
  sprintf( (char *) config.password, (char *) "Jinju124Amla" );
  wifi_station_set_config( &config );

  wifi_station_connect();

  return ( true );
}

bool WifiTask::send_msg( WifiTaskMsg *msg )
{
  ScopedMutex lock{ access_mutex_ };

  auto new_msg = new WifiTaskMsg();// msg_list_.Alloc();

  if( new_msg == nullptr )
  {
    DBG_ERROR( "WifiTask::send_msg:%d# Alloc Msg FAILED", __LINE__ );
    return ( false );
  }

  memcpy( new_msg, (const void *) msg, sizeof( WifiTaskMsg ) );

  OsalError osal_status = wifi_task_->SendMsg( new_msg );

  if( osal_status != OsalError::OK )
  {
    DBG_ERROR( "WifiTask::send_msg:%d# Task SendMsg FAILED", __LINE__ );
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

  if( WifiTaskMsgType::STATION_GOT_IP == msg->msgType )
  {
    DBG_INFO( "WifiTask::handle_msg:%d# Starting adapter_task", __LINE__ );
    gs_adapter_task.create_task();
  }
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

  DBG_INFO( "wifi_task_cb:%d# Callback event[%d]", __LINE__, evt->event_id );

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

      DBG_INFO( "wifi_task_cb:%d# Sending msg to WiFi Task", __LINE__ );
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


void task_handle_msg_cb(void *pv_msg, void *pv_user_data)
{
	static_cast<WifiTask*>(pv_user_data)->handle_msg((WifiTaskMsg *)pv_msg);
}

void task_delete_msg_cb(void *pv_msg, void *pv_user_data)
{
	static_cast<WifiTask*>(pv_user_data)->delete_msg((WifiTaskMsg *)pv_msg);
}

#endif
