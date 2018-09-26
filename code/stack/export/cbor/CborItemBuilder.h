#pragma once

#include "CborListener.h"
#include "CborItem.h"
#include <stack>

struct BuilderContextInfo
{
  BuilderContextInfo( CborItem *item, size_t subitems = 0 ) : _item{ item }, _subitems{ subitems } {}

  auto get_item_type() { return ( _item->get_type() ); }
  auto get_item() { return ( _item ); }
  void set_subitem_count( size_t new_count ) { _subitems = new_count; }
  auto get_subitem_count() { return ( _subitems ); }
  void dec_subitem_count() { _subitems--; }
  auto is_subitems_empty() { return ( _subitems == 0 ); }

  CborItem * _item     = nullptr;
  size_t     _subitems = 0;
};


class CborItemBuilder : public CborListener
{
  public:

    void append( CborItem *item );
    void push( CborItem *item, size_t size = 0 ) { _stack.push( { item, size } ); }
		static CborItem* Load(cbor_data source, size_t source_size, cbor_load_result *result);

    auto get_root_item() { return ( _root_item ); }
    void set_root_item( CborItem *item ) { _root_item = item; }
    auto get_item_count() { return ( _stack.size() ); }
		bool is_creation_failed() { return _creation_failed; }
		bool is_syntax_error() { return _syntax_error; }

  public:
    void on_uint8( uint8_t u8_value ) override;
    void on_uint16( uint16_t u16_value ) override;
    void on_uint32( uint32_t u32_value ) override;
    void on_uint64( uint64_t u64_value ) override;
    void on_int8( int8_t i8_value ) override;
    void on_int16( int16_t i16_value ) override;
    void on_int32( int32_t i32_value ) override;
    void on_int64( int64_t i64_value ) override;
    void on_float16( float i16_value ) override;
    void on_float32( float i32_value ) override;
    void on_float64( double i64_value ) override;
    void on_indefinite_byte_string() override;
    void on_definite_byte_string( uint8_t *data, size_t size ) override;
    void on_indefinite_text_string() override;
    void on_definite_text_string(uint8_t *data, size_t size) override;
    void on_indefinite_array() override;
    void on_definite_array(uint32_t size ) override;
    void on_indefinite_map() override;
    void on_definite_map(uint32_t size ) override;
    void on_tag(uint64_t tag ) override;
    void on_bool( bool ) override;
    void on_null() override;
    void on_undefined() override;
    void on_indefinite_break() override;
    void on_error( const char *error ) override;

  private:
    bool                             _creation_failed = false;
    bool                             _syntax_error    = false;
    CborItem *                       _root_item      = nullptr;
    std::stack<BuilderContextInfo>   _stack;
};