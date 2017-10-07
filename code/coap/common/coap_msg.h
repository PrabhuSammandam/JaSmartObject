#pragma once

#include <cstdint>
#include <ErrCode.h>
#include <byte_array.h>
#include <base_utils.h>
#include <ip_socket_addr.h>
#include <options/coap_option.h>
#include <options/coap_block_option.h>
#include <options/coap_option_set.h>

namespace ja_iot {
namespace coap {
using ErrCode      = ja_iot::base::ErrCode;
using ByteArray    = ja_iot::base::ByteArray;
using IpSocketAddr = ja_iot::base::IpSocketAddr;

enum class MsgType
{
  CON = 0,
  NON,
  ACK,
  RST,
  NONE
};

enum class MsgCode
{
  EMPTY                          = 0,
  GET                            = 1,
  POST                           = 2,
  PUT                            = 3,
  DELETE                         = 4,
  CREATED_201                    = 65,
  DELETED_202                    = 66,
  VALID_203                      = 67,
  CHANGED_204                    = 68,
  CONTENT_205                    = 69,
  CONTINUE_231                   = 95,
  BAD_REQUEST_400                = 128,
  UNAUTHORIZED_401               = 129,
  BAD_OPTION_402                 = 130,
  FORBIDDEN_403                  = 131,
  NOT_FOUND_404                  = 132,
  METHOD_NOT_ALLOWED_405         = 133,
  NOT_ACCEPTABLE_406             = 134,
  REQUEST_ENTITY_INCOMPLETE_408  = 136,
  PRECONDITION_FAILED_412        = 140,
  REQUEST_ENTITY_TOO_LARGE_413   = 141,
  UNSUPPORTED_CONTENT_FORMAT_415 = 143,
  INTERNAL_SERVER_ERROR_500      = 160,
  NOT_IMPLEMENTED_501            = 161,
  BAD_GATEWAY_502                = 162,
  SERVICE_UNAVAILABLE_503        = 163,
  GATEWAY_TIMEOUT_504            = 164,
  PROXYING_NOT_SUPPORTED_505     = 165
};

class CoapMsgHdr
{
  private:
    MsgType    _type         = MsgType::RST;
    MsgCode    _code         = MsgCode::EMPTY;
    uint16_t   _id           = -1;
    ErrCode    _errCode      = ErrCode::OK;
    uint8_t *  _payloadStart = nullptr;
    uint16_t   _payloadLen   = 0;
    uint8_t    _epHndl       = 0xFF;

  public:

    CoapMsgHdr () {}

    MsgType Type() const { return ( _type ); }
    void    Type( MsgType type ) { _type = type; }

    MsgCode Code() const { return ( _code ); }
    void    Code( MsgCode code ) { _code = code; }

    uint16_t Id() const { return ( _id ); }
    void     Id( uint16_t id ) { _id = id; }

    ErrCode Error() const { return ( _errCode ); }
    void    Error( ErrCode errCode ) { _errCode = errCode; }

    uint16_t PayloadLen() const { return ( _payloadLen ); }
    void     PayloadLen( uint16_t len ) { _payloadLen = len; }

    uint8_t* PayloadStart() const { return ( _payloadStart ); }
    void     PayloadStart( uint8_t *payloadStart ) { _payloadStart = payloadStart; }

    bool HasPayload() const { return ( _payloadStart != nullptr ); }
    bool IsConfirmable() const { return ( _type == MsgType::CON ); }
    bool HasMsgId() const { return ( _id != -1 ); }

    uint8_t EpHndl() const { return ( _epHndl ); }
    void    EpHndl( uint8_t epHndl ) { _epHndl = epHndl; }
};

const uint32_t coap_msg_bitmask_last_response = 0x04000000;
const uint32_t coap_msg_bitmask_duplicate     = 0x08000000;
const uint32_t coap_msg_bitmask_acknowledged  = 0x10000000;
const uint32_t coap_msg_bitmask_cancelled     = 0x20000000;
const uint32_t coap_msg_bitmask_rejected      = 0x40000000;
const uint32_t coap_msg_bitmask_timeout       = 0x80000000;

class CoapMsg
{
  public:

    CoapMsg ();

    ~CoapMsg ();

  private:
    MsgType    _type;
    MsgCode    _code;
    uint64_t   _tkn;
    uint16_t   _id;
    uint8_t    _tknLen;
    uint8_t    _epHndl = 0xFF;

