#pragma once

#include <cstdint>
#include "end_point.h"
#include "coap/coap_msg.h"
#include "Hash.h"

namespace ja_iot {
namespace network {
class CoapMsg;
}
}

class KeyMid
{
  public:
    KeyMid( const uint16_t mid, const ja_iot::network::Endpoint &endpoint ) : _mid{ mid }, endpoint{ endpoint }
    {
    }
    KeyMid( ja_iot::network::CoapMsg *coap_msg )
    {
      _mid     = coap_msg->get_id();
      endpoint = coap_msg->get_endpoint();
    }

    uint16_t _mid{};
    ja_iot::network::Endpoint   endpoint;
};

class KeyToken
{
  public:
    KeyToken( uint8_t *token, uint8_t token_len, const ja_iot::network::Endpoint &endpoint );
    KeyToken( ja_iot::network::CoapMsg *coap_msg )
    {
      _token   = coap_msg->get_token();
      endpoint = coap_msg->get_endpoint();
    }

    ja_iot::network::CoapMsgToken   _token;
    ja_iot::network::Endpoint       endpoint;
};

// custom specialization of std::hash can be injected in namespace std
namespace std {
template<> struct hash<KeyMid>
{
  typedef KeyMid argument_type;
  typedef size_t result_type;
  result_type operator () ( argument_type const &s ) const noexcept
  {
    auto       key_mid       = const_cast<argument_type &>( s );
    const auto endpoint_hash = hash<ja_iot::network::Endpoint> {} ( key_mid.endpoint );

    return ( ja_iot::base::Hash::get_hash( &key_mid._mid, 2, endpoint_hash ) );
  }
};
}

// custom specialization of std::hash can be injected in namespace std
namespace std {
template<> struct hash<KeyToken>
{
  typedef KeyToken argument_type;
  typedef size_t result_type;
  result_type operator () ( argument_type const &s ) const noexcept
  {
    auto       key_token     = const_cast<argument_type &>( s );
    const auto endpoint_hash = hash<ja_iot::network::Endpoint> {} ( key_token.endpoint );

    return ( ja_iot::base::Hash::get_hash( key_token._token.get_token(), key_token._token.get_length(), endpoint_hash ) );
  }
};
}

inline bool operator == ( const KeyMid &mid1, const KeyMid &mid2 )
{
  auto &key_mid1 = const_cast<KeyMid &>( mid1 );
  auto &key_mid2 = const_cast<KeyMid &>( mid2 );

  if( ( key_mid1._mid == key_mid2._mid )
    && ( key_mid1.endpoint.get_addr() == key_mid2.endpoint.get_addr() )
    && ( key_mid1.endpoint.get_port() == key_mid2.endpoint.get_port() ) )
  {
    return ( true );
  }

  return ( false );
}

inline bool operator == ( const KeyToken &token1, const KeyToken &token2 )
{
  auto &key_token1 = const_cast<KeyToken &>( token1 );
  auto &key_token2 = const_cast<KeyToken &>( token2 );

  if( ( key_token1._token == key_token2._token )
    && ( key_token1.endpoint.get_addr() == key_token2.endpoint.get_addr() )
    && ( key_token1.endpoint.get_port() == key_token2.endpoint.get_port() ) )
  {
    return ( true );
  }

  return ( false );
}