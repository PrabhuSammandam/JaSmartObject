#include <cstring>
#include <byte_array.h>
#include <algorithm>
#include <Hash.h>
#include <IMemAllocator.h>

using namespace ja_iot::memory;

namespace ja_iot
{
  namespace base
  {
    ByteArray::ByteArray()
    {
    }

    ByteArray::ByteArray(const uint16_t capacity) : _capacity{capacity}
    {
      if (_capacity > 0)
      {
        _byte_array_ptr = static_cast<uint8_t *>(new uint8_t[capacity]);
      }
    }

    ByteArray::ByteArray(uint8_t* buf, const uint16_t buf_len, const bool is_mem_owned)
    {
      set_array(buf, buf_len, is_mem_owned);
    }

    ByteArray::~ByteArray()
    {
      if (_is_mem_owned == true && _byte_array_ptr != nullptr)
      {
        delete[] _byte_array_ptr;
        _byte_array_ptr = nullptr;
      }

      _length = 0;
      _capacity = 0;
    }

    ByteArray::ByteArray(ByteArray&& other) noexcept : _byte_array_ptr{nullptr}
    {
      // You must do this to pass to move assignment
      *this = std::move(other); // <- Important
    }

    ByteArray& ByteArray::operator =(ByteArray&& other) noexcept
    {
      // Avoid self assignment
      if (&other == this)
      {
        return *this;
      }

      // Get moving
      std::swap(_byte_array_ptr, other._byte_array_ptr); // Moves ptrs
      this->_length = other._length;
      this->_capacity = other._capacity;
      this->_is_mem_owned = other._is_mem_owned;

      return *this;
    }

    uint8_t ByteArray::operator [](const uint16_t index) const
    {
      return _byte_array_ptr[index];
    }

    uint16_t ByteArray::get_len() const
    {
      return _length;
    }

    ByteArray* ByteArray::set_len(const uint16_t len)
    {
      _length = len;
      return this;
    }

    void ByteArray::copy_str(uint8_t* str)
    {
      if (str == nullptr || _length >= _capacity)
      {
        return;
      }

      uint16_t idx = 0;

      while (str[idx] != '\0' && _length < _capacity)
      {
        _byte_array_ptr[_length] = str[idx];
        _length++;
        idx++;
      }
    }

    void ByteArray::copy_n_bytes(uint8_t* src_buf, const uint16_t len)
    {
      if (src_buf == nullptr || _length >= _capacity)
      {
        return;
      }

      for (auto i = 0; i < len && _length < _capacity; i++)
      {
        _byte_array_ptr[_length] = src_buf[i];
        _length++;
      }
    }

    void ByteArray::copy_to(ByteArray* dst, const uint16_t from, const uint16_t len) const
    {
      if (dst == nullptr || from >= _length || len <= 0 || dst->get_array() == nullptr)
      {
        return;
      }

      const auto dst_buf = dst->get_array();
      uint16_t i = 0;

      for (; i < len && i + from < _length && i < dst->_capacity; i++)
      {
        dst_buf[i] = _byte_array_ptr[i + from];
      }

      dst->set_len(i);
      // SetArray((uint8_t*)0,0, false);
    }

    void ByteArray::set_array(uint8_t* buf, const uint16_t buf_len, const bool free_buf)
    {
      if (_is_mem_owned && _byte_array_ptr != nullptr)
      {
        delete[] _byte_array_ptr;
      }

      _byte_array_ptr = buf;
      _length = buf_len;
      _is_mem_owned = free_buf;
      _capacity = _length;
    }

    bool ByteArray::operator ==(const ByteArray& other) const
    {
      if (_length != other._length)
      {
        return false;
      }

      return memcmp(_byte_array_ptr, other._byte_array_ptr, _length) == 0;
    }

    void ByteArray::Assign(uint8_t* buf, uint16_t bufLen)
    {
    }

    uint32_t ByteArray::get_hash_value() const
    {
      if (_byte_array_ptr != nullptr)
      {
        return Hash::get_hash(_byte_array_ptr, _length);
      }

      return 0;
    }
  }
}
