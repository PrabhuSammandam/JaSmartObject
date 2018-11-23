#pragma once

#include "CborTypedefs.h"
#include <vector>
#include <type_traits>

struct CborItem
{
  CborItem () : CborItem{ CBOR_TYPE_UINT } {}
  CborItem( cbor_type type ) : CborItem{ type, false } {}
  CborItem( cbor_type type, bool is_stream ) : _type{ type }, _is_stream{ is_stream } {}
  CborItem( CborItem &other ) { *this = other; }
  virtual ~CborItem () {}

  CborItem & operator = ( CborItem &other );

  auto      is_stream() { return ( _is_stream ); }
  void      set_stream( bool v = true ) { _is_stream = v; }
  cbor_type get_type() { return ( (cbor_type) _type ); }
  void      set_type( cbor_type type ) { this->_type = type; }
	bool is_map() { return _type == CBOR_TYPE_MAP; }
	bool is_array() { return _type == CBOR_TYPE_ARRAY; }

  virtual size_t encode( uint8_t *buffer, size_t buffer_len ) { return ( 0 ); }

  uint8_t   _type      : 4;
  uint8_t   _is_stream : 1;
  uint8_t   _reserved  : 3;
};

template<typename T,
typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
struct IntItemTypeTrait
{
  static constexpr uint8_t   WIDTH = sizeof( T );
  static constexpr uint8_t   TYPE  = std::is_signed<T>::value ? CBOR_TYPE_NEGINT : CBOR_TYPE_UINT;
};

struct CborIntItem : public CborItem
{
  template<typename T>
  CborIntItem( T value ) : CborItem{}
  {
    IntItemTypeTrait<T> t;
    this->_width = (cbor_int_width) t.WIDTH;
    set_type( (cbor_type) t.TYPE );
    _value = value;
  }
  CborIntItem( const CborIntItem &other );
  CborIntItem & operator = ( const CborIntItem &other );

  size_t encode( uint8_t *buffer, size_t buffer_len ) override;

  template<typename T> T    get() { return ( (T) _value ); }
  template<typename T> void set( T value ) { _value = value; }
  void                      mart_uint() { set_type( CBOR_TYPE_UINT ); }
  void                      mart_negint() { set_type( CBOR_TYPE_NEGINT ); }
  cbor_int_width            get_width() { return ( _width ); }
  uint64_t                  get_int() { return ( _value ); }

  cbor_int_width   _width;
  uint64_t         _value = 0;
};

struct CborByteStringItem : public CborItem
{
  CborByteStringItem( bool is_stream = false );
  CborByteStringItem( CborByteStringItem &other );
  virtual ~CborByteStringItem ();

  CborByteStringItem & operator = ( CborByteStringItem &other );

  size_t encode( uint8_t *buffer, size_t buffer_len ) override;

  uint8_t*               get_data() { return ( this->data ); }
  void                   set_data( uint8_t *data, size_t length );
  bool                   add_data( uint8_t *data, size_t length );
  std::vector<CborItem *>& get_chunks_list() { return ( _items ); }
  size_t                 get_chunk_count() { return ( _items.size() ); }
  bool                   add_chunk( CborItem *chunk );
  void                   clear() { _items.clear(); }
  size_t                 get_length() { return ( _length ); }

  size_t                    _length = 0;
  std::vector<CborItem *>   _items;
  uint8_t *                 data = nullptr;
};

struct CborTextStringItem : public CborItem
{
  CborTextStringItem( bool is_stream = false );
  CborTextStringItem( const char *val );
  CborTextStringItem( const char *val, size_t length );
  virtual ~CborTextStringItem ();

  CborTextStringItem & operator = ( CborTextStringItem &other );

  size_t encode( uint8_t *buffer, size_t buffer_len ) override;

  void                   set_data( uint8_t *data, size_t length );
  bool                   add_data( uint8_t *data, size_t length );
  size_t                 get_chunk_count() { return ( _items.size() ); }
  std::vector<CborItem *>& get_chunk_list() { return ( _items ); }
  uint8_t*               get_data() { return ( this->data ); }
  size_t                 get_length() { return ( _length ); }
  size_t                 get_codepoint_count() { return ( _codepoint_count ); }
  void                   set_codepoint_count( size_t count ) { _codepoint_count = count; }
  bool                   add_chunk( CborItem *chunk );
  void                   clear() { _items.clear(); }

