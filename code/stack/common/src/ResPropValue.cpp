#include <stdio.h>
#include <inttypes.h>
#include "ResPropValue.h"

namespace ja_iot {
namespace stack {
#define REPRESENTATION_PRETTY_PRINT
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

  /* delete all the old values */
  for( auto it = _props.cbegin(); it != _props.cend(); ++it )
  {
    delete ( *it ).second;
  }

  _props.clear();

  for( auto it = other._props.cbegin(); it != other._props.cend(); ++it )
  {
    auto &key   = ( *it ).first;
    auto &value = ( *it ).second;
    _props[key] = new ResPropValue{ *value };
  }

  return ( *this );
}

/**
 * Move operator implementation
 * @param other
 * @return
 */
ResRepresentation & ResRepresentation::operator = ( ResRepresentation &&other ) noexcept
{
  if( this == &other )
  {
    return ( *this );
  }

  /* delete all the old values */
  for( auto it = _props.cbegin(); it != _props.cend(); ++it )
  {
    delete ( *it ).second;// calling delete for nullptr is ok
  }

  _props = std::move( other._props );
  return ( *this );
}

uint16_t ResRepresentation::no_of_props() const
{
  return ( uint16_t( _props.size() ) );
}

void ResRepresentation::remove_prop( const std::string &property_name )
{
  if( has_prop( property_name ) )
  {
    delete _props[property_name];
    _props.erase( property_name );
  }
}

void ResRepresentation::clear_prop( const std::string &property_name )
{
  auto find_result = _props.find( property_name );

  if( find_result != _props.end() )
  {
    delete _props[property_name];
    _props[property_name] = nullptr;
  }
}

bool ResRepresentation::has_prop( const std::string &property_name )
{
  const auto find_result = _props.find( property_name );

  return ( find_result != _props.end() ? true : false );
}

bool ResRepresentation::is_prop_none( const std::string &property_name )
{
  const auto find_result = _props.find( property_name );

  if( find_result != _props.end() && (find_result->second != nullptr))
  {
	  return ( find_result->second->val_type == ResPropValType::none );
  }

  return ( true );
}

#ifdef REPRESENTATION_PRETTY_PRINT

void print_object( ResRepresentation &object, uint8_t u8_indent );

template<typename T>
void print_array( std::vector<T> &array, uint8_t indent )
{
}

template<>
void print_array( std::vector<bool> &array, uint8_t indent )
{
  for( auto v : array )
  {
    printf( "%*s %s\n", indent, "", v ? "true" : "false" );
  }
}

template<>
void print_array( std::vector<long> &array, uint8_t indent )
{
  for( auto &v : array )
  {
    printf( "%*s %d\n", indent, "", v );
  }
}

template<>
void print_array( std::vector<double> &array, uint8_t indent )
{
  for( auto &v : array )
  {
    printf( "%*s %f\n", indent, "", v );
  }
}

template<>
void print_array( std::vector<std::string> &array, uint8_t indent )
{
  for( auto &v : array )
  {
    printf( "%*s %s\n", indent, "", v.c_str() );
  }
}

template<>
void print_array( std::vector<ResRepresentation> &array, uint8_t indent )
{
  for( auto &v : array )
  {
    print_object( v, indent + 4 );
  }
}


void print_object( ResRepresentation &object, uint8_t u8_indent )
{
  printf( "\n%*s{\n", u8_indent, "" );
  u8_indent += 4;

  for( auto &prop : object.get_props() )
  {
    auto prop_val = prop.second;

    switch( prop_val->get_type() )
    {
      case ResPropValType::boolean:
      {
        printf( "%*s\"%s\" : %s\n", u8_indent, "", prop.first.c_str(), prop_val->get<bool>() == true ? "true" : "false" );
      }
      break;
      case ResPropValType::integer:
      {
        printf( "%*s\"%s\" : %d\n", u8_indent, "", prop.first.c_str(), prop_val->get<long>() );
      }
      break;
      case ResPropValType::number:
      {
        printf( "%*s\"%s\" : %f\n", u8_indent, "", prop.first.c_str(), prop_val->get<double>() );
      }
      break;
      case ResPropValType::string:
      {
        printf( "%*s\"%s\" : %s\n", u8_indent, "", prop.first.c_str(), prop_val->get<std::string>().c_str() );
      }
      break;
      case ResPropValType::object:
      {
        printf( "%*s\"%s\" :", u8_indent, "", prop.first.c_str() );
        print_object( prop_val->get<ResRepresentation>(), u8_indent );
      }
      break;
      case ResPropValType::boolean_array:
      {
        printf( "%*s\"%s\" : [\n", u8_indent, "", prop.first.c_str() );
        auto b_array = prop_val->get_array<std::vector<bool> >();
        print_array( *b_array, u8_indent + 4 );
        printf( "%*s]\n", u8_indent, "" );
      }
      break;
      case ResPropValType::integer_array:
      {
        printf( "%*s\"%s\" : [\n", u8_indent, "", prop.first.c_str() );
        auto b_array = prop_val->get_array<std::vector<long> >();
        print_array( *b_array, u8_indent + 4 );
        printf( "%*s]\n", u8_indent, "" );
      }
      break;
      case ResPropValType::number_array:
      {
        printf( "%*s\"%s\" : [\n", u8_indent, "", prop.first.c_str() );
        auto b_array = prop_val->get_array<std::vector<double> >();
        print_array( *b_array, u8_indent + 4 );
        printf( "%*s]\n", u8_indent, "" );
      }
      break;
      case ResPropValType::string_array:
      {
        printf( "%*s\"%s\" : [\n", u8_indent, "", prop.first.c_str() );
        auto b_array = prop_val->get_array<std::vector<std::string> >();
        print_array( *b_array, u8_indent + 4 );
        printf( "%*s]\n", u8_indent, "" );
      }
      break;
      case ResPropValType::obj_array:
      {
        printf( "%*s\"%s\" : [\n", u8_indent, "", prop.first.c_str() );
        auto b_array = prop_val->get_array<std::vector<ResRepresentation> >();
        print_array( *b_array, u8_indent + 4 );
        printf( "%*s]\n", u8_indent, "" );
      }
      break;
      default:
      {
      }
      break;
    }
  }

  printf( "%*s}\n", u8_indent - 4, "" );
}
#endif

void ResRepresentation::print()
{
#ifdef REPRESENTATION_PRETTY_PRINT
  print_object( *this, 0 );
#endif
}


/*******************************************************************************/
/********************** ResPropValue Implementations****************************/
/*******************************************************************************/
ResPropValue::ResPropValue( const ResPropValType val_type ) : val_type{ val_type }
{
  init( val_type );
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
      this->val.d = other.val.d;
    }
    else if( other.val_type == ResPropValType::string )
    {
      this->val.str = other.val.str;
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
      this->val.d = other.val.d;
    }
    else if( other.val_type == ResPropValType::string )
    {
      this->val.str = std::move( other.val.str );
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
inline ResPropValue::~ResPropValue ()
{
  destroy();
}

void ResPropValue::init( const ResPropValType value_type )
{
  if( value_type == ResPropValType::string )
  {
    new(&this->val.str) std::string{};
  }
  else if( value_type == ResPropValType::object )
  {
    new(&this->val.obj) ResRepresentation{};
  }
  else if( value_type == ResPropValType::boolean_array )
  {
    new(&this->val.b_arr) std::vector<bool> {};
  }
  else if( value_type == ResPropValType::integer_array )
  {
    new(&this->val.l_arr) std::vector<long> {};
  }
  else if( value_type == ResPropValType::number_array )
  {
    new(&this->val.d_arr) std::vector<double> {};
  }
  else if( value_type == ResPropValType::string_array )
  {
    new(&this->val.s_arr) std::vector<std::string> {};
  }
  else if( value_type == ResPropValType::obj_array )
  {
    new(&val.o_arr) std::vector<ResRepresentation> {};
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
    this->val.str.~basic_string ();
  }
  else if( val_type == ResPropValType::object )
  {
    this->val.obj.~ResRepresentation ();
  }
  else if( val_type == ResPropValType::boolean_array )
  {
    this->val.b_arr.~vector ();
  }
  else if( val_type == ResPropValType::integer_array )
  {
    this->val.l_arr.~vector ();
  }
  else if( val_type == ResPropValType::number_array )
  {
    this->val.d_arr.~vector ();
  }
  else if( val_type == ResPropValType::string_array )
  {
    this->val.s_arr.~vector ();
  }
  else if( val_type == ResPropValType::obj_array )
  {
    this->val.o_arr.~vector ();
  }

  val_type = ResPropValType::none;
}
}
}
