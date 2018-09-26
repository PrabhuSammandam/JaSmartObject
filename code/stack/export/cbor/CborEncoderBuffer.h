#pragma once
#include <cstdint>
#include <string.h>

namespace ja_iot {
namespace stack {
class CborEncoderBuffer
{
  private:
    uint8_t *  _buffer;
    uint32_t   _capacity;
    uint32_t   _offset;
		bool _is_owned = true;
  public:
		CborEncoderBuffer(uint32_t capacity)
		{
			this->_capacity = capacity;
			this->_buffer = new uint8_t[capacity];
			this->_offset = 0;
		}
		CborEncoderBuffer(uint8_t *  buffer, uint32_t   capacity)
		{
			this->_is_owned = false;
			this->_capacity = capacity;
			this->_buffer = buffer;
			this->_offset = 0;
		}

		virtual ~CborEncoderBuffer()
		{
			if (this->_is_owned == true)
			{
				delete _buffer;
			}
		}

    unsigned char* data() { return ( _buffer ); }
    unsigned int   size() { return ( _offset ); }

		void put_byte(uint8_t value)
		{
			if (_offset < _capacity)
			{
				_buffer[_offset++] = value;
			}
		}

		void put_bytes(const uint8_t *data, int size)
		{
			if (_offset + size - 1 < _capacity)
			{
				memcpy(_buffer + _offset, data, size);
				_offset += size;
			}
		}
};
}
}
