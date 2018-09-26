#pragma once
#include <string>
#include <map>
#include "OcfResourceConsts.h"
#include <vector>

namespace ja_iot {
namespace ocfstack {
/* if same query repeats then it means any-of matching */
class OcfQueryContainer
{
  public:
    uint8_t get_interface_count();
    uint8_t get_type_count();
    bool    is_interface_matched( const std::vector<std::string> &if_types );
    bool    is_interface_available( OcfResInterfaceType interface_type );
    bool    is_res_type_available( const std::string &res_type );

    bool parse( std::vector<std::string> &query_string_list );

  private:
    std::multimap<std::string, std::string>   _query_map;
};
}
}