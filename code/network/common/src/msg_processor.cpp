/*
 * MsgProcessor.cpp
 *
 *  Created on: Sep 16, 2017
 *      Author: psammand
 */

#include <adapter_mgr.h>
#include <common/inc/logging_network.h>
#include <common/inc/msg_processor.h>
#include <Task.h>
#include <OsalMgr.h>
#include <config_network.h>
#include <PtrMsgQ.h>

using  namespace ja_iot::base;
using namespace ja_iot::osal;
// using namespace ja_iot::network;
// using namespace ja_iot::memory;

namespace ja_iot {
namespace network {
class MsgProcessorImpl : public IAdapterManagerDataHandler
{
  public:

    MsgProcessorImpl( MsgProcessor *host ) : host_{ host } {}

    ErrCode initialize();

    void HandleError( Endpoint const &end_point, const uint8_t *data, uint16_t data_len, ErrCode error ) override;
    void HandlePacketReceived( Endpoint const &end_point, const uint8_t *data, uint16_t data_len ) override;

    void handle_sender_task_msg( void *msg );
    void delete_sender_task_msg( void *msg );

    void set_event_handler( MsgProcessorEventHandler *msg_processor_event_handler ) { msg_processor_event_handler_ = msg_processor_event_handler; }

    /***
     * Sender task message handler.
     */
    DECLARE_TASK_MSG_HANDLER_CLASS( MsgProcessorSenderTaskMsgHandler, MsgProcessorImpl, handle_sender_task_msg, delete_sender_task_msg );

  public:
    Task *                     sender_task_                 = nullptr;
    Task *                     receiver_task_               = nullptr;
    MsgProcessorEventHandler * msg_processor_event_handler_ = nullptr;
    MsgProcessor *             host_;

    PtrMsgQ<MSG_PROCESSOR_TASK_MAX_NO_MSGS>   sender_task_msg_q;
    MsgProcessorSenderTaskMsgHandler sender_task_msg_handler{ this };
};

ErrCode MsgProcessorImpl::initialize()
{
  ErrCode   ret_status = ErrCode::OK;
  OsalError osal_error = OsalError::OK;

  DBG_INFO( "MsgProcessorImpl::initialize:%d# ENTER", __LINE__ );

  sender_task_ = OsalMgr::Inst()->AllocTask();

  if( sender_task_ == nullptr )
  {
    DBG_ERROR( "MsgProcessorImpl::initialize:%d# alloc sender_task FAILED", __LINE__ );
    ret_status = ErrCode::ERR; goto exit_label_;
  }

  TaskMsgQParam taskMsgQParam;
  taskMsgQParam.msgQ           = &sender_task_msg_q;
  taskMsgQParam.taskMsgHandler = &sender_task_msg_handler;

  osal_error = sender_task_->InitWithMsgQ( (uint8_t *) MSG_PROCESSOR_TASK_NAME, MSG_PROCESSOR_TASK_PRIORITY, MSG_PROCESSOR_TASK_PRIORITY, &taskMsgQParam, this );

  if( osal_error != OsalError::OK )
  {
    DBG_ERROR( "MsgProcessorImpl::initialize:%d# FAILED sender_task init", __LINE__ );
    ret_status = ErrCode::ERR; goto exit_label_;
  }

  osal_error = sender_task_->Start();

  if( osal_error != OsalError::OK )
  {
    DBG_ERROR( "MsgProcessorImpl::initialize:%d# FAILED sender_task start", __LINE__ );
    ret_status = ErrCode::ERR; goto exit_label_;
  }

exit_label_:
  DBG_INFO( "MsgProcessorImpl::initialize:%d# EXIT status %d", __LINE__, (int) ret_status );

  return ( ret_status );
}

MsgProcessor::MsgProcessor ()
{
  pimpl_ = new MsgProcessorImpl{ this };
}

ErrCode MsgProcessor::initialize( AdapterType adapter_type )
{
  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "MsgProcessor::initialize:%d# ENTER adapter_type[%x]", __LINE__, (int) adapter_type );

  ret_status = pimpl_->initialize();

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR( "MsgProcessor::initialize:%d# pimpl_->initialize FAILED", __LINE__ );
    goto exit_label_;
  }

  AdapterManager::Inst().SetAdapterDataHandler( pimpl_ );

  ret_status = AdapterManager::Inst().InitializeAdapters( adapter_type );

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR( "MsgProcessor::initialize:%d# AdapterManager::InitializeAdapters FAILED", __LINE__ );
    goto exit_label_;
  }

exit_label_:
  DBG_INFO( "MsgProcessor::initialize:%d# EXIT status %d", __LINE__, (int) ret_status );

  return ( ret_status );
}


ErrCode MsgProcessor::terminate()
{
  return ( ErrCode::OK );
}

ErrCode MsgProcessor::set_msg_processor_event_handler( MsgProcessorEventHandler *msg_processor_event_handler )
{
  this->pimpl_->set_event_handler( msg_processor_event_handler );

  return ( ErrCode::OK );
}

MsgProcessorEventHandler::MsgProcessorEventHandler ()
{
}

MsgProcessorEventHandler::~MsgProcessorEventHandler ()
{
}
}
}