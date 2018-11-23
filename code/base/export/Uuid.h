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

namespace ja_iot{
namespace base {
class Uuid
{
  public:
    Uuid ();
    Uuid( const Uuid &other );
    Uuid( Uuid &&other );

    Uuid & operator = ( const Uuid &other );
    Uuid & operator = ( Uuid &&other );

    Uuid & operator = ( std::string &uuid_string );

    bool operator == ( const Uuid &other );

    void operator   >> ( std::string &uuid_string );
    Uuid & operator << ( std::string &uuid_string );

    void clear();
    bool is_nil();
    std::string to_string();
    void        from_string( std::string &uuid_string );

  private:
    uint8_t   _uuid[16];
};
}
}
