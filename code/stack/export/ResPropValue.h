#pragma once
#include <vector>
#include <string>
#include <deque>
#include <map>
#include <forward_list>

namespace ja_iot {
namespace stack {
enum class ResPropValType : unsigned char
{
  none          = 0,
  boolean       = 1,
  integer       = 2,
  number        = 3,
  string        = 4,
  object        = 5,
  boolean_array = 6,
  integer_array = 7,
  number_array  = 8,
  string_array  = 9,
  obj_array     = 10,
};

class ResPropValue;
typedef std::map<std::string, ResPropValue *> ResPropMap;

/**
 * Class used to store the key,value.
 * This is customized dictionary class for storing key, value pairs.
 *
 * The key is always std::string
 * The value is instance of ResPropValue
 */
class ResRepresentation
{
  public:
    ResRepresentation () = default;
    ~ResRepresentation ();
    ResRepresentation( const ResRepresentation &other );// copy constructor
    ResRepresentation( ResRepresentation &&other ) noexcept;// move constructor
    ResRepresentation & operator = ( const ResRepresentation &other );// assignment operator
    ResRepresentation & operator = ( ResRepresentation &&other ) noexcept;// move operator

    ResPropMap& get_props()
    {
      return ( _props );
    }

    /***********************************************************************************/
    template<typename T,
    typename std::enable_if<std::is_class<T>::value, int>::type = 0>
    void add( const std::string &property_name, T &val );

    template<typename T,
    typename std::enable_if<std::is_fundamental<T>::value, int>::type = 0>
    void add( const std::string &property_name, T val );

    /**
     * Add the value by move semantics
     * @param property_name
     * @param val
     */
    template<typename T,
    typename std::enable_if<std::is_class<T>::value, int>::type = 0>
    void add( const std::string &property_name, T &&val );

    /***********************************************************************************/
    template<typename T,
    typename std::enable_if<std::is_fundamental<T>::value, int>::type = 0>
    T get_prop( const std::string &property_name, T def_value = 0 );

    template<typename T,
    typename std::enable_if<std::is_class<T>::value, int>::type = 0>
    bool get_prop( const std::string &property_name, T &prop_value );
    /***********************************************************************************/

    void     print();
    bool     has_prop( const std::string &property_name );
    uint16_t no_of_props() const;
    void     remove_prop( const std::string &property_name );// removes the property from map.
    void     clear_prop( const std::string &property_name );// only clears the value of property, the property still in the map.
    bool     is_prop_none( const std::string &property_name );