    OptionsSet     _optSet;
    ByteArray *    _payloadBlock = nullptr;
    IpSocketAddr   _srcSocketAddr;
    IpSocketAddr   _dstSocketAddr;
    uint32_t       _status = 0;

  public:
    void Init();

    MsgType Type() const { return ( _type ); }
    void    Type( MsgType type ) { _type = type; }

    MsgCode Code() const { return ( _code ); }
    void    Code( MsgCode code ) { _code = code; }

    uint16_t Id() const { return ( _id ); }
    void     Id( uint16_t id ) { _id = id; }

    void    TknLen( uint8_t tknLen ) { _tknLen = tknLen; }
    uint8_t TknLen() const { return ( _tknLen ); }

    uint64_t Tkn() const { return ( _tkn ); }
    void     Tkn( uint64_t token ) { _tkn = token; }
    void     Tkn( uint8_t *buff, uint8_t len );

    OptionsSet* GetOptionSet() { return ( &_optSet ); }

    ByteArray* Payload() const { return ( _payloadBlock ); }
    CoapMsg*   Payload( ByteArray *plBuf ) { _payloadBlock = plBuf; return ( this ); }
    uint32_t   GetPayloadSize() const { return ( ( _payloadBlock != nullptr ) ? _payloadBlock->get_len() : 0 ); }

    void Reset();
    void Print() const;

    IpSocketAddr* SrcSocketAddr() { return ( &_srcSocketAddr ); }
    void          SrcSocketAddr( const IpSocketAddr &srcAddr ) { _srcSocketAddr = srcAddr; }

    IpSocketAddr* DstSocketAddr() { return ( &_dstSocketAddr ); }
    void          DstSocketAddr( const IpSocketAddr &dstAddr ) { _dstSocketAddr = dstAddr; }

    uint8_t EpHndl() const { return ( _epHndl ); }
    void    EpHndl( uint8_t epHndl ) { _epHndl = epHndl; }

    bool IsPing() const { return ( _code == MsgCode::EMPTY && _type == MsgType::CON ); }
    bool IsRequest() const { return ( _code > MsgCode::EMPTY && _code <= MsgCode::DELETE ); }
    bool IsResponse() const { return ( _code >= MsgCode::CREATED_201 ); }
    bool IsEmpty() const { return ( _code == MsgCode::EMPTY ); }

    bool IsConfirmable() const { return ( _type == MsgType::CON ); }
    bool HasMsgId() const { return ( _id != 0xFFFF ); }
    bool IsNonConfirmable() const { return ( _type == MsgType::NON ); }
    bool IsAck() const { return ( _type == MsgType::ACK ); }
    bool IsRst() const { return ( _type == MsgType::RST ); }

    bool IsDuplicate() const { return ( ja_iot::base::is_bit_set( _status, coap_msg_bitmask_duplicate ) ); }
    void SetDuplicate( bool setValue );

    bool IsTimeout() const { return ( ja_iot::base::is_bit_set( _status, coap_msg_bitmask_timeout ) ); }
    void SetTimeout( bool setValue );

    bool IsAcknowledged() const { return ( ja_iot::base::is_bit_set( _status, coap_msg_bitmask_acknowledged ) ); }
    void SetAcknowledged( bool setValue );

    bool IsCancelled() const { return ( ja_iot::base::is_bit_set( _status, coap_msg_bitmask_cancelled ) ); }
    void SetCancelled( bool setValue );

    bool IsRejected() const { return ( ja_iot::base::is_bit_set( _status, coap_msg_bitmask_rejected ) ); }
    void SetRejected( bool setValue );

    bool IsLastRes() const { return ( ja_iot::base::is_bit_set( _status, coap_msg_bitmask_last_response ) ); }
    void SetLastRes( bool setValue );

    bool HasBlockOption() const;

    uint32_t GetHashBySrcEpAndMid();
    uint32_t GetHashBySrcEpAndTkn();

    uint32_t GetHashByDstEpAndMid();
    uint32_t GetHashByDstEpAndTkn();

    uint32_t GetHashBySrcEpAndUri();
    uint32_t GetHashByDstEpAndUri();

    CoapMsg* UpdateForResFromReq( MsgCode msgCode, CoapMsg *req );
    CoapMsg* UpdateForResFromRes( MsgCode msgCode, CoapMsg *res );

#if 0
    void * operator new ( size_t size );
    void operator   delete ( void *ptr );
#endif
};

typedef CoapMsg Request;
typedef CoapMsg Response;
}
}