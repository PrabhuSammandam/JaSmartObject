/*
 * wifi_task.h
 *
 *  Created on: Sep 10, 2017
 *      Author: psammand
 */

#ifndef CODE_INCLUDE_WIFI_TASK_H_
#define CODE_INCLUDE_WIFI_TASK_H_

#include "export/Task.h"
#include "export/Mutex.h"
#include "SimpleList.h"

const char *const        WIFI_TASK_NAME             = "wifi_task";
constexpr unsigned long  WIFI_TASK_PRIORITY         = 2;
constexpr unsigned short WIFI_TASK_STACK_LENGTH     = 256;
constexpr unsigned short WIFI_TASK_MSG_Q_LENGTH     = 5;
constexpr unsigned long  WIFI_TASK_MSG_Q_TIMEOUT_MS = 1000;

enum class WifiTaskState
{
  IDLE,
  CONNECTED,
  DISCONNECTED,
  WPS_STARTED
};

enum class WifiTaskMsgType
{
  STATION_GOT_IP,
  WPS_SUCCESS,
  WPS_FAILED,
  WPS_START
};

struct WifiTaskMsg
{
  WifiTaskMsgType   msgType;
  void *            msgData;
};

class WifiTask
{
  public:
    bool create_task();
    bool send_msg( WifiTaskMsg *msg );
    bool send_msg( WifiTaskMsgType msg_type, void *msg_param );

  private:
    void handle_msg( WifiTaskMsg *msg );
    void delete_msg( WifiTaskMsg *msg );

  private:
    static void task_handle_msg_cb(void *pv_msg, void *pv_user_data);
    static void task_delete_msg_cb(void *pv_msg, void *pv_user_data);

  private:
    ja_iot::osal::Task *                                            wifi_task_    = nullptr;
    ja_iot::osal::Mutex *                                           access_mutex_ = nullptr;
};

#endif /* CODE_INCLUDE_WIFI_TASK_H_ */
