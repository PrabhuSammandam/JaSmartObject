/*
 * main.cpp
 *
 *  Created on: 05-Nov-2018
 *      Author: prabhu
 */

#include <string.h>
#include <string>
#include <algorithm>
#include "QueryContainer.h"
#include "coap/coap_options_set.h"
#include "BaseResource.h"
#include "StackConsts.h"
#include "base_utils.h"
#include "ip_addr.h"

using namespace std;
using namespace ja_iot::stack;
using namespace ja_iot::network;
using namespace ja_iot::base;

class TestResource : BaseResource
{
  public:

    TestResource () : BaseResource( "/oic/l" )
    {
      init();
    }

    bool check_resource_type( std::vector<std::string> &query_type_list, std::vector<std::string> &res_type_list )
    {
      if( query_type_list.empty() )
      {
        return ( true );
      }

      if( res_type_list.empty() )
      {
        return ( false );
      }

      auto found = false;

      for( auto &rt : query_type_list )
      {
        for( auto &res_rt : res_type_list )
        {
          if( rt == res_rt )
          {
            found = true;
            break;
          }
        }

        if( found )
        {
          break;
        }
      }

      return ( found );
    }

    bool get_intersection( std::vector<std::string> &list1, std::vector<std::string> &list2, std::vector<std::string> &result )
    {
      std::set_intersection( list1.begin(), list1.end(), list2.begin(), list2.end(), back_inserter( result ) );

      return ( !result.empty() );
    }


    uint8_t handle_get( QueryContainer &query_container, Interaction *interaction ) override
    {
      uint8_t stack_status = STACK_STATUS_OK;
      bool    invalid      = false;
      auto    &any_of_map  = query_container.get_any_of_map();

      if(!query_container.check_valid_query(get_properties_list()))
      {
    	  return STACK_STATUS_INVALID_TYPE_QUERY;
      }

      if( !any_of_map.empty() )
      {
        for( auto &q : any_of_map )
        {
          if( q.first == "rt" )
          {
            std::vector<std::string> result;

            if( !get_intersection( q.second, get_types(), result ) )
            {
              stack_status = STACK_STATUS_INVALID_TYPE_QUERY;
              break;
            }
          }

          if( q.first == "if" )
          {
            std::vector<std::string> result;

            if( !get_intersection( q.second, get_interfaces(), result ) )
            {
              stack_status = STACK_STATUS_INVALID_INTERFACE_QUERY;
              break;
            }
          }
        }
      }

      if( stack_status != STACK_STATUS_OK )
      {
        return ( stack_status );
      }

      auto &all_of_map = query_container.get_all_of_map();

      if( !all_of_map.empty() )
      {
        for( auto &q : all_of_map )
        {
          if( q.first == "rt" )
          {
            auto found = find_in_list( get_types(), q.second );

            if( !found )
            {
              stack_status = STACK_STATUS_INVALID_TYPE_QUERY;
              invalid      = true;
            }
          }
          else if( q.first == "if" )
          {
            auto found = find_in_list( get_interfaces(), q.second );

            if( !found )
            {
              stack_status = STACK_STATUS_INVALID_INTERFACE_QUERY;
              invalid      = true;
            }
          }

          if( invalid )
          {
            break;
          }
        }
      }

      return ( STACK_STATUS_OK );
    }

    std::vector<std::string>& get_properties_list() { return ( _properties ); }

    void init()
    {
      add_type( "oic.wk.d" );
      add_interface( "oic.if.r" );
      add_interface( "oic.if.baseline" );
      set_property( OCF_RESOURCE_PROP_DISCOVERABLE );

      _properties.push_back( "if" );
      _properties.push_back( "rt" );
      _properties.push_back( "n" );
      _properties.push_back( "di" );
      _properties.push_back( "dmv" );
      _properties.push_back( "icv" );
    }

  private:
    std::vector<std::string>   _properties{};
};

int main()
{
  CoapOptionsSet option_set{};

  // string query = "if=ifb&rt=c&rt=b&rt=a&name=me&if=ifa";
  // string query = "if=ifa";

  string query = "rt=oic.wk.l&if=oic.if.baseline&if=oic.if.a";

  option_set.set_uri_query_string( query );

  auto           query_list = option_set.get_uri_querys_list();

  QueryContainer container{};

  container.parse( query_list );

//  TestResource test_res{};
//  test_res.handle_get( container, nullptr );

  IpAddress ip{IpAddrFamily::IPv6};
  uint8_t buffer[60];

  IpAddress::from_string("fe80:0000:0000:0000:0eed:8b3c:13d7:cea0", IpAddrFamily::IPv6, ip);

  ip.to_string(buffer, 60);
  printf("Ip Addr [%s]\n", buffer);

  IpAddress::from_string("0000:0000:0000:0000:0eed:8b3c:13d7:cea0", IpAddrFamily::IPv6, ip);

  ip.to_string(buffer, 60);
  printf("Ip Addr [%s]\n", buffer);

  IpAddress::from_string("ff02:0000:0000:0000:0000:0000:0000:158", IpAddrFamily::IPv6, ip);

  ip.to_string(buffer, 60);
  printf("Ip Addr [%s]\n", buffer);

  return ( 0 );
}
