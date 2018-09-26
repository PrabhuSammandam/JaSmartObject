/*
 * coap_msg_codec.cpp
 *
 *  Created on: Sep 27, 2017
 *      Author: psammand
 */

#include <string.h>
#include <cstring>
#include <coap/coap_msg_codec.h>
#include <konstants_network.h>
#include <IMemAllocator.h>
#include "coap/coap_consts.h"
#include "config_network.h"

using namespace ja_iot::base;

namespace ja_iot {
namespace network {
static ErrCode decode_coap_options( _in_out_ CoapMsg &pcz_coap_msg, _in_ const uint8_t *pu8_options_start, _in_ const uint16_t u16_options_data_len, _out_ uint8_t **pp_payload_start, OptionsCallback &optionsCallback );
static ErrCode parse_coap_options( _in_ const uint8_t *pu8_options_data_start, _in_ uint16_t u16_options_data_len, _out_ uint8_t **pp_payload_start, _out_ uint8_t &ru8_options_count );

ErrCode CoapMsgCodec::parse_coap_header( _in_ const uint8_t *pu8_coap_pdu, _in_ const uint16_t u16_coap_pdu_len, _out_ CoapMsgHdr &rcz_coap_msg_hdr )
{
  const ErrCode ret_status = ErrCode::OK;

  if( pu8_coap_pdu == nullptr )
  {
    return ( ErrCode::INVALID_PARAMS );
  }

  if( u16_coap_pdu_len < 4 )
  {
    return ( ErrCode::MSG_FORMAT_ERROR );
  }

  rcz_coap_msg_hdr._version = pu8_coap_pdu[0] >> 6 & 0x03;

  if( rcz_coap_msg_hdr._version != 0x01 )
  {
    return ( ErrCode::INVALID_COAP_VERSION );
  }

  rcz_coap_msg_hdr._type      = pu8_coap_pdu[0] >> 4 & 0x03;
  rcz_coap_msg_hdr._token_len = pu8_coap_pdu[0] & 0x0F;

  if( rcz_coap_msg_hdr._token_len > 8 )
  {
    return ( ErrCode::MSG_FORMAT_ERROR );
  }

  rcz_coap_msg_hdr._code = pu8_coap_pdu[1] & 0xFF;

  /* Section 4.1: An Empty message has the Code field set to 0.00.
   * The Token Length field MUST be set to 0 and bytes of msg_data
   * MUST NOT be present after the Message ID field. If there
   * are any bytes, they MUST be processed as a message format error.*/
  if( rcz_coap_msg_hdr._code == COAP_MSG_CODE_EMPTY )
  {
    if( ( u16_coap_pdu_len != 4 ) || ( rcz_coap_msg_hdr._token_len != 0 ) )
    {
      return ( ErrCode::MSG_FORMAT_ERROR );
    }
  }

  const uint8_t msgCodeClass = rcz_coap_msg_hdr._code >> 5;

  if( ( msgCodeClass == 1 ) || ( msgCodeClass == 6 ) || ( msgCodeClass == 7 ) )
  {
    return ( ErrCode::MSG_FORMAT_ERROR );
  }

  rcz_coap_msg_hdr._msg_id = pu8_coap_pdu[2] << 8 | pu8_coap_pdu[3];

  uint16_t currByteIdx = 4;

  if( u16_coap_pdu_len == currByteIdx )
  {
    return ( ret_status );
  }

  currByteIdx += rcz_coap_msg_hdr._token_len;

  if( u16_coap_pdu_len == currByteIdx )
  {
    return ( ret_status );
  }

  rcz_coap_msg_hdr._payloadStart = nullptr;
  rcz_coap_msg_hdr._payloadLen   = 0;

  const auto status = parse_coap_options( &pu8_coap_pdu[currByteIdx], u16_coap_pdu_len - currByteIdx,
      &rcz_coap_msg_hdr._payloadStart, rcz_coap_msg_hdr._options_count );

  if( status != ErrCode::OK )
  {
    return ( ret_status );
  }

  if( rcz_coap_msg_hdr._payloadStart != nullptr )
  {
    rcz_coap_msg_hdr._payloadLen = uint16_t( u16_coap_pdu_len - ( rcz_coap_msg_hdr._payloadStart - pu8_coap_pdu ) );

    if( rcz_coap_msg_hdr._payloadLen > JA_COAP_CONFIG_MAX_PAYLOAD_SIZE )
    {
      return ( ErrCode::PAYLOAD_TOO_LARGE );
    }
  }

  return ( ret_status );
}

ErrCode CoapMsgCodec::decode_coap_msg( _in_ const uint8_t *pu8_coap_pdu, _in_ const uint16_t u16_coap_pdu_len, _out_ CoapMsg &pcz_coap_msg, OptionsCallback &optionsCallback )
{
  pcz_coap_msg.set_type( pu8_coap_pdu[0] >> 4 & 0x03 );
  const uint8_t u8_token_len = pu8_coap_pdu[0] & 0x0F;
  pcz_coap_msg.set_code( pu8_coap_pdu[1] );
  pcz_coap_msg.set_id( static_cast<uint16_t>( pu8_coap_pdu[2] << 8 | pu8_coap_pdu[3] ) );

  uint16_t currByteIdx = 4;

  if( u16_coap_pdu_len == currByteIdx )
  {
    return ( ErrCode::OK );
  }

  pcz_coap_msg.set_token( u8_token_len > 0 ? (int8_t *) &pu8_coap_pdu[currByteIdx] : nullptr,
    u8_token_len );

  currByteIdx += u8_token_len;

  if( u16_coap_pdu_len == currByteIdx )
  {
    return ( ErrCode::OK );
  }

  uint8_t *pu8_payload_start = nullptr;

  auto     status = decode_coap_options( pcz_coap_msg, &pu8_coap_pdu[currByteIdx], u16_coap_pdu_len - currByteIdx,
      &pu8_payload_start, optionsCallback );

  if( status != ErrCode::OK )
  {
    return ( status );
  }

  if( pu8_payload_start != nullptr )
  {
    uint16_t u16_payload_len = uint16_t( u16_coap_pdu_len - ( pu8_payload_start - pu8_coap_pdu ) );

    if( u16_payload_len > JA_COAP_CONFIG_MAX_PAYLOAD_SIZE )
    {
      status = ErrCode::PAYLOAD_TOO_LARGE;
      goto exit_label;
    }

    if( u16_payload_len > 0 )
    {
      const auto pu8_temp_payload = static_cast<uint8_t *>( mnew_g( u16_payload_len ) );
      memcpy( pu8_temp_payload, pu8_payload_start, u16_payload_len );

      pcz_coap_msg.set_payload( pu8_temp_payload, u16_payload_len );
    }
  }

exit_label:

  if( status != ErrCode::OK )
  {
    pcz_coap_msg.get_option_set().clear();
  }

  return ( ErrCode::OK );
}

ErrCode CoapMsgCodec::encode_empty_coap_msg( uint8_t msg_type, uint16_t msg_id, uint8_t *pu8_coap_pdu, uint16_t &ru16_coap_pdu_len )
{
  pu8_coap_pdu[0]  = 0x40;
  pu8_coap_pdu[0] |= ( msg_type & 0x03 ) << 4;

  pu8_coap_pdu[1] = COAP_MSG_CODE_EMPTY;
  pu8_coap_pdu[2] = msg_id >> 8 & 0xFF;
  pu8_coap_pdu[3] = msg_id & 0xFF;

  ru16_coap_pdu_len = 4;

  return ( ErrCode::OK );
}

ErrCode CoapMsgCodec::encode_coap_msg( _in_ CoapMsg &rcz_coap_msg, uint8_t *pu8_coap_pdu, uint16_t &ru16_coap_pdu_len )
{
  auto &token = rcz_coap_msg.get_token();

  ru16_coap_pdu_len = 0;

  pu8_coap_pdu[0]  = 0x40;
  pu8_coap_pdu[0] |= ( rcz_coap_msg.get_type() & 0x03 ) << 4;
  pu8_coap_pdu[0] |= token.get_length() & 0x0F;

  pu8_coap_pdu[1] = rcz_coap_msg.get_code();
  pu8_coap_pdu[2] = rcz_coap_msg.get_id() >> 8 & 0xFF;
  pu8_coap_pdu[3] = rcz_coap_msg.get_id() & 0xFF;

  uint16_t u16_current_byte_idx = 4;

  if( token.get_length() > 0 )
  {
    memcpy( &pu8_coap_pdu[u16_current_byte_idx], token.get_token(), token.get_length() );
  }

  u16_current_byte_idx += token.get_length();

  uint16_t u16_last_option_no = 0;
  auto     cz_options_list    = rcz_coap_msg.get_option_set().get_sorted_options_list();

  for( auto &cz_option : cz_options_list )
  {
    uint16_t u16_cur_delta = cz_option.get_no() - u16_last_option_no;
    u16_last_option_no = cz_option.get_no();

    uint16_t u16_cur_option_len = cz_option.get_len();
    uint16_t u16_temp_byte_idx  = u16_current_byte_idx;

    u16_current_byte_idx++;
    pu8_coap_pdu[u16_temp_byte_idx] = 0;

    if( u16_cur_delta < 13 )
    {
      pu8_coap_pdu[u16_temp_byte_idx] |= (uint8_t) ( u16_cur_delta << 4 );
    }
    else if( u16_cur_delta < 269 )
    {
      pu8_coap_pdu[u16_temp_byte_idx]   |= (uint8_t) ( 13 << 4 );
      pu8_coap_pdu[u16_current_byte_idx] = u16_cur_delta - 13;
      u16_current_byte_idx++;
    }
    else
    {
      pu8_coap_pdu[u16_temp_byte_idx]   |= (uint8_t) ( 14 << 4 );
      pu8_coap_pdu[u16_current_byte_idx] = (uint8_t) ( ( u16_cur_delta - 269 ) >> 8 );
      u16_current_byte_idx++;
      pu8_coap_pdu[u16_current_byte_idx] = (uint8_t) ( ( u16_cur_delta - 269 ) & 0xFF );
      u16_current_byte_idx++;
    }

    /* encode options length */
    if( u16_cur_option_len < 13 )
    {
      pu8_coap_pdu[u16_temp_byte_idx] |= (uint8_t) ( u16_cur_option_len & 0x0F );
    }
    else if( u16_cur_option_len < 269 )
    {
      pu8_coap_pdu[u16_temp_byte_idx]   |= 13;
      pu8_coap_pdu[u16_current_byte_idx] = uint8_t( u16_cur_option_len ) - 13;
      u16_current_byte_idx++;
    }
    else
    {
      pu8_coap_pdu[u16_temp_byte_idx]   |= 14;
      pu8_coap_pdu[u16_current_byte_idx] = (uint8_t) ( ( u16_cur_option_len - 269 ) >> 8 );
      u16_current_byte_idx++;
      pu8_coap_pdu[u16_current_byte_idx] = (uint8_t) ( ( u16_cur_option_len - 269 ) & 0xFF );
      u16_current_byte_idx++;
    }

    if( u16_cur_option_len > 0 )
    {
      memcpy( &pu8_coap_pdu[u16_current_byte_idx], ( (Option &) cz_option ).get_val(), u16_cur_option_len );
      u16_current_byte_idx += u16_cur_option_len;
    }
  }

  if( rcz_coap_msg.has_payload() )
  {
    pu8_coap_pdu[u16_current_byte_idx++] = 0xFF;     // payload marker
    memcpy( &pu8_coap_pdu[u16_current_byte_idx], rcz_coap_msg.get_payload(), rcz_coap_msg.get_payload_len() );

    u16_current_byte_idx += rcz_coap_msg.get_payload_len();
  }

  ru16_coap_pdu_len = u16_current_byte_idx;

  return ( ErrCode::OK );
}

ErrCode decode_coap_options( _in_out_ CoapMsg &pcz_coap_msg, _in_ const uint8_t *pu8_options_start, _in_ const uint16_t u16_options_data_len, _out_ uint8_t **pp_payload_start, OptionsCallback &optionsCallback )
{
  auto     status          = ErrCode::OK;
  uint16_t cur_byte_index  = 0;
  uint16_t last_option_num = 0;

  *pp_payload_start = nullptr;

  if( pu8_options_start[0] == 0xFF )
  {
    *pp_payload_start = const_cast<uint8_t *>( &pu8_options_start[1] );
    status            = ErrCode::OK;
    goto success;
  }

  while( cur_byte_index < u16_options_data_len )
  {
    // check for payload marker, if it is there then mark the start of payload and return
    if( pu8_options_start[cur_byte_index] == 0xFF )
    {
      // at least one byte payload must follow to the payload marker
      if( u16_options_data_len - cur_byte_index < 2 )
      {
        status = ErrCode::MSG_FORMAT_ERROR;
        goto success;
      }

      cur_byte_index++;     // skip payload marker
      *pp_payload_start = const_cast<uint8_t *>( &pu8_options_start[cur_byte_index] );
      status            = ErrCode::OK;
      goto success;
    }

    uint16_t option_delta  = pu8_options_start[cur_byte_index] >> 4;    // initial delta in upper 4 bits
    uint16_t option_length = pu8_options_start[cur_byte_index] & 0x0F;     // initial length in lower 4 bits
    cur_byte_index++;     // skip the initial delta & length byte

    if( option_delta == 13 )
    {
      // An 8-bit unsigned integer follows the initial byte and indicates the Option Delta minus 13.
      option_delta = pu8_options_start[cur_byte_index] + 13;
      cur_byte_index++;     // skip 1 byte big delta
    }
    else if( option_delta == 14 )
    {
      // A 16-bit unsigned integer in network byte order follows the initial byte and indicates the Option Delta minus 269.
      option_delta = ( uint16_t( pu8_options_start[cur_byte_index] ) << 8 | uint16_t( pu8_options_start[cur_byte_index] )
                     ) + 269;
      cur_byte_index += 2;
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
      option_length = pu8_options_start[cur_byte_index] + 13;
      cur_byte_index++;     // skip 1 byte big delta
    }
    else if( option_length == 14 )
    {
      // A 16-bit unsigned integer in network byte order precedes the Option Value and indicates the Option Length minus 269.
      option_length = ( uint16_t( pu8_options_start[cur_byte_index] ) << 8 | uint16_t(
          pu8_options_start[cur_byte_index] ) ) + 269;
      cur_byte_index += 2;
    }
    else if( option_length == 15 )
    {
      // Reserved for future use.
      status = ErrCode::MSG_FORMAT_ERROR;
      goto success;
    }

    // if current option length is greater than max option size or
    // current option length is greater than remaining payload
    if( ( option_length > JA_COAP_CONFIG_MAX_OPTION_SIZE ) || ( u16_options_data_len - cur_byte_index < option_length ) )
    {
      status = ErrCode::MSG_FORMAT_ERROR;
      goto success;
    }

    last_option_num += option_delta;     // encode option delta

    if( optionsCallback && optionsCallback( last_option_num ) )
    {
      pcz_coap_msg.get_option_set().add_option( last_option_num, option_length, const_cast<uint8_t *>( &pu8_options_start[cur_byte_index] ) );
    }

    cur_byte_index += option_length;
  }

success:

  if( status != ErrCode::OK )
  {
    pcz_coap_msg.get_option_set().clear();
  }

  return ( status );
}

ErrCode parse_coap_options( _in_ const uint8_t *pu8_options_data_start, _in_ const uint16_t u16_options_data_len, _out_ uint8_t **pp_payload_start, _out_ uint8_t &ru8_options_count )
{
  uint16_t cur_buf_idx    = 0;
  uint16_t last_option_no = 0;

  ru8_options_count = 0;
  *pp_payload_start = nullptr;

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
      if( u16_options_data_len - cur_buf_idx < 2 )
      {
        return ( ErrCode::MSG_FORMAT_ERROR );
      }

      cur_buf_idx++;     // skip payload marker
      *pp_payload_start = const_cast<uint8_t *>( &pu8_options_data_start[cur_buf_idx] );
      return ( ErrCode::OK );
    }

