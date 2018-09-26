#pragma once

#include <cstdint>
#include <array>

namespace ja_iot
{
  namespace network
  {
    constexpr uint16_t OPTION_ERROR_CRITICAL_MASK = 0x0100;
    constexpr uint16_t OPTION_ERROR_CRITICAL_UNKNOWN = 0x0101;
    constexpr uint16_t OPTION_ERROR_CRITICAL_REPEAT_MORE = 0x0102;
    constexpr uint16_t OPTION_ERROR_CRITICAL_LEN_RANGE_OUT = 0x0104;
    constexpr uint16_t OPTION_ERROR_UNKNOWN = 0x0001;
    constexpr uint16_t OPTION_ERROR_REPEAT_MORE = 0x0002;
    constexpr uint16_t OPTION_ERROR_LEN_RANGE_OUT = 0x0004;

    class Option
    {
      uint16_t _is_mem_allocated : 1;
      uint16_t _u16_len : 15;
      uint16_t _u16_no{};

      union
      {
        std::array<uint8_t, 6> _au8_buffer;
        uint8_t* _pu8_buffer{};
      } u;

    public:
			Option() : _is_mem_allocated{ false }, _u16_len{ 0 }, _u16_no{ 0 }, u{0}
      {
      }

      ~Option();
      Option(uint16_t no, uint16_t len, uint8_t* val);
      Option(uint16_t no);
      Option(uint16_t no, const std::string& rcz_string_value);
      Option(uint16_t no, uint32_t u32_value);
      Option(const Option& other);
      Option(Option&& other) noexcept;
      Option& operator =(const Option& other);
      Option& operator =(Option&& other) noexcept;

      uint16_t get_no() const { return _u16_no; }
      void set_no(const uint16_t u16_option_no) { _u16_no = u16_option_no; }

      uint16_t get_len() const;

      uint8_t* get_val();
      void set_val(uint16_t u16_option_len, uint8_t* pu8_option_value);

      void set_string_value(const std::string& rcz_string_value);
      std::string get_string_value();
      void set_integer_value(uint32_t u32_value);
      uint32_t get_integer_value();

      bool has_value() const { return get_len() > 0; }

      void print();
    };
  }
}
