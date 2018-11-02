#include <coap/coap_block_option.h>
#include <cstdint>

namespace ja_iot {
namespace network {
void BlockOption::decode( uint8_t *pu8_option_buffer, const uint8_t u8_option_buffer_len )
{
  _len = u8_option_buffer_len;

  if( u8_option_buffer_len == 0 )
  {
    _szx = 0;
    _m   = false;
    _num = 0;
  }
  else
  {
    const uint8_t u8_lsb_byte = pu8_option_buffer[u8_option_buffer_len - 1];
    _szx = u8_lsb_byte & 0x07;
    _m   = ( u8_lsb_byte >> 3 & 0x01 ) == 0x01;

    uint32_t temp_num = ( u8_lsb_byte & 0xFF ) >> 4;

    for( int i = 1; i < u8_option_buffer_len; i++ )
    {
      temp_num += ( pu8_option_buffer[u8_option_buffer_len - i - 1] & 0xff ) << ( i * 8 - 4 );
    }

    _num = temp_num;
  }
}

void BlockOption::encode( uint8_t *buf, uint8_t &buf_len ) const
{
  const uint8_t  end = get_szx() | ( has_more() ? 1 << 3 : 0 );
  const uint32_t num = get_num();

  if( ( num == 0 ) && !has_more() && ( get_szx() == 0 ) )
  {
    buf_len = 0;
  }
  else if( num < 1 << 4 )
  {
    buf_len = 1;
    buf[0]  = num << 4 | end;
  }
  else if( num < 1 << 12 )
  {
    buf_len = 2;
    buf[0]  = num >> 4;
    buf[1]  = num << 4 | end;
  }
  else
  {
    buf_len = 3;
    buf[0]  = num >> 12;
    buf[1]  = num >> 4;
    buf[2]  = num << 4 | end;
  }
}
}
}
