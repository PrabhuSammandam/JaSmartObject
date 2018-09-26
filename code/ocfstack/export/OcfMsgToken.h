#pragma once
#include <array>

namespace ja_iot {
namespace ocfstack {
class OcfMsgToken
{
  public:
    OcfMsgToken( const uint8_t token_len, uint8_t *token ) : _token_len{ token_len }
    {
      if( ( token != nullptr ) && ( token_len > 0 ) )
      {
        for( auto i = 0; i < token_len; i++ )
        {
          _token[i] = token[i];
        }
      }
    }

  private:
    uint8_t                  _token_len = 0;
    std::array<uint8_t, 8>   _token;
};
}
}