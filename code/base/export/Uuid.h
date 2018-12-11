/*
 * Uuid.h
 *
 *  Created on: Nov 17, 2018
 *      Author: psammand
 */

#pragma once
#include <stdint.h>
#include <string>

constexpr uint8_t UUID_STRING_LEN = 37;

namespace ja_iot {
namespace base {
class Uuid
{
  public:
    Uuid ();
    Uuid( uint8_t *pu8_buff, uint8_t u8_buf_len );
    Uuid( const Uuid &other );
    Uuid( Uuid &&other );

    Uuid & operator = ( const Uuid &other );
    Uuid & operator = ( Uuid &&other );
    Uuid & operator = ( std::string &uuid_string );
    bool operator   == ( const Uuid &other );
    void operator   >> ( std::string &uuid_string );
    Uuid & operator << ( std::string &uuid_string );

    void        clear();
    bool        is_nil();
    void        set_value( uint8_t *pu8_buff, uint8_t u8_buf_len );
    std::string to_string();
    void        from_string( std::string &uuid_string );

		static bool is_valid_uuid_string(std::string& uuid_string);

  private:
    uint8_t   _uuid[16];
};
}
}