  size_t                    _length          = 0;
  size_t                    _codepoint_count = 0;                /* Sum of chunks' codepoint_counts for indefinite strings */
  std::vector<CborItem *>   _items;
  uint8_t *                 data = nullptr;
};

struct CborArrayItem : public CborItem
{
  CborArrayItem( bool is_stream = false );
  CborArrayItem( size_t size );
  virtual ~CborArrayItem ();

  size_t encode( uint8_t *buffer, size_t buffer_len ) override;

  size_t    get_size() { return ( _items.size() ); }
  CborItem* get_item( size_t index ) { return ( _items[index] ); }
  bool      set_item( size_t index, CborItem *new_item ) { _items[index] = new_item; return ( true ); }
  bool      replace_item( size_t index, CborItem *new_item );
  bool      add( CborItem *new_item );
  auto      & get_item_list() { return ( _items ); }
  void      clear() { _items.clear(); }

  std::vector<CborItem *>   _items;
};

struct cbor_pair
{
  cbor_pair( CborItem *k, CborItem *v ) : key{ k }, value{ v } {}
  CborItem * key, *value;
};

struct CborMapItem : public CborItem
{
  CborMapItem( bool is_stream = false );
  CborMapItem( size_t size );
  virtual ~CborMapItem ();

  size_t encode( uint8_t *buffer, size_t buffer_len ) override;

  bool                  add_key( CborItem *key );
  bool                  add_value( CborItem *value );
  bool                  add_data( cbor_pair pair );
  size_t                get_size() { return ( _items.size() ); }
  std::vector<cbor_pair>& get_data_list() { return ( _items ); }
  void                  clear() { _items.clear(); }

  std::vector<cbor_pair>   _items;
};

struct CborTaggedItem : public CborItem
{
  CborTaggedItem ();
  CborTaggedItem( uint64_t value );
  CborTaggedItem( CborItem *tagged_item, uint64_t value );

  size_t encode( uint8_t *buffer, size_t buffer_len ) override;

  void      set_tagged_item( CborItem *tagged_item );
  CborItem* get_tagged_item() { return ( _tagged_item ); }
  uint64_t  get_value() { return ( _value ); }
  void      set_value( uint64_t value ) { _value = value; }

  struct CborItem * _tagged_item = nullptr;
  uint64_t          _value       = 0;
};

template<typename T,
typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
struct FloatItemTypeTrait
{
  static constexpr uint8_t   WIDTH = sizeof( T );
};

struct CborSpecialItem : public CborItem
{
  CborSpecialItem ();
  CborSpecialItem( cbor_float_width width );
  CborSpecialItem( _cbor_ctrl ctrl_value );

  template<typename T>
  CborSpecialItem( T value ) : CborItem( CBOR_TYPE_FLOAT_CTRL )
  {
    FloatItemTypeTrait<T> t;
    _width = (cbor_float_width) t.WIDTH;
    _value = value;
  }
  CborSpecialItem( const CborSpecialItem &other );

  size_t encode( uint8_t *buffer, size_t buffer_len ) override;

  CborSpecialItem & operator = ( const CborSpecialItem &other );
  template<typename T> T     get() { return ( (T) _value ); }
  template<typename T> void  set( T value ) { _value = value; }
  double                     get_float() { return ( _value ); }
  cbor_float_width           get_width() { return ( _width ); }
  uint8_t                    get_ctrl() { return ( _ctrl ); }
  void                       set_ctrl( uint8_t ctrl ) { _ctrl = ctrl; }
  bool                       is_ctrl() { return ( _width == CBOR_FLOAT_0 ); }
  bool                       is_bool() { return ( _ctrl == CBOR_CTRL_TRUE || _ctrl == CBOR_CTRL_FALSE ); }
  bool                       get_bool() { return ( _ctrl == CBOR_CTRL_TRUE ); }

  cbor_float_width   _width = CBOR_FLOAT_0;
  uint8_t            _ctrl  = CBOR_CTRL_NONE;
  double             _value = 0.0;
};