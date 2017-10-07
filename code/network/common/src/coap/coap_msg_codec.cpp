/*
 * coap_msg_codec.cpp
 *
 *  Created on: Sep 27, 2017
 *      Author: psammand
 */

#include <string.h>
#include <common/inc/coap_msg_codec.h>
#include <konstants_network.h>
#include <IMemAllocator.h>

namespace ja_iot {
namespace network {
ErrCode CoapMsgCodec::parse_header( _in_ const uint8_t *pu8_coap_pdu, _in_ const uint16_t u16_coap_pdu_len, _out_ CoapMsgHdr *pcz_coap_msg_hdr )
{
  ErrCode ret_status = ErrCode::OK;

  if( ( pu8_coap_pdu == nullptr ) || ( pcz_coap_msg_hdr == nullptr ) )
  {
    return ( ErrCode::INVALID_PARAMS );
  }

  if( u16_coap_pdu_len < 4 )
  {
    return ( ErrCode::MSG_FORMAT_ERROR );
  }

  if( ( pu8_coap_pdu[0] >> 6 ) != 0x01 )
  {
    return ( ErrCode::INVALID_COAP_VERSION );
  }

  pcz_coap_msg_hdr->_type = ( static_cast<CoapMsgType>( ( pu8_coap_pdu[0] >> 4 ) & 0x03 ) );
  uint8_t tknLen = pu8_coap_pdu[0] & 0x0F;

  if( tknLen > 8 )
  {
    return ( ErrCode::MSG_FORMAT_ERROR );
  }

  pcz_coap_msg_hdr->_code = static_cast<CoapMsgCode>( pu8_coap_pdu[1] );

  /* Section 4.1: An Empty message has the Code field set to 0.00.
   * The Token Length field MUST be set to 0 and bytes of data
   * MUST NOT be present after the Message ID field. If there
   * are any bytes, they MUST be processed as a message format error.*/
  if( pcz_coap_msg_hdr->_code == CoapMsgCode::EMPTY )
  {
    if( ( u16_coap_pdu_len != 4 ) || ( tknLen != 0 ) )
    {
      return ( ErrCode::MSG_FORMAT_ERROR );
    }
  }

  uint8_t msgCodeClass = uint8_t( pcz_coap_msg_hdr->_code ) >> 5;

  if( ( msgCodeClass == 1 ) || ( msgCodeClass == 6 ) || ( msgCodeClass == 7 ) )
  {
    return ( ErrCode::MSG_FORMAT_ERROR );
  }

  pcz_coap_msg_hdr->_id = static_cast<uint16_t>( ( pu8_coap_pdu[2] << 8 ) | pu8_coap_pdu[3] );

  uint16_t currByteIdx = 4;

  if( u16_coap_pdu_len == currByteIdx )
  {
    return ( ret_status );
  }

  currByteIdx += tknLen;

  if( u16_coap_pdu_len == currByteIdx )
  {
    return ( ret_status );
  }

  pcz_coap_msg_hdr->_payloadStart = nullptr;
  pcz_coap_msg_hdr->_payloadLen   = 0;

  ErrCode status = check_options( &pu8_coap_pdu[currByteIdx], ( u16_coap_pdu_len - currByteIdx ), &pcz_coap_msg_hdr->_payloadStart, &pcz_coap_msg_hdr->_options_count );

  if( status != ErrCode::OK )
  {
    return ( ret_status );
  }

  if( pcz_coap_msg_hdr->_payloadStart != nullptr )
  {
    pcz_coap_msg_hdr->_payloadLen = uint16_t( u16_coap_pdu_len - ( pcz_coap_msg_hdr->_payloadStart - pu8_coap_pdu ) );

    if( pcz_coap_msg_hdr->_payloadLen > JA_COAP_CONFIG_MAX_PAYLOAD_SIZE )
    {
      return ( ErrCode::PAYLOAD_TOO_LARGE );
    }
  }

  return ( ret_status );
}

ErrCode CoapMsgCodec::check_options( _in_ const uint8_t *pu8_options_data_start, _in_ uint16_t u16_options_data_len, _out_ uint8_t **pp_payload_start, _out_ uint8_t *pu8_options_count )
{
  uint16_t cur_buf_idx    = 0;
  uint16_t last_option_no = 0;

  *pu8_options_count = 0;
  *pp_payload_start  = nullptr;

  if( pu8_options_data_start[0] == 0xFF )
  {
    return ( ErrCode::MSG_FORMAT_ERROR );
  }

  while( cur_buf_idx < u16_options_data_len )
  {
    // check for payload marker, if it is there then mark the start of payload and return
    if( pu8_options_data_start[cur_buf_idx] == 0xFF )
    {
      // at least one byte payload must follow to the payload marker
      if( ( u16_options_data_len - cur_buf_idx ) < 2 )
      {
        return ( ErrCode::MSG_FORMAT_ERROR );
      }
      else
      {
        cur_buf_idx++;                         // skip payload marker
        *pp_payload_start = (uint8_t *) &pu8_options_data_start[cur_buf_idx];
        // return ( ErrCode::OK );
      }
    }
    else
    {
      uint16_t option_delta  = pu8_options_data_start[cur_buf_idx] >> 4;                  // initial delta in upper 4 bits
      uint16_t option_length = pu8_options_data_start[cur_buf_idx] & 0x0F;                   // initial length in lower 4 bits
      cur_buf_idx++;                   // skip the initial delta & length byte

      if( option_delta == 13 )
      {
        // An 8-bit unsigned integer follows the initial byte and indicates the Option Delta minus 13.
        option_delta = pu8_options_data_start[cur_buf_idx] + 13;
        cur_buf_idx++;                         // skip 1 byte big delta
      }
      else if( option_delta == 14 )
      {
        // A 16-bit unsigned integer in network byte order follows the initial byte and indicates the Option Delta minus 269.
        option_delta = ( ( uint16_t( pu8_options_data_start[cur_buf_idx] ) << 8 ) | uint16_t( pu8_options_data_start[cur_buf_idx] ) ) + 269;
        cur_buf_idx += 2;
      }
      else if( option_delta == 15 )
      {
        // Reserved for the Payload Marker.
        return ( ErrCode::MSG_FORMAT_ERROR );
      }

      if( option_length == 13 )
      {
        // An 8-bit unsigned integer precedes the Option Value and indicates the Option Length minus 13.
        option_length = pu8_options_data_start[cur_buf_idx] + 13;
        cur_buf_idx++;                         // skip 1 byte big delta
      }
      else if( option_length == 14 )
      {
        // A 16-bit unsigned integer in network byte order precedes the Option Value and indicates the Option Length minus 269.
        option_length = ( ( uint16_t( pu8_options_data_start[cur_buf_idx] ) << 8 ) | uint16_t( pu8_options_data_start[cur_buf_idx] ) ) + 269;
        cur_buf_idx  += 2;
      }
      else if( option_length == 15 )
      {
        // Reserved for future use.
        return ( ErrCode::MSG_FORMAT_ERROR );
      }

      // if current option length is greater than max option size or
      // current option length is greater than remaining payload
      if( ( option_length > JA_COAP_CONFIG_MAX_OPTION_SIZE )
        || ( ( u16_options_data_len - cur_buf_idx ) < option_length ) )
      {
        return ( ErrCode::MSG_FORMAT_ERROR );
      }

      last_option_no += option_delta;                   // encode option delta
      cur_buf_idx    += option_length;
      ( *pu8_options_count )++;
    }
  }

  return ( ErrCode::OK );
}

ErrCode ja_iot::network::CoapMsgCodec::parse_coap_msg( _in_ const uint8_t *pu8_coap_pdu, _in_ const uint16_t u16_coap_pdu_len, _out_ CoapMsg *pcz_coap_msg )
{
  pcz_coap_msg->set_type( static_cast<CoapMsgType>( ( pu8_coap_pdu[0] >> 4 ) & 0x03 ) );
  pcz_coap_msg->set_token_len( ( pu8_coap_pdu[0] & 0x0F ) );
  pcz_coap_msg->set_code( static_cast<CoapMsgCode>( pu8_coap_pdu[1] ) );
  pcz_coap_msg->set_id( static_cast<uint16_t>( ( pu8_coap_pdu[2] << 8 ) | pu8_coap_pdu[3] ) );

  uint16_t currByteIdx = 4;

  if( u16_coap_pdu_len == currByteIdx )
  {
    return ( ErrCode::OK );
  }

  if( pcz_coap_msg->get_token_len() > 0 )
  {
    pcz_coap_msg->set_token( (uint8_t *) &pu8_coap_pdu[currByteIdx], pcz_coap_msg->get_token_len() );
  }

  currByteIdx += pcz_coap_msg->get_token_len();

  if( u16_coap_pdu_len == currByteIdx )
  {
    return ( ErrCode::OK );
  }

  uint8_t *pp_payload_start = nullptr;

  auto     status = parse_coap_options( pcz_coap_msg, &pu8_coap_pdu[currByteIdx], ( u16_coap_pdu_len - currByteIdx ), &pp_payload_start );

  if( status != ErrCode::OK )
  {
    return ( status );
  }

  if( pp_payload_start != nullptr )
  {
    uint16_t payloadLen = uint16_t( u16_coap_pdu_len - ( pp_payload_start - pu8_coap_pdu ) );

    if( payloadLen > JA_COAP_CONFIG_MAX_PAYLOAD_SIZE )
    {
      status = ErrCode::PAYLOAD_TOO_LARGE;
      goto exit_label;
    }

    if( payloadLen > 0 )
    {
      uint8_t *pu8_temp_payload = (uint8_t*)mnew_g( payloadLen );
      ::memcpy( pu8_temp_payload, pp_payload_start, payloadLen );

      pcz_coap_msg->set_payload_len( payloadLen );
      pcz_coap_msg->set_payload( pu8_temp_payload );
    }
  }

exit_label:

  if( status != ErrCode::OK )
  {
    pcz_coap_msg->get_option_set()->FreeOptions();
  }

  return ( ErrCode::OK );
}

ErrCode CoapMsgCodec::parse_coap_options( _in_out_ CoapMsg *pcz_coap_msg, _in_ const uint8_t *pu8_options_start, _in_ const uint16_t u16_options_data_len, _out_ uint8_t **pp_payload_start )
{
  ErrCode  status        = ErrCode::OK;
  uint16_t curByteIndex  = 0;
  uint16_t lastOptionNum = 0;

  *pp_payload_start = nullptr;

  if( pu8_options_start[0] == 0xFF )
  {
    status = ErrCode::MSG_FORMAT_ERROR;
    goto success;
  }

  while( curByteIndex < u16_options_data_len )
  {
    // check for payload marker, if it is there then mark the start of payload and return
    if( pu8_options_start[curByteIndex] == 0xFF )
    {
      // at least one byte payload must follow to the payload marker
      if( ( u16_options_data_len - curByteIndex ) < 2 )
      {
        status = ErrCode::MSG_FORMAT_ERROR;
        goto success;
      }
      else
      {
        curByteIndex++;                         // skip payload marker
        *pp_payload_start = (uint8_t*)&pu8_options_start[curByteIndex];
        status            = ErrCode::OK;
        goto success;
      }
    }
    else
    {
      uint16_t option_delta  = pu8_options_start[curByteIndex] >> 4;                  // initial delta in upper 4 bits
      uint16_t option_length = pu8_options_start[curByteIndex] & 0x0F;                   // initial length in lower 4 bits
      curByteIndex++;                   // skip the initial delta & length byte

      if( option_delta == 13 )
      {
        // An 8-bit unsigned integer follows the initial byte and indicates the Option Delta minus 13.
        option_delta = pu8_options_start[curByteIndex] + 13;
        curByteIndex++;                         // skip 1 byte big delta
      }
      else if( option_delta == 14 )
      {
        // A 16-bit unsigned integer in network byte order follows the initial byte and indicates the Option Delta minus 269.
        option_delta  = ( ( uint16_t( pu8_options_start[curByteIndex] ) << 8 ) | uint16_t( pu8_options_start[curByteIndex] ) ) + 269;
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
        option_length = pu8_options_start[curByteIndex] + 13;
        curByteIndex++;                         // skip 1 byte big delta
      }
      else if( option_length == 14 )
      {
        // A 16-bit unsigned integer in network byte order precedes the Option Value and indicates the Option Length minus 269.
        option_length = ( ( uint16_t( pu8_options_start[curByteIndex] ) << 8 ) | uint16_t( pu8_options_start[curByteIndex] ) ) + 269;
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
      if( ( option_length > JA_COAP_CONFIG_MAX_OPTION_SIZE ) || ( ( u16_options_data_len - curByteIndex ) < option_length ) )
      {
        status = ErrCode::MSG_FORMAT_ERROR;
        goto success;
      }

      lastOptionNum += option_delta;                   // encode option delta

      status = pcz_coap_msg->get_option_set()->AddOption( lastOptionNum, option_length, (uint8_t *) &pu8_options_start[curByteIndex] );

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
    pcz_coap_msg->get_option_set()->FreeOptions();
  }

  return ( status );
}
}
}
