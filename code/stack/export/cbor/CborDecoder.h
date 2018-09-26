#pragma once

#include "cbor/ICborDecoderListener.h"
#include "cbor/CborDecoderBuffer.h"

namespace ja_iot {
namespace stack {
class CborDecoder
{
  public:
		CborDecoder(CborDecoderBuffer *src_buffer) : _listener{ nullptr }, _src_buffer{ src_buffer } {}
		CborDecoder(CborDecoderBuffer *src_buffer, ICborDecoderListener *decoder_listener) : _listener{ decoder_listener }, _src_buffer{ src_buffer } {}
		~CborDecoder() {}
		void decode()
		{
			int8_t length_lookup[8] = { 1, 2, 4, 8, 0, 0, 0, 0 };  // 24, 25, 26, 27, 28, 29, 30, 31
			bool to_continue = true;

			while (to_continue)
			{
				if (_src_buffer->has_bytes(1))
				{
					uint8_t  initial_byte = _src_buffer->get<uint8_t>();
					uint8_t  major_type = initial_byte >> 5;
					uint8_t  additional_info = initial_byte & 0x1F;
					uint64_t value = additional_info;
					uint8_t  length = 0;

					if (additional_info >= 24)
					{
						length = length_lookup[additional_info - 24];

						if (length == 0)
						{
							continue;
						}

						if (!_src_buffer->has_bytes(length))
						{
							to_continue = false; continue;
						}

						switch (length)
						{
						case 1: value = _src_buffer->get<uint8_t>(); break;
						case 2: value = _src_buffer->get<uint16_t>(); break;
						case 4: value = _src_buffer->get<uint32_t>(); break;
						case 8: value = _src_buffer->get<uint64_t>(); break;
						}
					}

					if ((major_type == 2) || (major_type == 3))
					{
						if (!_src_buffer->has_bytes(static_cast<int32_t>(value)))
						{
							to_continue = false; continue;
						}
					}

					call_listener(major_type, length, value);
				}
				else
				{
					break;
				}
			}
		}
    void set_listener( ICborDecoderListener *decoder_listener ) { _listener = decoder_listener; }

  private:
		uint8_t call_listener(uint8_t major_type, uint8_t length, uint64_t value)
		{
			if (_listener == nullptr)
			{
				return 0;
			}

			switch (major_type)
			{
			case 0: /* POSITIVE INTEGER*/
			{
				switch (length)
				{
				case 0:
				case 1: _listener->on_uint8(static_cast<uint8_t>(value)); break;
				case 2: _listener->on_uint16(static_cast<uint16_t>(value)); break;
				case 4: _listener->on_uint32(static_cast<uint32_t>(value)); break;
				case 8: _listener->on_uint64(value); break;
				}
			}
			break;
			case 1:                         /* NEGATIVE INTEGER*/
			{
				switch (length)
				{
				case 0:
				case 1: _listener->on_int8(-1 - static_cast<int8_t>(value)); break;
				case 2: _listener->on_int16(-1 - static_cast<int16_t>(value)); break;
				case 4: _listener->on_int32(-1 - static_cast<int32_t>(value)); break;
				case 8: _listener->on_int64(-1 - value); break;
				}
			}
			break;
			case 2:             // BYTE_STRING
			{
				uint8_t *data = new uint8_t[static_cast<uint32_t>(value)];
				_src_buffer->get_bytes(data, static_cast<int32_t>(value));
				_listener->on_bytes(data, static_cast<int32_t>(value));
			}
			break;
			case 3:             // TEXT_STRING
			{
				uint8_t *data = new uint8_t[static_cast<uint32_t>(value)];
				_src_buffer->get_bytes(data, static_cast<int32_t>(value));
				std::string    str((const char *)data, (size_t)value);
				_listener->on_string(str);
			}
			break;
			case 4: _listener->on_array(static_cast<int32_t>(value)); break;
			case 5: _listener->on_map(static_cast<int32_t>(value)); break;
			case 6: _listener->on_tag(static_cast<int32_t>(value)); break;
			case 7:
			{
				switch (length)
				{
				case 0:
				{
					if (value < 20)
					{
						_listener->on_special(static_cast<uint32_t>(value));
					}
					else
					{
						switch (value)
						{
						case 20: _listener->on_bool(false); break;
						case 21: _listener->on_bool(true); break;
						case 22: _listener->on_null(); break;
						case 23: _listener->on_undefined(); break;
						}
					}
				}
				break;
				case 1: _listener->on_special(static_cast<uint32_t>(value)); break;
				case 2:
				{
					int    half = static_cast<int>(value);
					int    exp = (half >> 10) & 0x1f;
					int    mant = half & 0x3ff;
					double val;

					if (exp == 0)
					{
						val = ldexp(mant, -24);
					}
					else if (exp != 31)
					{
						val = ldexp(mant + 1024, exp - 25);
					}
					else
					{
						val = mant == 0 ? INFINITY : NAN;
					}

					_listener->on_float16(static_cast<float>(half & 0x8000 ? -val : val));
				}
				break;
				case 4:
				{
					union _cbor_float_helper {
						float    as_float;
						uint32_t as_uint;
					};

					union _cbor_float_helper helper;
					helper.as_uint = static_cast<uint32_t>(value);

					_listener->on_float32(helper.as_float);
				}
				break;
				case 8:
				{
					union _cbor_double_helper {
						double   as_double;
						uint64_t as_uint;
					};
					union _cbor_double_helper helper;
					helper.as_uint = value;
					_listener->on_float64(helper.as_double);
				}
				break;
				default:
					break;
				}
			}
			break;
			default: break;
			}


			return (0);
		}
    ICborDecoderListener * _listener;
    CborDecoderBuffer *    _src_buffer;
};
}
}