#include <base_utils.h>
#include <common/coap_msg.h>
#include <IMemAllocator.h>
#include <Hash.h>
#include <stdio.h>

using namespace ja_iot;

namespace ja_iot {
namespace coap {
CoapMsg::CoapMsg ()
{
  Init();
}

CoapMsg::~CoapMsg ()
{
  Reset();
}

void CoapMsg::Init()
{
  _type         = MsgType::NONE;
  _code         = MsgCode::EMPTY;
  _tknLen       = 0;
  _id           = -1;
  _tkn          = 0;
  _epHndl       = 0xFF;
  _status       = 0;
  _payloadBlock = nullptr;

  SetLastRes( true );

  _optSet.Init();
}

void CoapMsg::Tkn( uint8_t *buff, uint8_t len )
{
  for( auto i = 0; i < len; i++ )
  {
    _tkn |= uint64_t( buff[i] ) << i * 8;
  }
}

void CoapMsg::Reset()
{
  if( _payloadBlock != nullptr )
  {
    mdelete _payloadBlock;
    _payloadBlock = nullptr;
  }

  _optSet.Release();

  Init();
}

void CoapMsg::Print() const
{
  printf( "Message:\n" );
  printf( "\tType           :%d\n", uint8_t( _type ) );
  printf( "\tCode           :%d\n", uint8_t( _code ) );
  printf( "\tMsgId          :%x\n", _id );
  printf( "\tTokenLen       :%u\n", _tknLen );
  printf( "\tToken          :%I64u\n", _tkn );
  printf( "\tOptionCount    :%u\n", _optSet.GetNoOfOptions() );

  if( _payloadBlock != nullptr )
  {
    printf( "\tPayloadLength  :%u\n", _payloadBlock->get_len() );
  }

  printf( "\n" );
}

void CoapMsg::SetDuplicate( bool setValue )
{
  setValue ? base::set_bit( _status, coap_msg_bitmask_duplicate ) : base::clear_bit( _status, coap_msg_bitmask_duplicate );
}

void CoapMsg::SetTimeout( bool setValue )
{
  setValue ? base::set_bit( _status, coap_msg_bitmask_timeout ) : base::clear_bit( _status, coap_msg_bitmask_timeout );
}

void CoapMsg::SetAcknowledged( bool setValue )
{
  setValue ? base::set_bit( _status, coap_msg_bitmask_acknowledged ) : base::clear_bit( _status, coap_msg_bitmask_acknowledged );
}

void CoapMsg::SetCancelled( bool setValue )
{
  setValue ? base::set_bit( _status, coap_msg_bitmask_cancelled ) : base::clear_bit( _status, coap_msg_bitmask_cancelled );
}

void CoapMsg::SetRejected( bool setValue )
{
  setValue ? base::set_bit( _status, coap_msg_bitmask_rejected ) : base::clear_bit( _status, coap_msg_bitmask_rejected );
}

void CoapMsg::SetLastRes( bool setValue )
{
  setValue ?
  base::set_bit( _status, coap_msg_bitmask_last_response ) : base::clear_bit( _status, coap_msg_bitmask_last_response );
}

bool CoapMsg::HasBlockOption() const
{
  return ( _optSet.HasBlock1() || _optSet.HasBlock2() );
}

uint32_t CoapMsg::GetHashBySrcEpAndMid()
{
  uint32_t hashVal = _srcSocketAddr.GetHashValue();

  hashVal = base::Hash::get_hash( &_id, sizeof( uint16_t ), hashVal );

  return ( hashVal );
}

uint32_t CoapMsg::GetHashBySrcEpAndTkn()
{
  uint32_t hashVal = _srcSocketAddr.GetHashValue();

  hashVal = base::Hash::get_hash( &_tkn, sizeof( uint16_t ), hashVal );

  return ( hashVal );
}

uint32_t CoapMsg::GetHashByDstEpAndMid()
{
  uint32_t hashVal = _dstSocketAddr.GetHashValue();

  hashVal = base::Hash::get_hash( &_id, sizeof( uint16_t ), hashVal );

  return ( hashVal );
}

uint32_t CoapMsg::GetHashByDstEpAndTkn()
{
  uint32_t hashVal = _dstSocketAddr.GetHashValue();

  hashVal = base::Hash::get_hash( &_tkn, sizeof( uint16_t ), hashVal );

  return ( hashVal );
}

uint32_t CoapMsg::GetHashBySrcEpAndUri()
{
  return ( 0 ); // TODO ( Utils::HashByUri( this, _srcSocketAddr ) );
}

uint32_t CoapMsg::GetHashByDstEpAndUri()
{
  return ( 0 ); // TODO ( Utils::HashByUri( this, _dstSocketAddr ) );
}

CoapMsg * CoapMsg::UpdateForResFromReq( MsgCode msgCode, CoapMsg *req )
{
  _code                = msgCode;
  this->_dstSocketAddr = req->_srcSocketAddr;
  this->_epHndl        = req->_epHndl;
  return ( this );
}

CoapMsg * CoapMsg::UpdateForResFromRes( MsgCode msgCode, CoapMsg *res )
{
  _code                = msgCode;
  this->_dstSocketAddr = res->_dstSocketAddr;
  this->_epHndl        = res->_epHndl;
  return ( this );
}
}
}

#if 0
void * ja_iot::coap::CoapMsg::operator new ( size_t size )
{
#ifdef ENABLE_MEM_DEBUG
  printf( "Alloc Type[%-20s] size[%-6d] ", "Msg", (uint32_t) size );
#endif
  return ( static_cast<CoapMsg *>( MemMgr::AllocMsg() ) );
}

void ja_iot::coap::CoapMsg::operator delete ( void *ptr )
{
#ifdef ENABLE_MEM_DEBUG
  printf( "Free  Type[%-20s] ", "Msg" );
#endif
  MemMgr::FreeMsg( ( static_cast<CoapMsg *>( ptr ) ) );
}
#endif
