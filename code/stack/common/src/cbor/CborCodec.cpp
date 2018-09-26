/*
 * CborCodec.cpp
 *
 *  Created on: Feb 28, 2018
 *      Author: psammand
 */

#include <cbor/CborCodec.h>
#include <cbor/CborEncoder.h>
#include <cbor/ICborDecoderListener.h>
#include <cbor/CborDecoderBuffer.h>
#include <stack>
#include "ResPropValue.h"
#include "cbor/CborItem.h"
#include "cbor/CborItemBuilder.h"

using namespace ja_iot::base;
using namespace ja_iot::stack;

uint8_t encode_representation( ResRepresentation &res_rep, CborEncoder &encoder );
uint8_t encode_object( ResRepresentation &res_rep, CborEncoder &encoder );

template<typename T>
void encode_value_array( std::vector<T> &value_array, CborEncoder &encoder )
{
  for( auto &value : value_array )
  {
    encoder.write( value );
  }
}

template<>
void encode_value_array<ResRepresentation>( std::vector<ResRepresentation> &value_array, CborEncoder &encoder )
{
  for( auto &value : value_array )
  {
    encode_object( value, encoder );
  }
}

template<>
void encode_value_array<bool>( std::vector<bool> &value_array, CborEncoder &encoder )
{
  for( auto value : value_array )
  {
    encoder.write( value );
  }
}

template<typename T>
void encode_array( std::vector<T> &value_array, CborEncoder &encoder )
{
  if( value_array.empty() )
  {
    encoder.write_null();
  }
  else
  {
    encoder.write_array( static_cast<uint16_t>( value_array.size() ) );
    encode_value_array<T>( value_array, encoder );
  }
}

uint8_t encode_object( ResRepresentation &res_rep, CborEncoder &encoder )
{
  encoder.write_map( res_rep.no_of_props() );

  for( auto it = res_rep.get_props().cbegin(); it != res_rep.get_props().cend(); ++it )
  {
    auto &key  = it->first;
    auto value = it->second;

    encoder.write_string( (std::string &) key );

    switch( value->get_type() )
    {
      case ResPropValType::boolean:
      {
        encoder.write( value->get<bool>() );
      }
      break;
      case ResPropValType::integer:
      {
        encoder.write<long>( value->get<long>() );
      }
      break;
      case ResPropValType::number:
      {
        encoder.write( value->get<double>() );
      }
      break;
      case ResPropValType::string:
      {
        encoder.write( value->get<std::string>() );
      }
      break;
      case ResPropValType::object:
      {
        encode_object( value->get<ResRepresentation>(), encoder );
      }
      break;
      case ResPropValType::boolean_array:
      {
        encode_array<bool>( value->get<std::vector<bool> >(), encoder );
      }
      break;
      case ResPropValType::integer_array:
      {
        encode_array<long>( value->get<std::vector<long> >(), encoder );
      }
      break;
      case ResPropValType::number_array:
      {
        encode_array<double>( value->get<std::vector<double> >(), encoder );
      }
      break;
      case ResPropValType::string_array:
      {
        encode_array<std::string>( value->get<std::vector<std::string> >(), encoder );
      }
      break;
      case ResPropValType::obj_array:
      {
        encode_array<ResRepresentation>( value->get<std::vector<ResRepresentation> >(), encoder );
      }
      break;
      default:
      {
        encoder.write_null();
      }
      break;
    }
  }

  return ( 0 );
}

uint8_t encode_representation( ResRepresentation &res_rep, CborEncoder &encoder )
{
  if( ( res_rep.no_of_props() == 0 ) || ( res_rep.no_of_props() > 1 ) )
  {
    return ( 1 );
  }

  auto it    = res_rep.get_props().cbegin();
  auto value = ( *it ).second;

  if( value->get_type() == ResPropValType::obj_array )
  {
    encode_array( value->get<std::vector<ResRepresentation> >(), encoder );
  }
  else if( value->get_type() == ResPropValType::object )
  {
    encode_object( value->get<ResRepresentation>(), encoder );
  }
  else
  {
    return ( 1 );
  }

  return ( 0 );
}

