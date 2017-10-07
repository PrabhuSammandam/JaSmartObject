#include <coap_config.h>
#include <common/coap_msg.h>
#include <parser/msg_codec.h>
#include <cstring>

using namespace ja_iot::base;

namespace ja_iot {
namespace coap {
MsgCodec::MsgCodec ()
{
}

void MsgCodec::ParseOnly( ByteArray &byte_array, CoapMsgHdr &coap_msg )
{
  coap_msg.Error( ErrCode::OK );

  if( ( byte_array.get_array() == nullptr ) || byte_array.is_empty() )
  {
    coap_msg.Error( ErrCode::INVALID_PARAMS );
    return;
  }

  if( byte_array.get_len() < 4 )
  {
    coap_msg.Error( ErrCode::MSG_FORMAT_ERROR );
    return;
  }

  if( ( byte_array[0] >> 6 ) != 0x01 )
  {
    coap_msg.Error( ErrCode::INVALID_COAP_VERSION );
    return;
  }

  coap_msg.Type( static_cast<MsgType>( ( byte_array[0] >> 4 ) & 0x03 ) );
  uint8_t token_len = byte_array[0] & 0x0F;

  if( token_len > 8 )
  {
    coap_msg.Error( ErrCode::MSG_FORMAT_ERROR );
    return;
  }

  coap_msg.Code( static_cast<MsgCode>( byte_array[1] ) );

  /* Section 4.1: An Empty message has the Code field set to 0.00.
   * The Token Length field MUST be set to 0 and bytes of data
   * MUST NOT be present after the Message ID field. If there
   * are any bytes, they MUST be processed as a message format error.*/
  if( coap_msg.Code() == MsgCode::EMPTY )
  {
    if( ( byte_array.get_len() != 4 ) || ( token_len != 0 ) )
    {
      coap_msg.Error( ErrCode::MSG_FORMAT_ERROR );
      return;
    }
  }

  uint8_t msg_code_class = uint8_t( coap_msg.Code() ) >> 5;

  if( ( msg_code_class == 1 ) || ( msg_code_class == 6 ) || ( msg_code_class == 7 ) )
  {
    coap_msg.Error( ErrCode::MSG_FORMAT_ERROR );
    return;
  }

  coap_msg.Id( static_cast<uint16_t>( ( byte_array[2] << 8 ) | byte_array[3] ) );

  uint16_t cur_byte_idx = 4;

  if( byte_array.get_len() == cur_byte_idx )
  {
    return;
  }

  cur_byte_idx += token_len;

  if( byte_array.get_len() == cur_byte_idx )
  {
    return;
  }

  uint8_t *p_payload_start = nullptr;

  ErrCode  status = ParseOnlyOptions( &( byte_array.get_array() )[cur_byte_idx],
      ( byte_array.get_len() - cur_byte_idx ),
      &p_payload_start );

  if( status != ErrCode::OK )
  {
    coap_msg.Error( status );
    return;
  }

  coap_msg.PayloadStart( nullptr );
  coap_msg.PayloadLen( 0 );

  if( p_payload_start != nullptr )
  {
    uint16_t payload_len = uint16_t( byte_array.get_len() - ( p_payload_start - ( byte_array.get_array() ) ) );

    if( payload_len > JA_COAP_CONFIG_MAX_PAYLOAD_SIZE )
    {
      coap_msg.Error( ErrCode::PAYLOAD_TOO_LARGE );
      return;
    }

    coap_msg.PayloadStart( p_payload_start );
    coap_msg.PayloadLen( payload_len );
  }
}

ErrCode MsgCodec::Decode( ByteArray &byteArray, CoapMsg &msg )
{
  msg.Reset();
  msg.Type( static_cast<MsgType>( ( byteArray[0] >> 4 ) & 0x03 ) );
  msg.TknLen( ( byteArray[0] & 0x0F ) );
  msg.Code( static_cast<MsgCode>( byteArray[1] ) );
  msg.Id( static_cast<uint16_t>( ( byteArray[2] << 8 ) | byteArray[3] ) );

  uint16_t currByteIdx = 4;

  if( byteArray.get_len() == currByteIdx )
  {
    return ( ErrCode::OK );
  }

  if( msg.TknLen() > 0 )
  {
    msg.Tkn( &( byteArray.get_array() )[currByteIdx], msg.TknLen() );
  }

  currByteIdx += msg.TknLen();

  if( byteArray.get_len() == currByteIdx )
  {
    return ( ErrCode::OK );
  }

  uint8_t *payloadStart{ nullptr };

  auto status = ParseOptions( msg, &( byteArray.get_array() )[currByteIdx],
      ( byteArray.get_len() - currByteIdx ), &payloadStart );

  if( status != ErrCode::OK )
  {
    return ( status );
  }

  if( payloadStart != nullptr )
  {
    uint16_t payloadLen = uint16_t(
      byteArray.get_len() - ( payloadStart - ( byteArray.get_array() ) ) );

    if( payloadLen > JA_COAP_CONFIG_MAX_PAYLOAD_SIZE )
    {
      status = ErrCode::PAYLOAD_TOO_LARGE;
      goto exit_label;
    }

    if( payloadLen > 0 )
    {
      auto newPay = mnew ByteArray( payloadLen );
      ::memcpy( newPay->get_array(), payloadStart, payloadLen );
      newPay->set_len( payloadLen );
      msg.Payload( newPay );
    }
  }

exit_label:

  if( status != ErrCode::OK )
  {
    msg.GetOptionSet()->FreeOptions();
  }

  return ( ErrCode::OK );
}

ErrCode MsgCodec::Encode( CoapMsg &msg, ByteArray &byteArray )
{
  uint8_t *outBytes = byteArray.get_array();

  outBytes[0]  = 0x40;
  outBytes[0] |= ( uint8_t( msg.Type() ) & 0x03 ) << 4;
  outBytes[0] |= ( ( msg.TknLen() ) & 0x0F );

  outBytes[1] = uint8_t( msg.Code() );
  outBytes[2] = ( msg.Id() >> 8 ) & 0xFF;
  outBytes[3] = msg.Id() & 0xFF;

  uint16_t curByteIdx = 4;

  for( auto i = 0; i < msg.TknLen(); i++ )
  {
    outBytes[curByteIdx] = uint8_t( msg.Tkn() >> i * 8 & 0xFF );
    curByteIdx++;
  }

  curByteIdx += msg.TknLen();

  if( msg.GetOptionSet()->GetNoOfOptions() > 0 )
  {
    uint16_t optionLen = 0;
    EncodeOptions( msg, &outBytes[curByteIdx], &optionLen );
    curByteIdx += optionLen;
  }

  auto plBuf = msg.Payload();

  if( ( plBuf != nullptr ) && ( plBuf->get_len() > 0 ) )
  {
    outBytes[curByteIdx++] = 0xFF;

    ::memcpy( &outBytes[curByteIdx], plBuf->get_array(), plBuf->get_len() );

    curByteIdx += plBuf->get_len();
  }

  byteArray.set_len( curByteIdx );

  return ( ErrCode::OK );
}

ErrCode MsgCodec::EncodeOptions( CoapMsg &msg, uint8_t *outBuff, uint16_t *bufLen )
{
  uint16_t curByteIdx   = 0;     // Current "Write" Position while packing the options array to the byte array
  uint16_t lastOptionNo = 0;

  *bufLen = 0;

  auto it = msg.GetOptionSet()->OptList()->begin();

  auto curOption = *it;

  do
  {
    uint16_t curDelta = curOption->GetNo() - lastOptionNo;
    lastOptionNo = curOption->GetNo();

    uint16_t curOptionLen      = curOption->GetLen();
    uint16_t curOptionStartIdx = curByteIdx;
    curByteIdx++;
    outBuff[curOptionStartIdx] = 0;

    // Delta Bytes
    if( curDelta < 13 )
    {
      outBuff[curOptionStartIdx] |= uint8_t( curDelta ) << 4;
    }
    else if( curDelta < 269 )
    {
      outBuff[curOptionStartIdx] |= uint8_t( 13 ) << 4;
      outBuff[curByteIdx]         = uint8_t( curDelta ) - 13;
      curByteIdx++;
    }
    else
    {
      outBuff[curOptionStartIdx] |= uint8_t( 14 ) << 4;

      outBuff[curByteIdx] = uint8_t( ( curDelta - 269 ) >> 8 );
      curByteIdx++;
      outBuff[curByteIdx] = uint8_t( ( curDelta - 269 ) & 0xff );
      curByteIdx++;
    }

    // Length Bytes
    if( curOptionLen < 13 )
    {
      outBuff[curOptionStartIdx] |= uint8_t( curOptionLen );
    }
    else if( curOptionLen < 269 )
    {
      outBuff[curOptionStartIdx] |= uint8_t( 13 );
      outBuff[curByteIdx]         = uint8_t( curOptionLen ) - 13;
      curByteIdx++;
    }
    else
    {
      outBuff[curOptionStartIdx] |= uint8_t( 14 );

      outBuff[curByteIdx] = uint8_t( ( curOptionLen - 269 ) >> 8 );
      curByteIdx++;
      outBuff[curByteIdx] = uint8_t( ( curOptionLen - 269 ) & 0xff );
      curByteIdx++;
    }

    // Option Values
    auto optValue = curOption->GetVal();

    for( auto t = 0; t < curOptionLen; t++ )
    {
      outBuff[curByteIdx] = optValue[t];
      curByteIdx++;
    }

    ++it;

    if( it == msg.GetOptionSet()->OptList()->end() )
    {
      break;
    }

    curOption = *it;
  } while( true );

  *bufLen = curByteIdx;

  return ( ErrCode::OK );
}

ErrCode MsgCodec::ParseOptions( CoapMsg &msg, uint8_t *inByteArray, uint16_t optionLen, uint8_t **payloadStart )
{
  ErrCode  status        = ErrCode::OK;
  uint16_t curByteIndex  = 0;
  uint16_t lastOptionNum = 0;

  *payloadStart = nullptr;

  if( inByteArray[0] == 0xFF )
  {
    status = ErrCode::MSG_FORMAT_ERROR;
    goto success;
  }

  while( curByteIndex < optionLen )
  {
    // check for payload marker, if it is there then mark the start of payload and return
    if( inByteArray[curByteIndex] == 0xFF )
    {
      // at least one byte payload must follow to the payload marker
      if( ( optionLen - curByteIndex ) < 2 )
      {
        status = ErrCode::MSG_FORMAT_ERROR;
        goto success;
      }
      else
      {
        curByteIndex++;                         // skip payload marker
        *payloadStart = &inByteArray[curByteIndex];
        status        = ErrCode::OK;
        goto success;
      }
    }
    else
    {
      uint16_t option_delta  = inByteArray[curByteIndex] >> 4;                  // initial delta in upper 4 bits
      uint16_t option_length = inByteArray[curByteIndex] & 0x0F;                   // initial length in lower 4 bits
      curByteIndex++;                   // skip the initial delta & length byte

      if( option_delta == 13 )
      {
        // An 8-bit unsigned integer follows the initial byte and indicates the Option Delta minus 13.
        option_delta = inByteArray[curByteIndex] + 13;
        curByteIndex++;                         // skip 1 byte big delta
      }
      else if( option_delta == 14 )
      {
        // A 16-bit unsigned integer in network byte order follows the initial byte and indicates the Option Delta minus 269.
        option_delta = ( ( uint16_t( inByteArray[curByteIndex] ) << 8 )
          | uint16_t( inByteArray[curByteIndex] ) ) + 269;
        curByteIndex += 2;
      }
      else if( option_delta == 15 )
      {
        // Reserved for the Payload Marker.
        status = ErrCode::MSG_FORMAT_ERROR;
        goto success;
      }

      if( option_length == 13 )
      {
        // An 8-bit unsigned integer precedes the Option Value and indicates the Option Length minus 13.
        option_length = inByteArray[curByteIndex] + 13;
        curByteIndex++;                         // skip 1 byte big delta
      }
      else if( option_length == 14 )
      {
        // A 16-bit unsigned integer in network byte order precedes the Option Value and indicates the Option Length minus 269.
        option_length = ( ( uint16_t( inByteArray[curByteIndex] ) << 8 )
          | uint16_t( inByteArray[curByteIndex] ) ) + 269;
        curByteIndex += 2;
      }
      else if( option_length == 15 )
      {
        // Reserved for future use.
        status = ErrCode::MSG_FORMAT_ERROR;
        goto success;
      }

      // if current option length is greater than max option size or
      // current option length is greater than remaining payload
      if( ( option_length > JA_COAP_CONFIG_MAX_OPTION_SIZE )
        || ( ( optionLen - curByteIndex ) < option_length ) )
      {
        status = ErrCode::MSG_FORMAT_ERROR;
        goto success;
      }

      lastOptionNum += option_delta;                   // encode option delta

      status = msg.GetOptionSet()->AddOption( lastOptionNum, option_length,
          &inByteArray[curByteIndex] );

      if( status != ErrCode::OK )
      {
        goto success;
      }

      curByteIndex += option_length;
    }
  }

success:

  if( status != ErrCode::OK )
  {
    msg.GetOptionSet()->FreeOptions();
  }

  return ( status );
}

ErrCode MsgCodec::ParseOnlyOptions( uint8_t *inByteArray, uint16_t optionLen, uint8_t **payloadStart )
{
  uint16_t curByteIndex  = 0;
  uint16_t lastOptionNum = 0;

  *payloadStart = nullptr;

  if( inByteArray[0] == 0xFF )
  {
    return ( ErrCode::MSG_FORMAT_ERROR );
  }

  while( curByteIndex < optionLen )
  {
    // check for payload marker, if it is there then mark the start of payload and return
    if( inByteArray[curByteIndex] == 0xFF )
    {
      // at least one byte payload must follow to the payload marker
      if( ( optionLen - curByteIndex ) < 2 )
      {
        return ( ErrCode::MSG_FORMAT_ERROR );
      }
      else
      {
        curByteIndex++;                         // skip payload marker
        *payloadStart = &inByteArray[curByteIndex];
        return ( ErrCode::OK );
      }
    }
    else
    {
      uint16_t option_delta  = inByteArray[curByteIndex] >> 4;                  // initial delta in upper 4 bits
      uint16_t option_length = inByteArray[curByteIndex] & 0x0F;                   // initial length in lower 4 bits
      curByteIndex++;                   // skip the initial delta & length byte

      if( option_delta == 13 )
      {
        // An 8-bit unsigned integer follows the initial byte and indicates the Option Delta minus 13.
        option_delta = inByteArray[curByteIndex] + 13;
        curByteIndex++;                         // skip 1 byte big delta
      }
      else if( option_delta == 14 )
      {
        // A 16-bit unsigned integer in network byte order follows the initial byte and indicates the Option Delta minus 269.
        option_delta = ( ( uint16_t( inByteArray[curByteIndex] ) << 8 )
          | uint16_t( inByteArray[curByteIndex] ) ) + 269;
        curByteIndex += 2;
      }
      else if( option_delta == 15 )
      {
        // Reserved for the Payload Marker.
        return ( ErrCode::MSG_FORMAT_ERROR );
      }

      if( option_length == 13 )
      {
        // An 8-bit unsigned integer precedes the Option Value and indicates the Option Length minus 13.
        option_length = inByteArray[curByteIndex] + 13;
        curByteIndex++;                         // skip 1 byte big delta
      }
      else if( option_length == 14 )
      {
        // A 16-bit unsigned integer in network byte order precedes the Option Value and indicates the Option Length minus 269.
        option_length = ( ( uint16_t( inByteArray[curByteIndex] ) << 8 )
          | uint16_t( inByteArray[curByteIndex] ) ) + 269;
        curByteIndex += 2;
      }
      else if( option_length == 15 )
      {
        // Reserved for future use.
        return ( ErrCode::MSG_FORMAT_ERROR );
      }

      // if current option length is greater than max option size or
      // current option length is greater than remaining payload
      if( ( option_length > JA_COAP_CONFIG_MAX_OPTION_SIZE )
        || ( ( optionLen - curByteIndex ) < option_length ) )
      {
        return ( ErrCode::MSG_FORMAT_ERROR );
      }

      lastOptionNum += option_delta;                   // encode option delta
      curByteIndex  += option_length;
    }
  }

  return ( ErrCode::OK );
}
}
}