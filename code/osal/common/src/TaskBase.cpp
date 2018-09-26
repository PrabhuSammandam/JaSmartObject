/*
 * TaskBase.cpp
 *
 *  Created on: Sep 7, 2017
 *      Author: psammand
 */

#include <stdio.h>
#include <cstring>
#include <common/inc/TaskBase.h>
#include <MsgQ.h>
#include <Mutex.h>
#include <OsalError.h>
#include <OsalMgr.h>
#include <ScopedMutex.h>
#include <Sem.h>

// #define _DEBUG_

#ifdef _DEBUG_
#define dbg( format, ... ) printf( format "\n", ## __VA_ARGS__ )
#else
#define dbg( format, ... )
#endif

namespace ja_iot {
namespace osal {
static constexpr uint8_t k_bit_is_to_stop       = 0;
static constexpr uint8_t k_bit_is_msg_q_enabled = 1;

OsalError TaskBase::Init( task_creation_params_t *pst_task_creation_params )
{
  if( pst_task_creation_params->pcz_msg_queue != nullptr )
  {
    if( ( pst_task_creation_params->pfn_handle_msg == nullptr ) || ( pst_task_creation_params->pcz_msg_queue->GetCapacity() <= 0 ) )
    {
      return ( OsalError::INVALID_ARGS );
    }

    _msg_q_mutex = OsalMgr::Inst()->AllocMutex();

    if( _msg_q_mutex == nullptr )
    {
      return ( OsalError::OUT_OF_MEMORY );
    }

    _msg_q_semaphore = OsalMgr::Inst()->alloc_semaphore();

    if( _msg_q_semaphore == nullptr )
    {
      OsalMgr::Inst()->FreeMutex( _msg_q_mutex );
      return ( OsalError::OUT_OF_MEMORY );
    }

    _msg_q_semaphore->Init( 0, pst_task_creation_params->pcz_msg_queue->GetCapacity() );

    _st_task_params = *pst_task_creation_params;

    _bits.set( k_bit_is_to_stop );
    _bits.set( k_bit_is_msg_q_enabled );
  }
  else
  {
    _bits.reset( k_bit_is_msg_q_enabled );
    _st_task_params = *pst_task_creation_params;
  }

  return (OsalError::OK);
}

OsalError TaskBase::Init( const task_creation_params_t &task_creation_params )
{
  return ( Init( const_cast<task_creation_params_t *>( &task_creation_params ) ) );
}

OsalError TaskBase::Start()
{
  auto status = OsalError::OK;

  if( ( _bits[k_bit_is_msg_q_enabled] == true ) && ( _bits[k_bit_is_to_stop] == true ) )
  {
    dbg( "%s=>Creating msg q task\n", __FUNCTION__ );
    ScopedMutex scoped_mutex( _msg_q_mutex );
    _bits.reset( k_bit_is_to_stop );
    status = port_create_task();

    if( status != OsalError::OK )
    {
      _bits.set( k_bit_is_to_stop );
    }

    dbg( "%s=>Created task succesfully\n", __FUNCTION__ );
  }
  else
  {
    status = port_create_task();
  }

  return ( status );
}

OsalError TaskBase::Stop()
{
  /* get the lock to modify the shared variable */
  ScopedMutex scoped_mutex( _msg_q_mutex );

  if( _bits[k_bit_is_to_stop] == false )
  {
    _bits.set( k_bit_is_to_stop );

    /* inform the thread that there are some signals sent */
    _msg_q_semaphore->Post();
  }

  return ( OsalError::OK );
}

OsalError TaskBase::Destroy()
{
  if( _bits[k_bit_is_to_stop] == false )
  {
    return ( OsalError::ERR );
  }

  if( _st_task_params.pcz_msg_queue != nullptr )
  {
    if( _msg_q_mutex != nullptr )
    {
      _msg_q_mutex->Lock();
    }

    while( _st_task_params.pcz_msg_queue->IsEmpty() == false )
    {
      const auto msg = _st_task_params.pcz_msg_queue->Dequeue();

      if( ( msg != nullptr ) && ( this->_st_task_params.pfn_delete_msg != nullptr ) )
      {
        this->_st_task_params.pfn_delete_msg( msg, this->_st_task_params.pv_delete_msg_cb_data );
      }
    }

    if( _msg_q_mutex != nullptr )
    {
      _msg_q_mutex->Unlock();
    }
  }

  if( _msg_q_mutex != nullptr )
  {
    OsalMgr::Inst()->FreeMutex( _msg_q_mutex );
  }

  _msg_q_mutex = nullptr;

  if( _msg_q_semaphore != nullptr )
  {
    _msg_q_semaphore->Uninit();
    OsalMgr::Inst()->free_semaphore( _msg_q_semaphore );
  }

  _msg_q_semaphore = nullptr;

  port_delete_task();

	_st_task_params.clear();

  return ( OsalError::OK );
}

OsalError TaskBase::SendMsg( void *pv_msg )
{
  if( ( pv_msg == nullptr ) || ( _st_task_params.pcz_msg_queue == nullptr ) )
  {
    return ( OsalError::INVALID_ARGS );
  }

  dbg( "%s=>ENTER\n", __FUNCTION__ );

  ScopedMutex scoped_mutex( _msg_q_mutex );

  if( _st_task_params.pcz_msg_queue->Enqueue( pv_msg ) == true )
  {
    if( _msg_q_semaphore != nullptr )
    {
      _msg_q_semaphore->Post();
      dbg( "%s=>Msg sent successfully, msg_q_sem %p\n", __FUNCTION__, _msg_q_semaphore );
      return ( OsalError::OK );
    }

    dbg( "%s=>msg_q_sem NULL\n", __FUNCTION__ );
  }
  else
  {
      dbg( "%s=>enque failed\n", __FUNCTION__ );
  }

  return ( OsalError::ERR );
}


void TaskBase::Run()
{
  if( _bits[k_bit_is_msg_q_enabled] == true )
  {
    while( _bits[k_bit_is_to_stop] == false )
    {
      // dbg( "%s=>**********************************going to wait******************\n", __FUNCTION__ );

      _msg_q_semaphore->Wait();

      _msg_q_mutex->Lock();

      /* if it is exited from wait condition, first check whether stop signal is
       * send by anyone */
      if( _bits[k_bit_is_to_stop] == true )
      {
        _msg_q_mutex->Unlock();
        continue;
      }

      const auto new_msg = _st_task_params.pcz_msg_queue->Dequeue();

      _msg_q_mutex->Unlock();

      if( new_msg != nullptr )
      {
        dbg( "%s=>Calling task handler\n", __FUNCTION__ );

        if( this->_st_task_params.pfn_handle_msg )
        {
          this->_st_task_params.pfn_handle_msg( new_msg, this->_st_task_params.pv_handle_msg_cb_data );
        }

        if( this->_st_task_params.pfn_delete_msg )
        {
          this->_st_task_params.pfn_delete_msg( new_msg, this->_st_task_params.pv_delete_msg_cb_data );
        }
      }
    }
  }
  else
  {
    if( this->_st_task_params.pfn_run_cb )
    {
      this->_st_task_params.pfn_run_cb( this->_st_task_params.pv_task_arg );
    }
  }
}
}
}