    uint16_t option_delta  = pu8_options_data_start[cur_buf_idx] >> 4;    // initial delta in upper 4 bits
    uint16_t option_length = pu8_options_data_start[cur_buf_idx] & 0x0F;     // initial length in lower 4 bits
    cur_buf_idx++;     // skip the initial delta & length byte

    if( option_delta == 13 )
    {
      // An 8-bit unsigned integer follows the initial byte and indicates the Option Delta minus 13.
      option_delta = pu8_options_data_start[cur_buf_idx] + 13;
      cur_buf_idx++;     // skip 1 byte big delta
    }
    else if( option_delta == 14 )
    {
      // A 16-bit unsigned integer in network byte order follows the initial byte and indicates the Option Delta minus 269.
      option_delta = ( uint16_t( pu8_options_data_start[cur_buf_idx] ) << 8 | uint16_t(
          pu8_options_data_start[cur_buf_idx] ) ) + 269;
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
      cur_buf_idx++;     // skip 1 byte big delta
    }
    else if( option_length == 14 )
    {
      // A 16-bit unsigned integer in network byte order precedes the Option Value and indicates the Option Length minus 269.
      option_length = ( uint16_t( pu8_options_data_start[cur_buf_idx] ) << 8 | uint16_t(
          pu8_options_data_start[cur_buf_idx] ) ) + 269;
      cur_buf_idx += 2;
    }
    else if( option_length == 15 )
    {
      // Reserved for future use.
      return ( ErrCode::MSG_FORMAT_ERROR );
    }

    // if current option length is greater than max option size or
    // current option length is greater than remaining payload
    if( ( option_length > JA_COAP_CONFIG_MAX_OPTION_SIZE )
      || ( u16_options_data_len - cur_buf_idx < option_length ) )
    {
      return ( ErrCode::MSG_FORMAT_ERROR );
    }

    last_option_no += option_delta;     // encode option delta
    cur_buf_idx    += option_length;
    ru8_options_count++;
  }

  return ( ErrCode::OK );
}
}
}