#pragma once

#include <byte_array.h>
#include <common_defs.h>
#include <common/coap_msg.h>
#include <ErrCode.h>

namespace ja_iot {
namespace coap {
using ByteArray = ja_iot::base::ByteArray;
using ErrCode   = ja_iot::base::ErrCode;

class MsgCodec
{
  public:

    MsgCodec ();

    static void    ParseOnly( _in_ ByteArray &byteArray, _out_ CoapMsgHdr &msg );
    static ErrCode Decode( _in_ ByteArray &byteArray, _out_ CoapMsg &msg );
    static ErrCode Encode( _in_ CoapMsg &msg, _out_ ByteArray &byteArray );

  private:
    static ErrCode EncodeOptions( CoapMsg &msg, uint8_t *buffer, uint16_t *bufLen );
    static ErrCode ParseOptions( CoapMsg &msg, uint8_t *inBuffer, uint16_t optionLen, uint8_t **payloadStart );
    static ErrCode ParseOnlyOptions( uint8_t *inBuffer, uint16_t optionLen, uint8_t **payloadStart );
};
}
}
