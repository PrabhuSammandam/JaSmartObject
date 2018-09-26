#include <string>
#include "OcfQueryContainer.h"

namespace ja_iot {
namespace ocfstack {
std::string       empty_string         = "";
const std::string interface_key        = "if";
const std::string res_type_key         = "rt";
std::string       interface_baseline   = "oic.if.baseline";
std::string       interface_links_list = "oic.if.ll";
std::string       interface_batch      = "oic.if.b";
std::string       interface_readonly   = "oic.if.r";
std::string       interface_readwrite  = "oic.if.rw";
std::string       interface_actuator   = "oic.if.a";
std::string       interface_sensor     = "oic.if.s";

std::string& get_interface_string( const OcfResInterfaceType interface_type );

uint8_t OcfQueryContainer::get_interface_count()
{
  uint8_t interface_count = 0;

  for( auto &query : _query_map )
  {
    if( query.first == interface_key )
    {
      interface_count++;
    }
  }

  return ( interface_count );
}

uint8_t OcfQueryContainer::get_type_count()
{
  uint8_t res_type_count = 0;

  for( auto &query : _query_map )
  {
    if( query.first == res_type_key )
    {
      res_type_count++;
    }
  }

  return ( res_type_count );
}

bool OcfQueryContainer::is_interface_matched( const std::vector<std::string> &if_types )
{
  for( auto &query : _query_map )
  {
    if( query.first == interface_key )
    {
      for( auto &required_interface : if_types )
      {
        if( query.second == required_interface )
        {
          return ( true );
        }
      }
    }
  }

  return ( false );
}

bool OcfQueryContainer::is_interface_available( const OcfResInterfaceType interface_type )
{
  auto &interface_string = get_interface_string( interface_type );

  for( auto &query : _query_map )
  {
    if( ( query.first == interface_key ) && ( query.second == interface_string ) )
    {
      return ( true );
    }
  }

  return ( false );
}

bool OcfQueryContainer::is_res_type_available( const std::string &res_type )
{
  for( auto &query : _query_map )
  {
    if( ( query.first == res_type_key ) && ( query.second == res_type ) )
    {
      return ( true );
    }
  }

  return ( false );
}

bool OcfQueryContainer::parse( std::vector<std::string> &query_string_list )
{
  for( auto &query : query_string_list )
  {
    const auto start_eq = query.find_first_of( '=' );

    if( ( start_eq == std::string::npos ) || ( start_eq == 0 ) || ( start_eq == ( query.length() - 1 ) ) )
    {
      continue;
    }

    _query_map.emplace( query.substr( 0, start_eq ), query.substr( start_eq + 1 ) );
  }

  return ( true );
}

std::string& get_interface_string( const OcfResInterfaceType interface_type )
{
  switch( interface_type )
  {
    case OcfResInterfaceType::BaseLine: return ( interface_baseline );
    case OcfResInterfaceType::LinksList: return ( interface_links_list );
    case OcfResInterfaceType::Batch: return ( interface_batch );
    case OcfResInterfaceType::ReadOnly: return ( interface_readonly );
    case OcfResInterfaceType::ReadWrite: return ( interface_readwrite );
    case OcfResInterfaceType::Actuator: return ( interface_actuator );
    case OcfResInterfaceType::Sensor: return ( interface_sensor );
    default:;
  }

  return ( empty_string );
}
}
}