ErrCode decode_object( CborMapItem *map_item, ResRepresentation &properties );
ErrCode decode_object_array( CborArrayItem *map_item, ResRepresentation &properties );

ErrCode decode_object( CborMapItem *map_item, ResRepresentation &properties )
{
  for( auto &loop_item : map_item->get_data_list() )
  {
    if( ( loop_item.key == nullptr ) || ( loop_item.value == nullptr ) || ( loop_item.key->get_type() != CBOR_TYPE_STRING ) )
    {
      return ( ErrCode::INVALID_PARAMS );
    }

    auto key_item = static_cast<CborTextStringItem *>( loop_item.key );

    if( ( key_item->get_length() == 0 ) || ( key_item->get_data() == nullptr ) )
    {
      return ( ErrCode::INVALID_PARAMS );
    }

    std::string key{ (char *) key_item->get_data(), key_item->get_length() };

    auto val_item = loop_item.value;

    switch( val_item->get_type() )
    {
      case CBOR_TYPE_BYTESTRING:
      case CBOR_TYPE_TAG:
      {
      }
      break;
      case CBOR_TYPE_UINT:
      {
        properties.add( key, (long) static_cast<CborIntItem *>( val_item )->get_int() );
      }
      break;
      case CBOR_TYPE_NEGINT:
      {
        properties.add( key, (long) ( ( int32_t ) static_cast<CborIntItem *>( val_item )->get_int() ) );
      }
      break;
      case CBOR_TYPE_STRING:
      {
        auto string_item = static_cast<CborTextStringItem *>( val_item );
        std::string str{ (const char *) string_item->get_data(), string_item->get_length() };

        properties.add( key, str );
      }
      break;
      case CBOR_TYPE_ARRAY:
      {
        auto array_item = static_cast<CborArrayItem *>( val_item );
        auto first_item = array_item->get_item( 0 );

        if( first_item == nullptr )
        {
          break;
        }

        switch( first_item->get_type() )
        {
          case CBOR_TYPE_UINT:
          case CBOR_TYPE_NEGINT:
          {
            std::vector<long> long_array{};
            long_array.reserve( array_item->get_size() );

            for( auto &l : array_item->get_item_list() )
            {
              long_array.push_back( (long) static_cast<CborIntItem *>( l )->get_int() );
            }

            properties.add( key, std::move( long_array ) );
          }
          break;
          case CBOR_TYPE_STRING:
          {
            std::vector<std::string> string_array{};
            string_array.reserve( array_item->get_size() );

            for( auto &str : array_item->get_item_list() )
            {
              auto string_item = static_cast<CborTextStringItem *>( str );
              std::string val_str{ (const char *) string_item->get_data(), string_item->get_length() };

              string_array.push_back( std::move( val_str ) );
            }

            properties.add( key, std::move( string_array ) );
          }
          break;

          case CBOR_TYPE_FLOAT_CTRL:
          {
            auto special_item = static_cast<CborSpecialItem *>( first_item );

            if( special_item->is_ctrl() )
            {
              if( special_item->is_bool() )
              {
                std::vector<bool> b_array{};
                b_array.reserve( array_item->get_size() );

                for( auto &loop_item : array_item->get_item_list() )
                {
                  b_array.push_back( static_cast<CborSpecialItem *>( loop_item )->get_bool() );
                }

                properties.add( key, std::move( b_array ) );
              }
            }
            else
            {
              std::vector<double> d_array{};
              d_array.reserve( array_item->get_size() );

              for( auto &loop_item : array_item->get_item_list() )
              {
                d_array.push_back( static_cast<CborSpecialItem *>( loop_item )->get_float() );
              }

              properties.add( key, std::move( d_array ) );
            }
          }
          break;
          case CBOR_TYPE_MAP:
          {
            std::vector<ResRepresentation> obj_array{};
            obj_array.reserve( array_item->get_size() );

            for( auto &loop_item : array_item->get_item_list() )
            {
              ResRepresentation new_object{};

              if( decode_object( static_cast<CborMapItem *>( loop_item ), new_object ) == ErrCode::OK )
              {
                obj_array.push_back( std::move( new_object ) );
              }
              else
              {
                return ( ErrCode::INVALID_PARAMS );
              }
            }

            properties.add( key, std::move( obj_array ) );
          }
          break;
          default:
          {
          }
          break;
        }
      }
      break;
      case CBOR_TYPE_MAP:
      {
        ResRepresentation new_object{};

        if( decode_object( static_cast<CborMapItem *>( val_item ), new_object ) == ErrCode::OK )
        {
          properties.add( key, std::move( new_object ) );
        }
        else
        {
          return ( ErrCode::INVALID_PARAMS );
        }
      }
      break;
      case CBOR_TYPE_FLOAT_CTRL:
      {
        auto special_item = static_cast<CborSpecialItem *>( val_item );

        if( special_item->is_ctrl() )
        {
          if( special_item->is_bool() )
          {
            properties.add( key, special_item->get_bool() );
          }
        }
        else
        {
          properties.add( key, special_item->get_float() );
        }
      }
      break;
      default:
      {
      }
      break;
    }
  }

  return ( ErrCode::OK );
}

