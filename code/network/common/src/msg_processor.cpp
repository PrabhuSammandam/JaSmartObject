/*
 * MsgProcessor.cpp
 *
 *  Created on: Sep 16, 2017
 *      Author: psammand
 */

#include <adapter_mgr.h>
#include <common/inc/logging_network.h>
#include <common/inc/msg_processor.h>
#include <common/inc/coap_msg_codec.h>
#include <Task.h>
#include <OsalMgr.h>
#include <config_network.h>
#include <PtrMsgQ.h>
#include <base_utils.h>
#include <config_mgr.h>
#include <packet_event_handler.h>
#include "adapter_mgr.h"

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
    void HandlePacketReceived( Endpoint const &end_point, const uint8_t *pu8_coap_pdu, uint16_t u16_coap_pdu_len ) override;

    void handle_sender_task_msg( void *msg );
    void delete_sender_task_msg( void *msg );

    void                set_event_handler( PacketEventHandler *pcz_packet_event_handler ) { msg_processor_event_handler_ = pcz_packet_event_handler; }
    PacketEventHandler* get_event_handler() { return ( msg_processor_event_handler_ ); }
    Packet*             create_packet( const uint8_t *data, uint16_t data_len );
    ErrCode             send_msg_multi_adapter( Packet *pcz_packet );
    ErrCode             send_msg( Packet *pcz_packet );
    /***
     * Sender task message handler.
     */
    DECLARE_TASK_MSG_HANDLER_CLASS( MsgProcessorSenderTaskMsgHandler, MsgProcessorImpl, handle_sender_task_msg, delete_sender_task_msg );

  public:
    Task *               sender_task_                 = nullptr;
    Task *               receiver_task_               = nullptr;
    Mutex *              receive_mutex_               = nullptr;
    PacketEventHandler * msg_processor_event_handler_ = nullptr;
    MsgProcessor *       host_;

    PtrMsgQ<MSG_PROCESSOR_SENDER_TASK_MAX_NO_MSGS>   sender_task_msg_q_;
    PtrMsgQ<MSG_PROCESSOR_RECEIVE_MAX_NO_MSGS>       receiver_task_msg_q_;
    MsgProcessorSenderTaskMsgHandler sender_task_msg_handler_{ this };
};

ErrCode MsgProcessorImpl::initialize()
{
  ErrCode   ret_status = ErrCode::OK;
  OsalError osal_error = OsalError::OK;

  DBG_INFO( "MsgProcessorImpl::initialize:%d# ENTER", __LINE__ );

  receive_mutex_ = OsalMgr::Inst()->AllocMutex();

  if( receive_mutex_ == nullptr )
  {
    DBG_ERROR( "MsgProcessorImpl::initialize:%d# alloc receive mutex FAILED", __LINE__ );
    ret_status = ErrCode::ERR; goto exit_label_;
  }

  sender_task_ = OsalMgr::Inst()->AllocTask();

  if( sender_task_ == nullptr )
  {
    DBG_ERROR( "MsgProcessorImpl::initialize:%d# alloc sender_task FAILED", __LINE__ );
    ret_status = ErrCode::ERR; goto exit_label_;
  }

  TaskMsgQParam taskMsgQParam;
  taskMsgQParam.msgQ           = &sender_task_msg_q_;
  taskMsgQParam.taskMsgHandler = &sender_task_msg_handler_;

  osal_error = sender_task_->InitWithMsgQ( (uint8_t *) MSG_PROCESSOR_SENDER_TASK_NAME, MSG_PROCESSOR_SENDER_TASK_PRIORITY, MSG_PROCESSOR_SENDER_TASK_PRIORITY, &taskMsgQParam, this );

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

void MsgProcessorImpl::HandleError( Endpoint const &end_point, const uint8_t *data, uint16_t data_len, ErrCode error )
{
}

Packet * MsgProcessorImpl::create_packet( const uint8_t *data, uint16_t data_len )
{
  return ( nullptr );
}

ErrCode MsgProcessorImpl::send_msg_multi_adapter( Packet *pcz_packet )
{
  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "MsgProcessorImpl::send_msg_multi_adapter:%d# ENTER", __LINE__ );

  auto u16_active_adapters = AdapterManager::Inst().get_active_adapters();

  for( int i = 0; i < ADAPTER_MGR_MAX_NO_ADAPTERS; ++i )
  {
    if( ( u16_active_adapters & ( 1 << i ) ) != 0 )
    {
      pcz_packet->get_end_point()->set_adapter_type( kAdapterType_default );
      pcz_packet->get_end_point()->set_adapter_type( ( 1 << i ) );

      ret_status = send_msg( pcz_packet );

      if( ret_status != ErrCode::OK )
      {
        DBG_ERROR( "MsgProcessorImpl::send_msg_multi_adapter:%d# SendMsg Failed for Adapter[0x%x]", __LINE__, ( 1 << i ) );
        break;
      }
    }
  }

  DBG_INFO( "MsgProcessorImpl::send_msg_multi_adapter:%d# EXIT status %d", __LINE__, (int) ret_status );

  return ( ret_status );
}

