#include <stdio.h>
#include <inttypes.h>
#include "ResPropValue.h"

namespace ja_iot {
namespace stack {
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
    _props[key] = new ResPropValue( *value );
  }

  return ( *this );
}

ResRepresentation & ResRepresentation::operator = ( ResRepresentation &&other ) noexcept
{
  if( this == &other )
  {
    return ( *this );
  }

	/* delete all the old values */
	for (auto it = _props.cbegin(); it != _props.cend(); ++it)
	{
		delete (*it).second;
	}

  _props = std::move( other._props );
  return ( *this );
}

void * ResRepresentation::operator new(size_t size)
{
	return umm_malloc(size);
}

void ResRepresentation::operator delete(void * p)
{
	umm_free(p);
}

void * ResRepresentation::operator new(std::size_t count, void * ptr)
{
	return ptr;
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

#ifdef REPRESENTATION_PRETTY_PRINT

void print_object( ResRepresentation &object, uint8_t indent );

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


void print_object( ResRepresentation &object, uint8_t indent )
{
  printf( "\n%*s{\n", indent, "" );
  indent += 4;

  for( auto &prop : object.get_props() )
  {
    auto prop_val = prop.second;

    switch( prop_val->get_type() )
    {
      case ResPropValType::boolean:
      {
        printf( "%*s\"%s\" : %s\n", indent, "", prop.first.c_str(), prop_val->get<bool>() == true ? "true" : "false" );
      }
      break;
      case ResPropValType::integer:
      {
        printf( "%*s\"%s\" : %d\n", indent, "", prop.first.c_str(), prop_val->get<long>() );
      }
      break;
      case ResPropValType::number:
      {
        printf( "%*s\"%s\" : %f\n", indent, "", prop.first.c_str(), prop_val->get<double>() );
      }
      break;
      case ResPropValType::string:
      {
        printf( "%*s\"%s\" : %s\n", indent, "", prop.first.c_str(), prop_val->get<std::string>().c_str() );
      }
      break;
      case ResPropValType::object:
      {
        printf( "%*s\"%s\" :", indent, "", prop.first.c_str() );
        print_object( prop_val->get<ResRepresentation>(), indent );
      }
      break;
      case ResPropValType::boolean_array:
      {
        printf( "%*s\"%s\" : [\n", indent, "", prop.first.c_str() );
        auto b_array = prop_val->get_array<std::vector<bool> >();
        print_array( *b_array, indent + 4 );
        printf( "%*s]\n", indent, "" );
      }
      break;
      case ResPropValType::integer_array:
      {
        printf( "%*s\"%s\" : [\n", indent, "", prop.first.c_str() );
        auto b_array = prop_val->get_array<std::vector<long> >();
        print_array( *b_array, indent + 4 );
        printf( "%*s]\n", indent, "" );
      }
      break;
      case ResPropValType::number_array:
      {
        printf( "%*s\"%s\" : [\n", indent, "", prop.first.c_str() );
        auto b_array = prop_val->get_array<std::vector<double> >();
        print_array( *b_array, indent + 4 );
        printf( "%*s]\n", indent, "" );
      }
      break;
      case ResPropValType::string_array:
      {
        printf( "%*s\"%s\" : [\n", indent, "", prop.first.c_str() );
        auto b_array = prop_val->get_array<std::vector<std::string> >();
        print_array( *b_array, indent + 4 );
        printf( "%*s]\n", indent, "" );
      }
      break;
      case ResPropValType::obj_array:
      {
        printf( "%*s\"%s\" : [\n", indent, "", prop.first.c_str() );
        auto b_array = prop_val->get_array<std::vector<ResRepresentation> >();
        print_array( *b_array, indent + 4 );
        printf( "%*s]\n", indent, "" );
      }
      break;
      default:
      {
      }
      break;
    }
  }

  printf( "%*s}\n", indent - 4, "" );
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

void * ResPropValue::operator new(size_t size)
{
	return umm_malloc(size);
}

void ResPropValue::operator delete(void * p)
{
	umm_free(p);
}

void * ResPropValue::operator new(std::size_t count, void * ptr)
{
	return ptr;
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

inline ResPropValue::~ResPropValue()
{
	destroy();
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
    new(&val.o_arr) std::vector<ResRepresentation>();
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
		this->val.s_arr.~vector();
    //this->val.s_arr.clear();
  }
  else if( val_type == ResPropValType::obj_array )
  {
    this->val.o_arr.clear();
  }

  val_type = ResPropValType::none;
}
}
}