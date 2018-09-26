#include <cstring>
#include "coap/coap_msg.h"

namespace ja_iot {
namespace network {
CoapMsg & CoapMsg::operator = ( const CoapMsg &other )
{
  if( &other != this )
  {
    _version = other._version;
    _type    = other._type;
    _code    = other._code;
    _msg_id  = other._msg_id;
    _token   = other._token;

    if( other._u16_payload_len > 0 )
    {
      this->_pu8_payload = new uint8_t[other._u16_payload_len];
      std::memcpy( &this->_pu8_payload[0], &other._pu8_payload[0], other._u16_payload_len );

      this->_u16_payload_len = other._u16_payload_len;
    }

    this->_coap_options_set = other._coap_options_set;
    this->_endpoint         = other._endpoint;
  }

  return ( *this );
}

CoapMsg & CoapMsg::operator = ( CoapMsg &&other ) noexcept
{
  if( &other != this )
  {
    _version = other._version;
    _type    = other._type;
    _code    = other._code;
    _msg_id  = other._msg_id;
    _token   = other._token;

    _pu8_payload     = other._pu8_payload;
    _u16_payload_len = other._u16_payload_len;

    this->_coap_options_set = std::move( other._coap_options_set );

    other._u16_payload_len = 0;
    other._pu8_payload     = nullptr;
  }

  return ( *this );
}

CoapMsg* CoapMsg::create_msg()
{
	auto new_msg = new CoapMsg{};

	new_msg->set_type(_type);
	new_msg->set_code(_code);
	new_msg->set_token(_token);
	new_msg->set_endpoint(_endpoint);

	return new_msg;
}

CoapMsg* CoapMsg::create_msg(uint8_t code)
{
	auto new_msg = create_msg();
	new_msg->set_code(code);
	return new_msg;
}

CoapMsg* CoapMsg::create_msg(uint8_t type, uint8_t code)
{
	auto new_msg = create_msg();
	new_msg->set_type(type);
	new_msg->set_code(code);
	return new_msg;
}

CoapMsg* CoapMsg::create_msg_with_options()
{
	auto new_msg = create_msg();
	new_msg->get_option_set() = _coap_options_set;

	return new_msg;
}

CoapMsg* CoapMsg::create_msg_with_options(uint8_t code)
{
	auto new_msg = create_msg_with_options();
	new_msg->set_code(code);
	return new_msg;
}

CoapMsg* CoapMsg::create_msg_with_options(uint8_t type, uint8_t code)
{
	auto new_msg = create_msg_with_options();
	new_msg->set_code(code);
	new_msg->set_type(type);
	return new_msg;
}

void CoapMsg::copy_with_options(CoapMsg* src_msg)
{
	this->set_type(src_msg->get_type());
	this->set_code(src_msg->get_code());
	this->set_token(src_msg->get_token());
	this->set_endpoint(src_msg->get_endpoint());
	this->_coap_options_set = src_msg->get_option_set();
}

bool CoapMsg::has_block_option()
{
  return ( _coap_options_set.has_block1() || _coap_options_set.has_block2() );
}

bool operator == (const CoapMsgToken &tkn1, const CoapMsgToken &tkn2)
{
	if ((tkn1.get_length() == tkn2.get_length())
		&& (memcmp(tkn1.get_token(), tkn2.get_token(), tkn1.get_length()) == 0))
	{
		return (true);
	}

	return (false);
}

}
}