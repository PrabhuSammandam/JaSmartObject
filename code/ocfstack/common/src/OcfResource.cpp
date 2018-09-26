#include "OcfResource.h"

namespace ja_iot {
namespace ocfstack {
OcfResource::OcfResource( std::string uri ) : _uri{ uri }
{
}

uint8_t OcfResource::handle_request( OcfExchange &exchange )
{
  return ( 0 );
}

std::vector<std::string> & OcfResource::get_types()
{
  return ( _types );
}

void OcfResource::add_type( const std::string &types )
{
  _types.push_back( types );
}

std::vector<std::string> & OcfResource::get_interfaces()
{
  return ( _interfaces );
}

void OcfResource::add_interface( const std::string &interfaces )
{
  _interfaces.push_back( interfaces );
}

uint8_t OcfResource::get_property()
{
  return ( _property );
}

void OcfResource::set_property( const uint8_t property )
{
  _property = property;
}

bool OcfResource::is_collection()
{
  return ( false );
}

std::string & OcfResource::get_uri()
{
  return ( _uri );
}

void OcfResource::set_uri( const std::string &uri )
{
  _uri = uri;
}

std::string & OcfResource::get_unique_id()
{
  return ( _unique_id );
}

void OcfResource::set_unique_id( const std::string &unique_id )
{
  _unique_id = unique_id;
}

uint8_t OcfResource::get_representation( OcfResInterfaceType interface_type, ResRepresentation &representation )
{
  return ( 1 );
}

uint8_t OcfResource::get_discovery_representation( ResRepresentation &representation )
{
  return ( 1 );
}
}
}