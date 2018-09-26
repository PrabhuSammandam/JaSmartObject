#include <coap/coap_block_option.h>
#include <common/inc/BlockTransferStatus.h>
#include <string.h>

namespace ja_iot {
namespace stack {
BlockTransferStatus::BlockTransferStatus ()
{
}
BlockTransferStatus::~BlockTransferStatus ()
{
}

void BlockTransferStatus::clear()
{
  if( _payload_buffer != nullptr )
  {
    delete[] _payload_buffer;
  }

  _payload_buffer       = nullptr;
  _payload_buffer_size  = 0;
  _payload_buffer_index = 0;
  _block_size           = 0;
  _current_block_no     = 0;
  _next_block_no        = 0;
}

bool BlockTransferStatus::resize_buffer( uint16_t new_buffer_size )
{
  if( _payload_buffer != nullptr )
  {
    delete[] _payload_buffer;
    _payload_buffer_size  = 0;
    _payload_buffer_index = 0;
  }

  _payload_buffer = new int8_t[new_buffer_size];

  if( _payload_buffer == nullptr )
  {
    return ( false );
  }

  _payload_buffer_size  = new_buffer_size;
  _payload_buffer_index = 0;

  return ( true );
}

bool BlockTransferStatus::add_payload( int8_t *payload, uint16_t payload_size )
{
  memcpy( &_payload_buffer[_payload_buffer_index], payload, payload_size );
  _payload_buffer_index += payload_size;

  return ( true );
}

uint32_t BlockTransferStatus::move_to_next_block()
{
  _current_block_no = _next_block_no;
  _next_block_no++;

  return ( _next_block_no );
}

uint8_t BlockTransferStatus::get_szx()
{
  return ( ja_iot::network::BlockOption::size_to_szx( _block_size ) );
}
}
}