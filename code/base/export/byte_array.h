#pragma once

#include <cstdint>
#include <memory>

namespace ja_iot
{
  namespace base
  {
    using ByteArrayBufPtr = std::unique_ptr<uint8_t, void ( *)(uint8_t*)>;

    class ByteArray
    {
      uint8_t* _byte_array_ptr = nullptr;
      uint16_t _length = 0;
      uint16_t _capacity = 0;
      bool _is_mem_owned = true;

    public:

      ByteArray();

      ByteArray(uint16_t size);

      ByteArray(uint8_t* buf, uint16_t buf_len, bool free_buf = false);

      ~ByteArray();

      ByteArray(ByteArray&& other) noexcept;
      ByteArray& operator =(ByteArray&& other) noexcept;

      uint8_t* get_array() const { return _byte_array_ptr; }
      void set_array(uint8_t* buf, uint16_t buf_len, bool free_buf = true);

      void Assign(uint8_t* buf, uint16_t buf_len);

      uint16_t get_len() const;
      ByteArray* set_len(uint16_t len);

      void copy_str(uint8_t* str);
      void copy_n_bytes(uint8_t* src_buf, uint16_t len);
      void copy_to(ByteArray* dst, uint16_t from, uint16_t len) const;

      uint32_t get_hash_value() const;

      bool is_empty() const { return _length == 0; }

      bool operator ==(const ByteArray& other) const;
      uint8_t operator [](uint16_t index) const;
    };
  }
}