ErrCode decode_object_array( CborArrayItem *array_item, ResRepresentation &properties )
{
  std::vector<ResRepresentation> obj_array{};
  obj_array.reserve( array_item->get_size() );

  for( auto &loop_item : array_item->get_item_list() )
  {
    ResRepresentation new_object{};

    if( decode_object( static_cast<CborMapItem *>( loop_item ), new_object ) == ErrCode::OK )
    {
      obj_array.push_back( std::move( new_object ) );
    }
    else
    {
      return ( ErrCode::INVALID_PARAMS );
    }
  }

  properties.add( "", obj_array );

  return ( ErrCode::OK );
}

ErrCode decode_representation( CborItem *root_map_item, ResRepresentation &properties )
{
  if( root_map_item->get_type() == CBOR_TYPE_MAP )
  {
    decode_object( static_cast<CborMapItem *>( root_map_item ), properties );
  }
  else if( root_map_item->get_type() == CBOR_TYPE_ARRAY )
  {
    decode_object_array( static_cast<CborArrayItem *>( root_map_item ), properties );
  }
  else
  {
    return ( ErrCode::INVALID_PARAMS );
  }

  return ( ErrCode::OK );
}

uint8_t buffer[1024];

ErrCode CborCodec::encode( ResRepresentation &res_representation, uint8_t * &dst_buffer, uint16_t &dst_buffer_len )
{
  dst_buffer     = nullptr;
  dst_buffer_len = 0;

  CborEncoderBuffer encode_buffer{ &buffer[0], 1024 };
  CborEncoder encoder{ &encode_buffer };

  encode_representation( res_representation, encoder );

  if( encode_buffer.size() > 0 )
  {
    dst_buffer = new uint8_t[encode_buffer.size()];
    memcpy( dst_buffer, encode_buffer.data(), encode_buffer.size() );
    dst_buffer_len = encode_buffer.size();

    return ( ErrCode::OK );
  }

  return ( ErrCode::ERR );
}

ErrCode CborCodec::decode( uint8_t *src_buffer, uint16_t src_buffer_len, ResRepresentation &res_representation )
{
  if( src_buffer_len == 0 )
  {
    return ( ErrCode::INVALID_PARAMS );
  }

  struct cbor_load_result result;
  CborItem *              root_item = CborItemBuilder::Load( src_buffer, src_buffer_len, &result );

  if( ( root_item == nullptr ) || ( result.error.code != CBOR_ERR_NONE ) )
  {
    return ( ErrCode::INVALID_PARAMS );
  }

  if( decode_representation( root_item, res_representation ) != ErrCode::OK )
  {
    return ( ErrCode::INVALID_PARAMS );
  }

  delete root_item;

  return ( ErrCode::OK );
}

