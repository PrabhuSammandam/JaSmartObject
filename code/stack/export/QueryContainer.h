#pragma once
#include <string>
#include <map>
#include <vector>
#include "ResourceConsts.h"

namespace ja_iot {
namespace stack {
/* if same query repeats then it means any-of matching */
class QueryContainer
{
  public:
    uint8_t    get_interface_count();
    uint8_t    get_type_count();
    bool       is_interface_matched( const std::vector<std::string> &if_types );
    bool       is_interface_available( ResInterfaceType interface_type );
    bool       is_res_type_available( const std::string &res_type );
    std::string& get_first_if_name();

    bool                    parse( std::vector<std::string> &query_string_list );
    auto                    & get_query_map() { return ( _query_map ); }
    auto &get_any_of_map(){return _any_of_map;}
    auto &get_all_of_map(){return _all_of_map;}
    static std::string      & get_interface_string( const ResInterfaceType interface_type );
    static ResInterfaceType get_interface_enum( std::string &if_string );

  private:
    std::multimap<std::string, std::string>   _query_map;
    std::multimap<std::string, std::vector<std::string>>   _any_of_map;
    std::multimap<std::string, std::string>   _all_of_map;
};
}
}
