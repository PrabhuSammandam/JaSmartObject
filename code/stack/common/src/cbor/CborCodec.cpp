/*
 * CborCodec.cpp
 *
 *  Created on: Feb 28, 2018
 *      Author: psammand
 */

#include <cbor/CborDecoderBuffer.h>
#include <cbor/CborEncoder.h>
#include <cbor/ICborDecoderListener.h>
#include <CborCodec.h>
#include <stack>
#include "ResPropValue.h"
#include "common/inc/cbor/CborItem.h"
#include "common/inc/cbor/CborItemBuilder.h"

using namespace ja_iot::base;
using namespace ja_iot::stack;

static ErrCode encode_representation( ResRepresentation &rcz_res_rep, CborEncoder &rcz_encoder );
static ErrCode encode_object( ResRepresentation &rcz_res_rep, CborEncoder &rcz_encoder );

/********************************************************************************************/
/**
 * This is the template function to encode the array of value of specified type.
 * Following are the types used by this function int, long, double, float.
 *
 * @param rcz_value_array - list of all values of type T.
 * @param encoder         - main encoder object.
 */
template<typename T>
void encode_value_array( std::vector<T> &rcz_value_array, CborEncoder &rcz_encoder )
{
  for( auto &value : rcz_value_array )
  {
    rcz_encoder.write( value );
  }
}

/**
 * This is the partial specialization function to encode 'ResRepresentation' values.
 *
 * @param rcz_value_array - list of 'ResRepresentation' values
 * @param encoder         - encoder object to encode
 */
template<>
void encode_value_array<ResRepresentation>( std::vector<ResRepresentation> &rcz_value_array, CborEncoder &rcz_encoder )
{
  for( auto &rcz_res_rep : rcz_value_array )
  {
    encode_object( rcz_res_rep, rcz_encoder );
  }
}

/**
 * This is the partial specialization function to encode 'bool' values.
 * @param rcz_value_array
 * @param rcz_encoder
 */
template<>
void encode_value_array<bool>( std::vector<bool> &rcz_value_array, CborEncoder &rcz_encoder )
{
  for( auto bool_value : rcz_value_array )
  {
    rcz_encoder.write( bool_value );
  }
}
/********************************************************************************************/

/**
 * This is the template function for encoding the CBOR array type.
 * The array type are bool, int, long, object, array, string, byte string.
 *
 * @param rcz_value_array
 * @param rcz_encoder
 */
template<typename T>
void encode_array( std::vector<T> &rcz_value_array, CborEncoder &rcz_encoder )
{
  if( rcz_value_array.empty() )
  {
    /* array has no value then write NULL CBOR type */
    rcz_encoder.write_null();
  }
  else
  {
    /* inform the encoder about the number of values */
    rcz_encoder.write_array( static_cast<uint16_t>( rcz_value_array.size() ) );
    /* encode the values*/
    encode_value_array<T>( rcz_value_array, rcz_encoder );
  }
}

static ErrCode encode_object( ResRepresentation &rcz_res_rep, CborEncoder &rcz_encoder )
{
  /* inform the encoder about the number of properties in the map */
  rcz_encoder.write_map( rcz_res_rep.no_of_props() );

  for( auto it = rcz_res_rep.get_props().cbegin(); it != rcz_res_rep.get_props().cend(); ++it )
  {
    auto &key  = it->first;
    auto value = it->second;

    /* encode the name of the object */
    rcz_encoder.write_string( (std::string &) key );

    switch( value->get_type() )
    {
      case ResPropValType::boolean:
      {
        rcz_encoder.write( value->get<bool>() );
      }
      break;
      case ResPropValType::integer:
      {
        rcz_encoder.write<long>( value->get<long>() );
      }
      break;
      case ResPropValType::number:
      {
        rcz_encoder.write( value->get<double>() );
      }
      break;
      case ResPropValType::string:
      {
        rcz_encoder.write( value->get<std::string>() );
      }
      break;
      case ResPropValType::object:
      {
        /* Recursively call the encode_object api */
        encode_object( value->get<ResRepresentation>(), rcz_encoder );
      }
      break;
      case ResPropValType::boolean_array:
      {
        encode_array<bool>( value->get<std::vector<bool> >(), rcz_encoder );
      }
      break;
      case ResPropValType::integer_array:
      {
        encode_array<long>( value->get<std::vector<long> >(), rcz_encoder );
      }
      break;
      case ResPropValType::number_array:
      {
        encode_array<double>( value->get<std::vector<double> >(), rcz_encoder );
      }
      break;
      case ResPropValType::string_array:
      {
        encode_array<std::string>( value->get<std::vector<std::string> >(), rcz_encoder );
      }
      break;
      case ResPropValType::obj_array:
      {
        encode_array<ResRepresentation>( value->get<std::vector<ResRepresentation> >(), rcz_encoder );
      }
      break;
      default:
      {
        rcz_encoder.write_null();
      }
      break;
    }
  }

  return ( ErrCode::OK );
}

