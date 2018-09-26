#pragma once

#include <cstdint>

namespace ja_iot {
namespace stack {
class BlockTransferStatus
{
  public:
    BlockTransferStatus ();
    ~BlockTransferStatus ();

    void     clear();
    bool     resize_buffer( uint16_t new_buffer_size );
    bool     add_payload( int8_t *payload_buffer, uint16_t payload_buffer_size );
    int8_t*  get_payload_buffer() { return ( _payload_buffer ); }
    uint16_t get_payload_capacity() { return ( _payload_buffer_size ); }
    uint16_t get_payload_length() { return ( _payload_buffer_index ); }
    uint16_t get_block_size() { return ( _block_size ); }
    void     set_block_size( uint16_t block_size ) { _block_size = block_size; }
    uint32_t move_to_next_block();
    uint32_t get_next_block_no() { return ( _next_block_no ); }
    uint32_t get_current_block_no() { return ( _current_block_no ); }
    void     set_current_block_no( uint32_t block_no ) { _current_block_no = block_no; }
    uint8_t  get_szx();

  private:
    int8_t *   _payload_buffer       = nullptr;
    uint16_t   _payload_buffer_size  = 0;
    uint16_t   _payload_buffer_index = 0;
    uint16_t   _block_size           = 0;
    uint32_t   _current_block_no     = 0;
    uint32_t   _next_block_no        = 0;
};
}
}