  private:
    ResPropMap   _props;
};

union PropUnion
{
  PropUnion() {
  }
  ~PropUnion () {
  }
  bool                           b;
  long                           l;
  double                         d;
  std::string                    str;
  ResRepresentation              obj;
  std::vector<bool>              b_arr;
  std::vector<long>              l_arr;
  std::vector<double>            d_arr;
  std::vector<std::string>       s_arr;
  std::vector<ResRepresentation> o_arr;
};

template<typename T> struct res_prop_type_trait
{
  static constexpr ResPropValType   PROP_VAL_TYPE = ResPropValType::none;
};

template<> struct res_prop_type_trait<bool>
{
  static constexpr ResPropValType   PROP_VAL_TYPE = ResPropValType::boolean;
  typedef bool PROP_VALUE;
};

template<> struct res_prop_type_trait<long>
{
  static constexpr ResPropValType   PROP_VAL_TYPE = ResPropValType::integer;
  typedef long PROP_VALUE;
};

template<> struct res_prop_type_trait<double>
{
  static constexpr ResPropValType   PROP_VAL_TYPE = ResPropValType::number;
  typedef double PROP_VALUE;
};

template<> struct res_prop_type_trait<std::string>
{
  static constexpr ResPropValType   PROP_VAL_TYPE = ResPropValType::string;
  typedef std::string PROP_VALUE;
};

template<> struct res_prop_type_trait<ResRepresentation>
{
  static constexpr ResPropValType   PROP_VAL_TYPE = ResPropValType::object;
  typedef ResRepresentation PROP_VALUE;
};

template<> struct res_prop_type_trait<std::vector<bool> >
{
  static constexpr ResPropValType   PROP_VAL_TYPE = ResPropValType::boolean_array;
  typedef std::vector<bool> PROP_VALUE;
};

template<> struct res_prop_type_trait<std::vector<long> >
{
  static constexpr ResPropValType   PROP_VAL_TYPE = ResPropValType::integer_array;
  typedef std::vector<long> PROP_VALUE;
};

template<> struct res_prop_type_trait<std::vector<double> >
{
  static constexpr ResPropValType   PROP_VAL_TYPE = ResPropValType::number_array;
  typedef std::vector<double> PROP_VALUE;
};

template<> struct res_prop_type_trait<std::vector<std::string> >
{
  static constexpr ResPropValType   PROP_VAL_TYPE = ResPropValType::string_array;
  typedef std::vector<std::string> PROP_VALUE;
};

template<> struct res_prop_type_trait<std::vector<ResRepresentation> >
{
  static constexpr ResPropValType   PROP_VAL_TYPE = ResPropValType::obj_array;
  typedef std::vector<ResRepresentation> PROP_VALUE;
};

template<typename T>
struct PropTraits {};

template<>
struct PropTraits<bool> : res_prop_type_trait<bool>
{
  static void       initialize( PropUnion &value ) { value.b = false; }
  static PROP_VALUE get_value( PropUnion &value ) { return ( value.b ); }
  static void       set_value( PropUnion &value, const PROP_VALUE new_value ) { value.b = new_value; }
  static void       move_value( PropUnion &value, const PROP_VALUE new_value ) { value.b = new_value; }
};

template<>
struct PropTraits<long> : res_prop_type_trait<long>
{
  static void       initialize( PropUnion &value ) { value.l = 0; }
  static PROP_VALUE get_value( PropUnion &value ) { return ( value.l ); }
  static void       set_value( PropUnion &value, const PROP_VALUE new_value ) { value.l = new_value; }
  static void       move_value( PropUnion &value, const PROP_VALUE new_value ) { value.l = new_value; }
};

template<>
struct PropTraits<double> : res_prop_type_trait<double>
{
  static void       initialize( PropUnion &value ) { value.d = 0; }
  static PROP_VALUE get_value( PropUnion &value ) { return ( value.d ); }
  static void       set_value( PropUnion &value, const PROP_VALUE new_value ) { value.d = new_value; }
  static void       move_value( PropUnion &value, const PROP_VALUE new_value ) { value.d = new_value; }
};

template<>
struct PropTraits<std::string> : res_prop_type_trait<std::string>
{
  static void      initialize( PropUnion &value ) { new(&value.str) PROP_VALUE; }
  static PROP_VALUE& get_value( PropUnion &value ) { return ( value.str ); }
  static void      set_value( PropUnion &value, const PROP_VALUE &new_value ) { value.str = new_value; }
  static void      move_value( PropUnion &value, PROP_VALUE &new_value ) { value.str = std::move( new_value ); }
};

template<>
struct PropTraits<std::vector<bool> > : res_prop_type_trait<std::vector<bool> >
{
  static void      initialize( PropUnion &value ) { new(&value.b_arr) PROP_VALUE; }
  static PROP_VALUE& get_value( PropUnion &value ) { return ( value.b_arr ); }
  static void      set_value( PropUnion &value, const PROP_VALUE &new_value ) { value.b_arr = new_value; }
  static void      move_value( PropUnion &value, PROP_VALUE &new_value ) { value.b_arr = std::move( new_value ); }
};

template<>
struct PropTraits<std::vector<long> > : res_prop_type_trait<std::vector<long> >
{
  static void      initialize( PropUnion &value ) { new(&value.l_arr) PROP_VALUE; }
  static PROP_VALUE& get_value( PropUnion &value ) { return ( value.l_arr ); }
  static void      set_value( PropUnion &value, const PROP_VALUE &new_value ) { value.l_arr = new_value; }
  static void      move_value( PropUnion &value, PROP_VALUE &new_value ) { value.l_arr = std::move( new_value ); }
};

template<>
struct PropTraits<std::vector<double> > : res_prop_type_trait<std::vector<double> >
{
  static void      initialize( PropUnion &value ) { new(&value.d_arr) PROP_VALUE; }
  static PROP_VALUE& get_value( PropUnion &value ) { return ( value.d_arr ); }
  static void      set_value( PropUnion &value, const PROP_VALUE &new_value ) { value.d_arr = new_value; }
  static void      move_value( PropUnion &value, PROP_VALUE &new_value ) { value.d_arr = std::move( new_value ); }
};

template<>
struct PropTraits<std::vector<std::string> > : res_prop_type_trait<std::vector<std::string> >
{
  static void      initialize( PropUnion &value ) { new(&value.s_arr) PROP_VALUE; }
  static PROP_VALUE& get_value( PropUnion &value ) { return ( value.s_arr ); }
  static void      set_value( PropUnion &value, const PROP_VALUE &new_value ) { value.s_arr = new_value; }
  static void      move_value( PropUnion &value, PROP_VALUE &new_value ) { value.s_arr = std::move( new_value ); }
};

/***
 * Class that represents the value of the property.
 *
 * Following are the values supported by this class.
 * 1. bool
 * 2. long
 * 3. double
 * 4. std::string
 * 5. ResRepresentation (ie object)
 * 6. std::vector<bool>
 * 7. std::vector<long>
 * 8. std::vector<double>
 * 9. std::vector<std::string>
 * 10. std::vector<ResRepresentation>
 *
 * The special purpose of this class is that value is stored in the union. So because of union small memory is used
 * for the value and also supports the different data types with single unified interface.
 *
 * Following are the main operations supported by this class.
 * 1. set
 * 2. get
 * 3. move
 */
class ResPropValue
{
  public:
    ResPropValue( ResPropValType val_type = ResPropValType::none );// default constructor
    ResPropValue( const ResPropValue &other );// copy constructor
    ResPropValue( ResPropValue &&other ) noexcept;// move constructor