uint8_t encode_object( CborItem *root_map, ResRepresentation &object )
{
  auto &props_list = object.get_props();

  for( auto it = props_list.cbegin(); it != props_list.cend(); ++it )
  {
    auto      &key  = it->first;
    auto      value = it->second;

    auto      string_key = new CborTextStringItem{ key.c_str() };
    CborItem *cbor_value = nullptr;

    switch( value->get_type() )
    {
      case ResPropValType::boolean:
      {
        cbor_value = new CborSpecialItem{ value->get<bool>() ? CBOR_CTRL_TRUE : CBOR_CTRL_FALSE };
      }
      break;
      case ResPropValType::integer:
      {
        auto v = value->get<long>();
        cbor_value = new CborIntItem{ v };
        ( v < 0 ) ? static_cast<CborIntItem *>( cbor_value )->mart_negint() : static_cast<CborIntItem *>( cbor_value )->mart_uint();
      }
      break;
      case ResPropValType::number:
      {
        cbor_value = new CborSpecialItem{ value->get<double>() };
      }
      break;
      case ResPropValType::string:
      {
        cbor_value = new CborTextStringItem{ value->get<std::string>().c_str() };
      }
      break;
      case ResPropValType::object:
      {
        auto &obj = value->get<ResRepresentation>();
        cbor_value = new CborMapItem{ obj.get_props().size() };
        encode_object( cbor_value, obj );
      }
      break;
      case ResPropValType::boolean_array:
      {
        auto &b_array   = value->get<std::vector<bool> >();
        auto array_item = new CborArrayItem{ b_array.size() };

        for( auto b : b_array )
        {
          auto item = new CborSpecialItem{ b ? CBOR_CTRL_TRUE : CBOR_CTRL_FALSE };
          array_item->add( item );
        }

        cbor_value = array_item;
      }
      break;
      case ResPropValType::integer_array:
      {
        auto &long_array = value->get<std::vector<long> >();
        auto array_item  = new CborArrayItem{ long_array.size() };

        for( auto &l : long_array )
        {
          auto item = new CborIntItem{ l };
          ( l < 0 ) ? static_cast<CborIntItem *>( item )->mart_negint() : static_cast<CborIntItem *>( item )->mart_uint();
          array_item->add( item );
        }

        cbor_value = array_item;
      }
      break;
      case ResPropValType::number_array:
      {
        auto &double_array = value->get<std::vector<double> >();
        auto array_item    = new CborArrayItem{ double_array.size() };

        for( auto &d : double_array )
        {
          auto item = new CborSpecialItem{ d };
          array_item->add( item );
        }

        cbor_value = array_item;
      }
      break;
      case ResPropValType::string_array:
      {
        auto &string_array = value->get<std::vector<std::string> >();
        auto array_item    = new CborArrayItem{ string_array.size() };

        for( auto &str : string_array )
        {
          auto item = new CborTextStringItem{ str.c_str() };
          array_item->add( item );
        }

        cbor_value = array_item;
      }
      break;
      case ResPropValType::obj_array:
      {
        auto &obj_array = value->get<std::vector<ResRepresentation> >();
        auto array_item = new CborArrayItem{ obj_array.size() };

        for( auto &obj : obj_array )
        {
          auto *sub_root_map = new CborMapItem{ obj.get_props().size() };
          encode_object( sub_root_map, obj );
          array_item->add( sub_root_map );
        }

        cbor_value = array_item;
      }
      break;
			default:
				break;
    }

    if( cbor_value != nullptr )
    {
      static_cast<CborMapItem *>( root_map )->add_data( { string_key, cbor_value } );
    }
  }

  return ( 0 );
}


#if 0

static int8_t length_lookup[8] = { 1, 2, 4, 8, 0, 0, 0, 0 };  // 24, 25, 26, 27, 28, 29, 30, 31

struct Mapinfo
{
  ResPropValue * key;
  ResPropValue * value;
};

struct ParsedData
{
  ParsedData( ResPropValue *v ) : type{ 0 }, value{ v } {}
  ParsedData( std::string &str ) : type{ 1 }, string{ str } {}

  uint8_t        type  = 0;
  ResPropValue * value = nullptr;
  std::string    string;
};

struct DecodeInfo
{
  uint8_t                type       = 0;
  uint16_t               subitems   = 0;
  uint8_t                array_type = 0;
  ResPropValue *         value      = nullptr;
  std::vector<Mapinfo>   _map_values{};
};