ErrCode MsgProcessorImpl::send_msg( Packet *pcz_packet )
{
  ErrCode ret_status     = ErrCode::OK;
  Packet *pcz_new_packet = nullptr;

  DBG_INFO( "MsgProcessorImpl::send_msg:%d# ENTER", __LINE__ );

  pcz_new_packet = new Packet( *pcz_packet );

  if( pcz_new_packet == nullptr )
  {
    DBG_ERROR( "MsgProcessorImpl::send_msg:%d# alloc for new packet FAILED", __LINE__ );
    ret_status = ErrCode::OUT_OF_MEM; goto exit_label_;
  }

  if( sender_task_->SendMsg( pcz_new_packet ) != OsalError::OK )
  {
    DBG_ERROR( "MsgProcessorImpl::send_msg:%d# send msg FAILED ", __LINE__ );
    ret_status = ErrCode::ERR; goto exit_label_;
  }

exit_label_:
  DBG_INFO( "MsgProcessorImpl::send_msg:%d# EXIT status %d", __LINE__, (int) ret_status );

  return ( ret_status );
}


void MsgProcessorImpl::HandlePacketReceived( Endpoint const &end_point, const uint8_t *pu8_coap_pdu, uint16_t u16_coap_pdu_len )
{
  ErrCode ret_status = ErrCode::OK;
  Packet *pcz_packet = nullptr;
  ByteArray cz_resource_uri{};
  CoapMsgHdr cz_coap_msg_hdr{};

  DBG_INFO( "MsgProcessorImpl::HandlePacketReceived:%d# ENTER data_len[%d]", __LINE__, u16_coap_pdu_len );

  if( u16_coap_pdu_len == 0 )
  {
    DBG_ERROR( "MsgProcessorImpl::HandlePacketReceived:%d# received data_len = 0", __LINE__ );
    goto exit_label_;
  }

  ret_status = CoapMsgCodec::parse_header( pu8_coap_pdu, u16_coap_pdu_len, &cz_coap_msg_hdr );

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR( "MsgProcessorImpl::HandlePacketReceived:%d# received data not valid, error[%d]", __LINE__, (int) ret_status );
    goto exit_label_;
  }

  if( cz_coap_msg_hdr.is_request() == true )
  {
    pcz_packet = new RequestPacket();
  }
  else
  {
    pcz_packet = new ResponsePacket();
  }

  ret_status = CoapMsgCodec::parse_coap_msg( pu8_coap_pdu, u16_coap_pdu_len, pcz_packet->get_msg() );

  if( ret_status != ErrCode::OK )
  {
    DBG_ERROR( "MsgProcessorImpl::HandlePacketReceived:%d# Failed to parse coap msg error[%d]", __LINE__, (int) ret_status );
    goto exit_label_;
  }

  cz_resource_uri.set_array( pcz_packet->get_resource_uri(), k_resource_uri_path_max_size, false );

  pcz_packet->get_msg()->get_option_set()->get_uri( cz_resource_uri );

  *( pcz_packet->get_end_point() ) = end_point; // copy the end point

  pcz_packet->set_addressing_method( AddressingMethod::UNICAST );

  DBG_INFO("MsgProcessorImpl::HandlePacketReceived:%d# Received URI[%s]", __LINE__, pcz_packet->get_resource_uri());

  receive_mutex_->Lock();

  if( !receiver_task_msg_q_.Enqueue( pcz_packet ) )
  {
    DBG_ERROR( "MsgProcessorImpl::HandlePacketReceived:%d# Failed enque new packet", __LINE__ );
    delete pcz_packet;
  }

  receive_mutex_->Unlock();