    /* Parameterized constructor. Valid types are bool, long, double, std::string, ResRepresentation and array of all the previous */
    template<typename T>
    explicit ResPropValue( const T &value )
    {
      typedef PropTraits<T> PropTraitsValue;
      val_type = PropTraitsValue::PROP_VAL_TYPE;
      PropTraitsValue::initialize( val );// based on the type initialize the union value.
      PropTraitsValue::set_value( val, value );// and finally set the passed value
    }

    ResPropValue & operator = ( const ResPropValue &other );// assignment operator, definition in cpp file.

    ResPropValue & operator = ( ResPropValue &&other ) noexcept;// move operator, definition in cpp file.
    ~ResPropValue ();           // definition in cpp file.

    void init( const ResPropValType value_type );

    void destroy();

    /***
     * Templated get function for fundamental data types ie bool, long, double
     * @return
     */
    template<typename T,
    typename std::enable_if<std::is_fundamental<T>::value, int>::type = 0>
    T get()
    {
      typedef PropTraits<T> PropTraitsValue;
      return ( PropTraitsValue::get_value( val ) );
    }

    /***
     * Templated get function for class based data types ie std::string, ResRepresentation, and array of all the data types.
     * This is different from the templated get function of fundatmental types by returing the reference of the class.
     * @return
     */
    template<typename T,
    typename std::enable_if<std::is_class<T>::value, int>::type = 0>
    T& get()
    {
      typedef PropTraits<T> PropTraitsValue;
      return ( PropTraitsValue::get_value( val ) );
    }

    template<typename T,
    typename std::enable_if<std::is_fundamental<T>::value, int>::type = 0>
    void set( const T new_value )
    {
      typedef PropTraits<T> PropTraitsValue;
      val_type = PropTraitsValue::PROP_VAL_TYPE;
      PropTraitsValue::set_value( val, new_value );
    }

    template<typename T,
    typename std::enable_if<std::is_class<T>::value, int>::type = 0>
    void set( const T &new_value )
    {
      typedef PropTraits<T> PropTraitsValue;

      destroy();
      val_type = PropTraitsValue::PROP_VAL_TYPE;
      PropTraitsValue::initialize( val );
      PropTraitsValue::set_value( val, new_value );
    }