/**
 * In OCF the top most resource representation is either single object or array of objects.
 *
 * @param rcz_res_rep
 * @param rcz_encoder
 * @return
 */
static ErrCode encode_representation( ResRepresentation &rcz_res_rep, CborEncoder &rcz_encoder )
{
  if( ( rcz_res_rep.no_of_props() == 0 ) || ( rcz_res_rep.no_of_props() > 1 ) )
  {
    return ( ErrCode::ERR );
  }

  auto it = rcz_res_rep.get_props().cbegin();

  if( ( it == rcz_res_rep.get_props().cend() ) || ( ( ( *it ).second ) == nullptr ) )
  {
    return ( ErrCode::ERR );
  }

  auto value = ( *it ).second;

  if( value->get_type() == ResPropValType::obj_array )
  {
    encode_array( value->get<std::vector<ResRepresentation> >(), rcz_encoder );
  }
  else if( value->get_type() == ResPropValType::object )
  {
    encode_object( value->get<ResRepresentation>(), rcz_encoder );
  }
  else
  {
    return ( ErrCode::ERR );
  }

  return ( ErrCode::OK );
}

/*=============================================================================================================*/

static ErrCode decode_object( CborMapItem *map_item, ResRepresentation &rcz_res_rep );
static ErrCode decode_object_array( CborArrayItem *map_item, ResRepresentation &rcz_res_rep );

static ErrCode decode_cbor_array( CborArrayItem *array_cbor_item, ResRepresentation &rcz_res_rep, std::string& str_key )
{
  auto first_array_item = array_cbor_item->get_item( 0 );

  if( first_array_item == nullptr )
  {
    return ( ErrCode::ERR );
  }

  switch( first_array_item->get_type() )
  {
    case CBOR_TYPE_UINT:
    case CBOR_TYPE_NEGINT:
    {
      std::vector<long> long_array{};
      long_array.reserve( array_cbor_item->get_size() );

      for( auto &l : array_cbor_item->get_item_list() )
      {
        long_array.push_back( (long) static_cast<CborIntItem *>( l )->get_int() );
      }

      rcz_res_rep.add( str_key, std::move( long_array ) );
    }
    break;
    case CBOR_TYPE_STRING:
    {
      std::vector<std::string> string_array{};
      string_array.reserve( array_cbor_item->get_size() );

      for( auto &str : array_cbor_item->get_item_list() )
      {
        auto string_item = static_cast<CborTextStringItem *>( str );
        std::string val_str{ (const char *) string_item->get_data(), string_item->get_length() };

        string_array.push_back( std::move( val_str ) );
      }

      rcz_res_rep.add( str_key, std::move( string_array ) );
    }
    break;

    case CBOR_TYPE_FLOAT_CTRL:
    {
      auto special_item = static_cast<CborSpecialItem *>( first_array_item );

      if( special_item->is_ctrl() )
      {
        if( special_item->is_bool() )
        {
          std::vector<bool> b_array{};
          b_array.reserve( array_cbor_item->get_size() );

          for( auto &loop_item : array_cbor_item->get_item_list() )
          {
            b_array.push_back( static_cast<CborSpecialItem *>( loop_item )->get_bool() );
          }

          rcz_res_rep.add( str_key, std::move( b_array ) );
        }
      }
      else
      {
        std::vector<double> d_array{};
        d_array.reserve( array_cbor_item->get_size() );

        for( auto &loop_item : array_cbor_item->get_item_list() )
        {
          d_array.push_back( static_cast<CborSpecialItem *>( loop_item )->get_float() );
        }

        rcz_res_rep.add( str_key, std::move( d_array ) );
      }
    }
    break;
    case CBOR_TYPE_MAP:
    {
      std::vector<ResRepresentation> obj_array{};
      obj_array.reserve( array_cbor_item->get_size() );

      for( auto &loop_item : array_cbor_item->get_item_list() )
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

      rcz_res_rep.add( str_key, std::move( obj_array ) );
    }
    break;
    default:
    {
    }
    break;
  }

  return ErrCode::OK;
}

