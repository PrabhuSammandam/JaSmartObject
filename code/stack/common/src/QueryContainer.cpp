#include <string>
#include "QueryContainer.h"

namespace ja_iot {
namespace stack {
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

uint8_t QueryContainer::get_interface_count()
{
  uint8_t interface_count = 0;

  if( _query_map.empty() )
  {
    return ( interface_count );
  }

  for( auto &query : _query_map )
  {
    if( query.first == interface_key )
    {
      interface_count++;
    }
  }

  return ( interface_count );
}

/**
 * Returns the no of resource type query count.
 * @return
 */
uint8_t QueryContainer::get_type_count()
{
  uint8_t res_type_count = 0;

  if( _query_map.empty() )
  {
    return ( res_type_count );
  }

  for( auto &query : _query_map )
  {
    if( query.first == res_type_key )
    {
      res_type_count++;
    }
  }

  return ( res_type_count );
}

bool QueryContainer::is_interface_matched( const std::vector<std::string> &if_types )
{
  if( _query_map.empty() || if_types.empty() )
  {
    return ( false );
  }

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

bool QueryContainer::is_interface_available( const ResInterfaceType interface_type )
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

bool QueryContainer::is_res_type_available( const std::string &res_type )
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

std::string& QueryContainer::get_first_if_name()
{
	for (auto &query : _query_map)
	{
		if (query.first == interface_key)
		{
			return query.second;
		}
	}
	return (empty_string);
}

bool QueryContainer::parse( std::vector<std::string> &query_string_list )
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

std::string & QueryContainer::get_interface_string( const ResInterfaceType interface_type )
{
  switch( interface_type )
  {
    case ResInterfaceType::BaseLine: return ( interface_baseline );
    case ResInterfaceType::LinksList: return ( interface_links_list );
    case ResInterfaceType::Batch: return ( interface_batch );
    case ResInterfaceType::ReadOnly: return ( interface_readonly );
    case ResInterfaceType::ReadWrite: return ( interface_readwrite );
    case ResInterfaceType::Actuator: return ( interface_actuator );
    case ResInterfaceType::Sensor: return ( interface_sensor );
    default:;
  }


  return ( empty_string );
}
ResInterfaceType QueryContainer::get_interface_enum( std::string &if_string )
{
  if( if_string.empty() )
  {
    return ( ResInterfaceType::none );
  }
  else if( if_string == interface_baseline )
  {
    return ( ResInterfaceType::BaseLine );
  }
  else if( if_string == interface_links_list )
  {
    return ( ResInterfaceType::LinksList );
  }
  else if( if_string == interface_batch )
  {
    return ( ResInterfaceType::Batch );
  }
  else if( if_string == interface_readonly )
  {
    return ( ResInterfaceType::ReadOnly );
  }
  else if( if_string == interface_readwrite )
  {
    return ( ResInterfaceType::ReadWrite );
  }
  else if( if_string == interface_actuator )
  {
    return ( ResInterfaceType::Actuator );
  }
  else if( if_string == interface_sensor )
  {
    return ( ResInterfaceType::Sensor );
  }

  return ( ResInterfaceType::none );
}
}
}
