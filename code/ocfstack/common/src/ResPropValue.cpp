#include <iostream>
#include "ResPropValue.h"

namespace ja_iot {
namespace ocfstack {
ResRepresentation::~ResRepresentation ()
{
  for( auto it = _props.cbegin(); it != _props.cend(); ++it )
  {
    delete ( *it ).second;
  }

  _props.clear();
}
ResRepresentation::ResRepresentation( const ResRepresentation &other ) : _props{}
{
  *this = other;
}
ResRepresentation::ResRepresentation( ResRepresentation &&other ) noexcept
{
  *this = std::move( other );
}

ResRepresentation & ResRepresentation::operator = ( const ResRepresentation &other )
{
  if( this == &other )
  {
    return ( *this );
  }

  for( auto it = other._props.cbegin(); it != other._props.cend(); ++it )
  {
    _props[( *it ).first] = new ResPropValue( *( ( *it ).second ) );
  }

  return ( *this );
}

ResRepresentation & ResRepresentation::operator = ( ResRepresentation &&other ) noexcept
{
  if( this == &other )
  {
    return ( *this );
  }

  _props = std::move( other._props );
  return ( *this );
}

uint16_t ResRepresentation::no_of_props() const
{
  return ( uint16_t( _props.size() ) );
}

void ResRepresentation::remove_prop( const std::string &str )
{
  if( has_prop( str ) )
  {
    delete _props[str];
    _props.erase( str );
  }
}

void ResRepresentation::clear_prop( const std::string &str )
{
  auto find_result = _props.find( str );

  if( find_result != _props.end() )
  {
    find_result->second->destroy();
  }
}

void ResRepresentation::print()
{
  for( auto &prop : _props )
  {
    auto prop_val = prop.second;

    if( prop_val->val_type == ResPropValType::boolean_array )
    {
      std::cout << "\"" << prop.first << "\" : [ ";
      const auto b_array = prop_val->get_array<std::vector<bool> >();

      for( auto it = b_array->begin(); it != b_array->end(); ++it )
      {
        std::cout << ( ( ( *it ) == true ) ? "true" : "false" ) << ", ";
      }

      std::cout << " ]" << std::endl;
    }

    if( prop_val->val_type == ResPropValType::string_array )
    {
      std::cout << "\"" << prop.first << "\" : [ ";
      const auto s_array = prop_val->get_array<std::vector<std::string> >();

      for( auto &val : *s_array )
      {
        std::cout << val << ", ";
      }

      std::cout << " ]" << std::endl;
    }

#if 0

    || prop_val->val_type == ResPropValType::integer_array
    || prop_val->val_type == ResPropValType::number_array
    || prop_val->val_type == ResPropValType::string_array
    || prop_val->val_type == ResPropValType::obj_array
#endif
  }
}

bool ResRepresentation::has_prop( const std::string &name )
{
  const auto find_result = _props.find( name );

  return ( find_result != _props.end() ? true : false );
}

bool ResRepresentation::is_prop_none( const std::string &str )
{
  const auto find_result = _props.find( str );

  if( find_result != _props.end() )
  {
    return ( find_result->second->val_type == ResPropValType::none );
  }

  return ( true );
}
ResPropValue::ResPropValue( const ResPropValType val_type ) : val_type{ val_type }
{
}
ResPropValue::ResPropValue( const ResPropValue &other ) : val_type{ other.val_type }
{
  init( val_type );
  *this = other;
}
ResPropValue::ResPropValue( ResPropValue &&other ) noexcept : val_type{ other.val_type }
{
  init( val_type );
  *this = std::move( other );
}

ResPropValue & ResPropValue::operator = ( const ResPropValue &other )
{
  if( &other != this )
  {
    destroy();

    this->val_type = other.val_type;
    init( val_type );

    if( other.val_type == ResPropValType::number )
    {
      val.d = other.val.d;
    }
    else if( other.val_type == ResPropValType::string )
    {
      val.str = other.val.str;
    }
    else if( other.val_type == ResPropValType::object )
    {
      this->val.obj = other.val.obj;
    }
    else if( other.val_type == ResPropValType::boolean_array )
    {
      this->val.b_arr = other.val.b_arr;
    }
    else if( other.val_type == ResPropValType::integer_array )
    {
      this->val.l_arr = other.val.l_arr;
    }
    else if( other.val_type == ResPropValType::number_array )
    {
      this->val.d_arr = other.val.d_arr;
    }
    else if( other.val_type == ResPropValType::string_array )
    {
      this->val.s_arr = other.val.s_arr;
    }
    else if( other.val_type == ResPropValType::obj_array )
    {
      this->val.o_arr = other.val.o_arr;
    }
    else
    {
      this->val.l = other.val.l;
    }
  }

  return ( *this );
}

ResPropValue & ResPropValue::operator = ( ResPropValue &&other ) noexcept
{
  if( &other != this )
  {
    destroy();

    this->val_type = other.val_type;
    init( val_type );

    if( other.val_type == ResPropValType::number )
    {
      val.d = other.val.d;
    }
    else if( other.val_type == ResPropValType::string )
    {
      val.str = std::move( other.val.str );
    }
    else if( other.val_type == ResPropValType::object )
    {
      this->val.obj = std::move( other.val.obj );
    }
    else if( other.val_type == ResPropValType::boolean_array )
    {
      this->val.b_arr = std::move( other.val.b_arr );
    }
    else if( other.val_type == ResPropValType::integer_array )
    {
      this->val.l_arr = std::move( other.val.l_arr );
    }
    else if( other.val_type == ResPropValType::number_array )
    {
      this->val.d_arr = std::move( other.val.d_arr );
    }
    else if( other.val_type == ResPropValType::string_array )
    {
      this->val.s_arr = std::move( other.val.s_arr );
    }
    else if( other.val_type == ResPropValType::obj_array )
    {
      this->val.o_arr = std::move( other.val.o_arr );
    }
    else
    {
      this->val.l = other.val.l;
    }

    other.val.l    = 0;
    other.val_type = ResPropValType::none;
  }

  return ( *this );
}

void ResPropValue::init( const ResPropValType value_type )
{
  if( value_type == ResPropValType::string )
  {
    new(&val.str) std::string();
  }
  else if( value_type == ResPropValType::object )
  {
    new(&this->val.obj) ResRepresentation();
  }
  else if( value_type == ResPropValType::boolean_array )
  {
    new(&this->val.b_arr) std::vector<bool>();
  }
  else if( value_type == ResPropValType::integer_array )
  {
    new(&this->val.l_arr) std::vector<int>();
  }
  else if( value_type == ResPropValType::number_array )
  {
    new(&this->val.d_arr) std::vector<double>();
  }
  else if( value_type == ResPropValType::string_array )
  {
    new(&this->val.s_arr) std::vector<std::string>();
  }
  else if( value_type == ResPropValType::obj_array )
  {
    new(&val.o_arr) std::vector<ResPropValue *>();
  }
  else
  {
    val.l = 0;
  }
}

void ResPropValue::destroy()
{
  if( val_type == ResPropValType::string )
  {
    val.str.~basic_string ();
  }

  if( val_type == ResPropValType::object )
  {
    val.obj.~ResRepresentation ();
  }
  else if( val_type == ResPropValType::boolean_array )
  {
    this->val.b_arr.clear();
  }
  else if( val_type == ResPropValType::integer_array )
  {
    this->val.l_arr.clear();
  }
  else if( val_type == ResPropValType::number_array )
  {
    this->val.d_arr.clear();
  }
  else if( val_type == ResPropValType::string_array )
  {
    this->val.s_arr.clear();
  }
  else if( val_type == ResPropValType::obj_array )
  {
    val.o_arr.clear();
  }

  val_type = ResPropValType::none;
}
}
}