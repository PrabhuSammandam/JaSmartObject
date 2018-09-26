/*
 * Task.h
 *
 *  Created on: Jun 27, 2017
 *      Author: psammand
 */

#pragma once

#include <cstdint>
#include <string>

#include "OsalError.h"
#include "MsgQ.h"

namespace ja_iot {
namespace osal {
typedef void ( *pfn_task_run_cb ) ( void *pv_task_arg );
typedef void ( *pfn_task_handle_msg_cb ) ( void *pv_msg, void *pv_user_data );
typedef void ( *pfn_task_delete_msg_cb ) ( void *pv_msg, void *pv_user_data );

struct task_creation_params_t
{
  task_creation_params_t(){}
  task_creation_params_t( const std::string &cz_name, uint32_t u32_priority, uint16_t u16_stack_size, base::MsgQ *pcz_msg_queue, pfn_task_handle_msg_cb pfn_handle_msg, void *pv_handle_msg_cb_data, pfn_task_delete_msg_cb pfn_delete_msg, void *pv_delete_msg_cb_data )
    : cz_name{ cz_name },
    u32_priority{ u32_priority },
    u16_stack_size{ u16_stack_size },
    pcz_msg_queue{ pcz_msg_queue },
    pfn_handle_msg{ pfn_handle_msg },
    pv_handle_msg_cb_data{ pv_handle_msg_cb_data },
    pfn_delete_msg{ pfn_delete_msg },
    pv_delete_msg_cb_data{ pv_delete_msg_cb_data }
  {
  }
  task_creation_params_t( const std::string &cz_name, uint32_t u32_priority, uint16_t u16_stack_size, pfn_task_run_cb pfn_run_cb, void *pv_task_arg )
    : cz_name{ cz_name },
    u32_priority{ u32_priority },
    u16_stack_size{ u16_stack_size },
    pfn_run_cb{ pfn_run_cb },
    pv_task_arg{ pv_task_arg }
  {
  }

	void clear()
	{
		cz_name.clear();
		u32_priority=0;
		u16_stack_size=0;
		pcz_msg_queue=nullptr;
		pfn_run_cb = nullptr;
		pfn_handle_msg = nullptr;
		pv_handle_msg_cb_data = nullptr;
		pfn_delete_msg = nullptr;
		pv_delete_msg_cb_data = nullptr;
		pv_task_arg = nullptr;
	}

  std::string              cz_name{};
  uint32_t                 u32_priority{};
  uint16_t                 u16_stack_size{};
  base::MsgQ *             pcz_msg_queue{};
  pfn_task_run_cb          pfn_run_cb{};
  pfn_task_handle_msg_cb   pfn_handle_msg{};
  void *                   pv_handle_msg_cb_data{};
  pfn_task_delete_msg_cb   pfn_delete_msg{};
  void *                   pv_delete_msg_cb_data{};
  void *                   pv_task_arg{};
};

class Task
{
  public:
    virtual ~Task ()
    {
    }

    virtual OsalError Init( task_creation_params_t *pst_task_creation_params ) = 0;
    virtual OsalError Init(const task_creation_params_t &task_creation_params) = 0;
    virtual OsalError Start()                                                  = 0;
    virtual OsalError Stop()                                                   = 0;
    virtual OsalError Destroy()                                                = 0;
    virtual OsalError Wait()                                                   = 0;

    virtual OsalError SendMsg( void *pv_msg ) = 0;
};
}
}