void append( std::stack<DecodeInfo> &stack, ResPropValue *value )
{
  if( stack.empty() )
  {
    return;
  }

  auto &decode_info = stack.top();

  if( decode_info.type == 0 )
  {
    if( decode_info.subitems % 2 )
    {
      auto &map_info = decode_info._map_values.back();
      map_info.value = value;
    }
    else
    {
      Mapinfo map_info;
      map_info.key = value;
      decode_info._map_values.push_back( map_info );
    }

    if( --decode_info.subitems == 0 )
    {
      for( auto &m : decode_info._map_values )
      {
        decode_info.value->get<ResRepresentation>().get_props()[m.key->get<std::string>()] = m.value;
        delete m.key;
      }

      stack.pop();
      append( stack, decode_info.value );
    }
  }
  else
  {
    switch( value->get_type() )
    {
      case ResPropValType::boolean:
      {
        if( decode_info.value == nullptr )
        {
          decode_info.value = new ResPropValue{ ResPropValType::boolean_array };
        }

        decode_info.value->get<std::vector<bool> >().push_back( value->get<bool>() );
      }
      break;
      case ResPropValType::integer:
      {
        if( decode_info.value == nullptr )
        {
          decode_info.value = new ResPropValue{ ResPropValType::integer_array };
        }

        decode_info.value->get<std::vector<long> >().push_back( value->get<long>() );
      }
      break;
    }

    delete value;

    if( --decode_info.subitems == 0 )
    {
      stack.pop();
      append( stack, decode_info.value );
    }
  }
}

ErrCode CborCodec::decode( uint8_t *src_buffer, uint16_t src_buffer_len, ResRepresentation &res_representation )
{
  if( src_buffer_len == 0 )
  {
    return ( ErrCode::OK );
  }

  bool to_continue = true;
  CborDecoderBuffer decoder_buffer{ src_buffer, src_buffer_len };

  if( ( decoder_buffer.get_major_type() != 0x04 ) && ( decoder_buffer.get_major_type() != 0x05 ) )
  {
    return ( ErrCode::ERR );
  }

  std::stack<DecodeInfo> _stack{};

  ResRepresentation *current_container = &res_representation;

  while( to_continue )
  {
    if( !decoder_buffer.has_bytes( 1 ) )
    {
      break;
    }

    uint8_t  initial_byte    = decoder_buffer.get<uint8_t>();
    uint8_t  major_type      = initial_byte >> 5;
    uint8_t  additional_info = initial_byte & 0x1F;
    uint64_t value           = additional_info;
    uint8_t  length          = 0;

    if( additional_info >= 24 )
    {
      length = length_lookup[additional_info - 24];

      if( length == 0 )
      {
        continue;
      }

      if( !decoder_buffer.has_bytes( length ) )
      {
        to_continue = false; continue;
      }

      switch( length )
      {
        case 1:
        { value = decoder_buffer.get<uint8_t>(); }
        break;
        case 2:
        { value = decoder_buffer.get<uint16_t>(); }
        break;
        case 4:
        { value = decoder_buffer.get<uint32_t>(); }
        break;
        case 8:
        { value = decoder_buffer.get<uint64_t>(); }
        break;
      }
    }

    if( ( major_type == 2 ) || ( major_type == 3 ) )
    {
      if( !decoder_buffer.has_bytes( static_cast<int32_t>( value ) ) )
      {
        to_continue = false; continue;
      }
    }

    switch( major_type )
    {
      case 4:
      {
        DecodeInfo decode_info;
        decode_info.type     = 1;
        decode_info.subitems = value;

        _stack.push( decode_info );
      }
      break;
      case 5:
      {
        DecodeInfo decode_info;
        decode_info.type     = 0;
        decode_info.subitems = value * 2;
        decode_info.value    = new ResPropValue{ ResPropValType::object };

        _stack.push( decode_info );
      }
      break;

      case 0:
      {
        auto v = new ResPropValue{ (long) value };
        append( _stack, v );
      }
      break;

      case 3:
      {
        uint8_t *   data = new uint8_t[static_cast<uint32_t>( value )];
        decoder_buffer.get_bytes( data, static_cast<int32_t>( value ) );
        std::string str( (const char *) data, (size_t) value );
        auto        v = new ResPropValue{ str };
        append( _stack, v );
      }
      break;
    }
  }

  return ( ErrCode::OK );
}
#endif