exit_label_:
  DBG_INFO( "MsgProcessorImpl::HandlePacketReceived:%d# EXIT status %d", __LINE__, (int) ret_status );
}

void MsgProcessorImpl::handle_sender_task_msg( void *msg )
{
  Packet *pcz_packet = (Packet *) msg;

  if( pcz_packet == nullptr )
    DBG_ERROR( "MsgProcessorImpl::handle_sender_task_msg:%d# received NULL msg", __LINE__ );
  {
  }

  DBG_INFO( "MsgProcessorImpl::handle_sender_task_msg:%d# ENTER", __LINE__ );

  DBG_INFO( "MsgProcessorImpl::handle_sender_task_msg:%d# EXIT", __LINE__ );
}
void MsgProcessorImpl::delete_sender_task_msg( void *msg )
{
  Packet *pcz_packet = (Packet *) msg;

  if( pcz_packet == nullptr )
  {
    DBG_ERROR( "MsgProcessorImpl::delete_sender_task_msg:%d# received NULL msg", __LINE__ );
  }

  DBG_INFO( "MsgProcessorImpl::delete_sender_task_msg:%d# ENTER", __LINE__ );

  DBG_INFO( "MsgProcessorImpl::delete_sender_task_msg:%d# EXIT", __LINE__ );
}


/**********************************************************************************************************************/
/******************************************  MsgProcessor IMPLEMENTATION  *********************************************/
/**********************************************************************************************************************/

MsgProcessor::MsgProcessor ()
{
  pimpl_ = new MsgProcessorImpl{ this };
}

ErrCode MsgProcessor::initialize( uint16_t adapter_type )
{
  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "MsgProcessor::initialize:%d# ENTER adapter_type[%x]", __LINE__, adapter_type );

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

ErrCode MsgProcessor::send_msg( Packet *pcz_packet )
{
  ErrCode ret_status = ErrCode::OK;

  DBG_INFO( "MsgProcessor::send_msg:%d# ENTER", __LINE__ );

  if( pcz_packet == nullptr )
  {
    DBG_ERROR( "MsgProcessor::send_msg:%d# packet NULL", __LINE__ );
    ret_status = ErrCode::INVALID_PARAMS; goto exit_label_;
  }

  if( AdapterManager::Inst().get_active_adapters() != kAdapterType_default )
  {
    DBG_ERROR( "MsgProcessor::send_msg:%d# No selected adapters", __LINE__ );
    ret_status = ErrCode::INVALID_PARAMS; goto exit_label_;
  }

  if( pcz_packet->is_multicast_addressed() )
  {
    ret_status = pimpl_->send_msg_multi_adapter( pcz_packet );
  }
  else
  {
    ret_status = pimpl_->send_msg( pcz_packet );
  }

exit_label_:
  DBG_INFO( "MsgProcessor::send_msg:%d# EXIT status %d", __LINE__, (int) ret_status );

  return ( ret_status );
}

ErrCode MsgProcessor::set_packet_event_handler( PacketEventHandler *pcz_packet_event_handler )
{
  this->pimpl_->set_event_handler( pcz_packet_event_handler );

  return ( ErrCode::OK );
}

ErrCode MsgProcessor::handle_req_res_callbacks()
{
#ifdef _SINGLE_THREAD_
	AdapterManager::Inst().ReadData();
#else
  this->pimpl_->receive_mutex_->Lock();

  auto pcz_new_packet = (Packet *) this->pimpl_->receiver_task_msg_q_.Dequeue();

  this->pimpl_->receive_mutex_->Unlock();

  /* no valid message received*/
  if( pcz_new_packet == nullptr )
  {
    return ( ErrCode::OK );
  }

  auto event_handler = this->pimpl_->get_event_handler();

  if( event_handler != nullptr )
  {
    PacketEvent cz_packet_event{ pcz_new_packet };

    event_handler->handle_packet_event( &cz_packet_event );
  }

  delete pcz_new_packet;

#endif /* _SINGLE_THREAD_ */
  return ( ErrCode::OK );
}
}
}