    template<typename T,
    typename std::enable_if<std::is_fundamental<T>::value, int>::type = 0>
    void move( T new_value )
    {
      typedef PropTraits<T> PropTraitsValue;
      val_type = PropTraitsValue::PROP_VAL_TYPE;
      PropTraitsValue::initialize( val );
      PropTraitsValue::move_value( val, new_value );
    }

    template<typename T,
    typename std::enable_if<std::is_class<T>::value, int>::type = 0>
    void move( T &new_value )
    {
      typedef PropTraits<T> PropTraitsValue;

      destroy();
      val_type = PropTraitsValue::PROP_VAL_TYPE;
      PropTraitsValue::initialize( val );
      PropTraitsValue::move_value( val, new_value );
    }

    template<typename T>
    T* get_array()
    {
      typedef PropTraits<T> PropTraitsValue;

      if( PropTraitsValue::PROP_VAL_TYPE != val_type )
      {
        return ( nullptr );
      }

      return ( &PropTraitsValue::get_value( val ) );
    }

    ResPropValType get_type() { return ( val_type ); }

  public:
    PropUnion        val;
    ResPropValType   val_type;
};

template<>
struct PropTraits<ResRepresentation> : res_prop_type_trait<ResRepresentation>
{
  static void      initialize( PropUnion &value ) { new(&value.obj) PROP_VALUE; }
  static PROP_VALUE& get_value( PropUnion &value ) { return ( value.obj ); }
  static void      set_value( PropUnion &value, const PROP_VALUE &new_value )
  {
    value.obj = new_value;
  }
  static void move_value( PropUnion &value, PROP_VALUE &new_value ) { value.obj = std::move( new_value ); }
};

template<>
struct PropTraits<std::vector<ResRepresentation> > : res_prop_type_trait<std::vector<ResRepresentation> >
{
  static void      initialize( PropUnion &value ) { new(&value.o_arr) PROP_VALUE; }
  static PROP_VALUE& get_value( PropUnion &value ) { return ( value.o_arr ); }
  static void      set_value( PropUnion &value, const PROP_VALUE &new_value )
  {
    value.o_arr = new_value;
  }
  static void move_value( PropUnion &value, PROP_VALUE &new_value ) { value.o_arr = std::move( new_value ); }
};

/******************************************************************************************************************/
/************************************* ResRepresentation Implementations ******************************************/
/******************************************************************************************************************/
template<typename T,
typename std::enable_if<std::is_class<T>::value, int>::type>
void ResRepresentation::add( const std::string &property_name, T &val )
{
  _props[property_name] = new ResPropValue{ val };
}

template<typename T,
typename std::enable_if<std::is_fundamental<T>::value, int>::type>
void ResRepresentation::add( const std::string &property_name, T val )
{
  _props[property_name] = new ResPropValue{ val };
}

/**
 * This api will add the value by move semantics.
 * @param property_name
 * @param val
 */
template<typename T,
typename std::enable_if<std::is_class<T>::value, int>::type>
void ResRepresentation::add( const std::string &property_name, T &&val )
{
  auto rv = new ResPropValue{};

  rv->move( val );
  _props[property_name] = rv;
}

template<typename T,
typename std::enable_if<std::is_fundamental<T>::value, int>::type>
T ResRepresentation::get_prop( const std::string &property_name, T def_value )
{
  const auto find_result = _props.find( property_name );

  if( find_result != _props.end() )
  {
    ResPropValue *value = find_result->second;
    return ( value->get<T>() );
  }

  return ( def_value );
}

template<typename T,
typename std::enable_if<std::is_class<T>::value, int>::type>
bool ResRepresentation::get_prop( const std::string &property_name, T &prop_value )
{
  const auto find_result = _props.find( property_name );

  if( find_result != _props.end() )
  {
    ResPropValue *value = find_result->second;
    prop_value = value->get<T>();
    return ( true );
  }

  /* if there is no such property, then initialize to default value */
  prop_value = T();
  return ( false );
}
}
}