static ErrCode decode_object( CborMapItem *map_item, ResRepresentation &rcz_res_rep )
{
  for( auto &loop_cbor_pair_item : map_item->get_data_list() )
  {
    if( ( loop_cbor_pair_item.key == nullptr ) ||
      ( loop_cbor_pair_item.value == nullptr ) ||
      ( loop_cbor_pair_item.key->get_type() != CBOR_TYPE_STRING ) )
    {
      /* skip this item */
      continue;
    }

    auto pcz_key_string_item = static_cast<CborTextStringItem *>( loop_cbor_pair_item.key );

    if( ( pcz_key_string_item->get_length() == 0 ) || ( pcz_key_string_item->get_data() == nullptr ) )
    {
      /* skip this item */
      continue;
    }

    std::string str_key{ (char *) pcz_key_string_item->get_data(), pcz_key_string_item->get_length() };

    auto pcz_val_item = loop_cbor_pair_item.value;

    switch( pcz_val_item->get_type() )
    {
      case CBOR_TYPE_BYTESTRING:
      case CBOR_TYPE_TAG:
      {
      }
      break;
      case CBOR_TYPE_UINT:
      {
        auto l = static_cast<CborIntItem *>( pcz_val_item )->get_int();
        rcz_res_rep.add( str_key, (long) l );
      }
      break;
      case CBOR_TYPE_NEGINT:
      {
        int32_t l = static_cast<CborIntItem *>( pcz_val_item )->get_int();
        rcz_res_rep.add( str_key, (long) l );
      }
      break;
      case CBOR_TYPE_STRING:
      {
        auto string_cbor_item = static_cast<CborTextStringItem *>( pcz_val_item );
        std::string str_value{ (const char *) string_cbor_item->get_data(), string_cbor_item->get_length() };

        rcz_res_rep.add( str_key, str_value );
      }
      break;
      case CBOR_TYPE_ARRAY:
      {
        auto array_cbor_item  = static_cast<CborArrayItem *>( pcz_val_item );
        decode_cbor_array(array_cbor_item, rcz_res_rep, str_key);
      }
      break;
      case CBOR_TYPE_MAP:
      {
        ResRepresentation new_object{};

        if( decode_object( static_cast<CborMapItem *>( pcz_val_item ), new_object ) == ErrCode::OK )
        {
          rcz_res_rep.add( str_key, std::move( new_object ) );
        }
      }
      break;
      case CBOR_TYPE_FLOAT_CTRL:
      {
        auto special_item = static_cast<CborSpecialItem *>( pcz_val_item );

        if( special_item->is_ctrl() )
        {
          if( special_item->is_bool() )
          {
            rcz_res_rep.add( str_key, special_item->get_bool() );
          }
        }
        else
        {
          rcz_res_rep.add( str_key, special_item->get_float() );
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

static ErrCode decode_object_array( CborArrayItem *array_item, ResRepresentation &rcz_res_rep )
{
  std::vector<ResRepresentation> obj_array{};
  obj_array.reserve( array_item->get_size() );

  for( auto &loop_cbor_item : array_item->get_item_list() )
  {
    ResRepresentation new_object{};

    if( decode_object( static_cast<CborMapItem *>( loop_cbor_item ), new_object ) == ErrCode::OK )
    {
      obj_array.push_back( std::move( new_object ) );
    }
    else
    {
      return ( ErrCode::INVALID_PARAMS );
    }
  }

  rcz_res_rep.add( "", obj_array );

  return ( ErrCode::OK );
}

static ErrCode decode_representation( CborItem *root_map_item, ResRepresentation &properties )
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

/**
 * This api should be called to encode the resource representation.
 *
 * @param rcz_res_rep          - object to encode
 * @param rpu8_dst_buffer      - encoded buffer
 * @param ru16_dst_buffer_len  - encoded buffer length
 * @return
 */
ErrCode CborCodec::encode( ResRepresentation &rcz_res_rep, uint8_t * &rpu8_dst_buffer, uint16_t &ru16_dst_buffer_len )
{
#if 0
  /* initialize the buffers */
  ru16_dst_buffer_len = 0;

#if defined ( _OS_FREERTOS_ )
  static uint8_t buffer[1024];
  rpu8_dst_buffer = &buffer[0];
#else
  rpu8_dst_buffer = new uint8_t[1024];

  if( rpu8_dst_buffer == nullptr )
  {
    return ( ErrCode::ERR );
  }

#endif

  CborEncoderBuffer cz_encode_buffer{ rpu8_dst_buffer, 1024 };
  CborEncoder cz_encoder{ &cz_encode_buffer };

  encode_representation( rcz_res_rep, cz_encoder );

  ru16_dst_buffer_len = cz_encode_buffer.size();
#endif

  CborEncoder cz_encoder{1024};
  encode_representation( rcz_res_rep, cz_encoder );
  rpu8_dst_buffer = cz_encoder.get_buf();
  ru16_dst_buffer_len = cz_encoder.get_buf_len();

  return ( ErrCode::OK );

  #if 0

  CborEncoderBuffer cz_encode_buffer{ &buffer[0], 1024 };
  CborEncoder cz_encoder{ &cz_encode_buffer };

  encode_representation( rcz_res_rep, cz_encoder );

  if( cz_encode_buffer.size() > 0 )
  {
    rpu8_dst_buffer = new uint8_t[cz_encode_buffer.size()];
    memcpy( rpu8_dst_buffer, cz_encode_buffer.data(), cz_encode_buffer.size() );
    ru16_dst_buffer_len = cz_encode_buffer.size();

    return ( ErrCode::OK );
  }

  return ( ErrCode::ERR );
#endif
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

#if 0
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
      {
      }
      break;
    }

    if( cbor_value != nullptr )
    {
      static_cast<CborMapItem *>( root_map )->add_data( { string_key, cbor_value } );
    }
  }

  return ( 0 );
}
#endif

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
