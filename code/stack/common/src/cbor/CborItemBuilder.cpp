#include "common/inc/cbor/CborItem.h"
#include "common/inc/cbor/CborItemBuilder.h"
#include "common/inc/cbor/CborStreaming.h"

CborItem * CborItemBuilder::Load( cbor_data source, size_t source_size, cbor_load_result *result )
{
  if( source_size == 0 )
  {
    result->error.code = CBOR_ERR_NODATA;
    return ( nullptr );
  }

  /* Target for callbacks */
  CborItemBuilder     item_builder;

  cbor_decoder_result decode_result;
  result->read       = 0;
  result->error.code = CBOR_ERR_NONE;

  do
  {
    if( source_size > result->read )                         /* Check for overflows */
    {
      auto remaining_data     = (uint8_t *) ( source + result->read );
      auto remaining_data_len = source_size - result->read;

      decode_result = cbor_stream_decode( remaining_data, remaining_data_len, item_builder );
    }
    else
    {
      result->error.code     = CBOR_ERR_NOTENOUGHDATA;
      result->error.position = result->read;
      goto error;
    }

    switch( decode_result.status )
    {
      case CBOR_DECODER_FINISHED:
        /* Everything OK */
      {
        result->read += decode_result.read;
        break;
      }
      case CBOR_DECODER_NEDATA:
        /* Data length doesn't match MTB expectation */
      {
        result->error.code = CBOR_ERR_NOTENOUGHDATA;
        goto error;
      }
      case CBOR_DECODER_EBUFFER:
      /* Fallthrough */
      case CBOR_DECODER_ERROR:
        /* Reserved/malformated item */
      {
        result->error.code = CBOR_ERR_MALFORMATED;
        goto error;
      }
    }

    if( item_builder.is_creation_failed() )
    {
      /* Most likely unsuccessful allocation - our callback has failed */
      result->error.code = CBOR_ERR_MEMERROR;
      goto error;
    }
    else if( item_builder.is_syntax_error() )
    {
      result->error.code = CBOR_ERR_SYNTAXERROR;
      goto error;
    }
  } while( item_builder.get_item_count() != 0 );

  /* Move the result before free */
  return ( item_builder.get_root_item() );

error:
  result->error.position = result->read;

  // debug_print("Failed with decoder error %d at %d\n", result->error.code, result->error.position);
  // cbor_describe(stack.top->item, stdout);
  /* Free the stack */
  // item_builder.clear();

  return ( nullptr );
}

void CborItemBuilder::append( CborItem *pcz_cbor_item )
{
  if( _stack.empty() )
  {
    _root_item = pcz_cbor_item;
    return;
  }

  switch( _stack.top().get_item_type() )
  {
    case CBOR_TYPE_ARRAY:
    {
      auto array_item = (CborArrayItem *) _stack.top().get_item();
      array_item->add( pcz_cbor_item );

      if( !array_item->is_stream() )
      {
        _stack.top().dec_subitem_count();

        if( _stack.top().is_subitems_empty() )
        {
          _stack.pop();
          append( array_item );
        }
      }
    }
    break;
    case CBOR_TYPE_MAP:
    {
      auto &top_item_info = _stack.top();
      auto map_item       = (CborMapItem *) top_item_info.get_item();

      /* We use 0 and 1 subitems to distinguish between keys and values in indefinite items */
      if( top_item_info.get_subitem_count() % 2 )
      {
        /* Odd record, this is a value */
        map_item->add_value( pcz_cbor_item );
      }
      else
      {
        /* Even record, this is a key */
        map_item->add_key( pcz_cbor_item );
      }

      if( !map_item->is_stream() )
      {
        top_item_info.dec_subitem_count();

        if( top_item_info.is_subitems_empty() )
        {
          _stack.pop();
          append( map_item );
        }
      }
      else
      {
        top_item_info.set_subitem_count( top_item_info.get_subitem_count() ^ 1 );                                    /* Flip the indicator for indefinite items */
      }
    }
    break;
    case CBOR_TYPE_TAG:
    {
      auto tag_item = (CborTaggedItem *) _stack.top().get_item();
      tag_item->set_tagged_item( pcz_cbor_item );
      _stack.pop();
      append( tag_item );
      break;
    }
    default:
    {
      _syntax_error = true;
    }
  }
}

void CborItemBuilder::on_uint8( uint8_t u8_value )
{
  auto int_item = new CborIntItem{ u8_value };

  append( int_item );
}

void CborItemBuilder::on_uint16( uint16_t u16_value )
{
  auto int_item = new CborIntItem{ u16_value };

  append( int_item );
}

void CborItemBuilder::on_uint32( uint32_t u32_value )
{
  auto int_item = new CborIntItem{ u32_value };

  append( int_item );
}

void CborItemBuilder::on_uint64( uint64_t u64_value )
{
  auto int_item = new CborIntItem{ u64_value };

  append( int_item );
}

void CborItemBuilder::on_int8( int8_t i8_value )
{
  auto int_item = new CborIntItem{ i8_value };

  append( int_item );
}

void CborItemBuilder::on_int16( int16_t i16_value )
{
  auto int_item = new CborIntItem{ i16_value };

  append( int_item );
}

void CborItemBuilder::on_int32( int32_t i32_value )
{
  auto int_item = new CborIntItem{ i32_value };

  append( int_item );
}

void CborItemBuilder::on_int64( int64_t i64_value )
{
  auto int_item = new CborIntItem{ i64_value };

  append( int_item );
}

void CborItemBuilder::on_float16( float i16_value )
{
  auto float_item = new CborSpecialItem{ i16_value };

  float_item->_width = CBOR_FLOAT_16;
  append( float_item );
}

void CborItemBuilder::on_float32( float i32_value )
{
  auto float_item = new CborSpecialItem{ i32_value };

  append( float_item );
}

void CborItemBuilder::on_float64( double i64_value )
{
  auto float_item = new CborSpecialItem{ i64_value };

  append( float_item );
}

void CborItemBuilder::on_indefinite_byte_string()
{
  auto byte_string_item = new CborByteStringItem{ true };

  push( byte_string_item );
}

void CborItemBuilder::on_definite_byte_string( uint8_t *data, size_t size )
{
  auto byte_string_item = new CborByteStringItem{ false };

  byte_string_item->add_data( data, size );

  auto top_item = _stack.top().get_item();

  if( ( top_item != nullptr ) && ( top_item->get_type() == CBOR_TYPE_BYTESTRING ) )
  {
    if( top_item->is_stream() )
    {
      static_cast<CborByteStringItem *>( top_item )->add_chunk( byte_string_item );
    }
    else
    {
      _syntax_error = true;
    }
  }
  else
  {
    append( byte_string_item );
  }
}

void CborItemBuilder::on_indefinite_text_string()
{
  auto text_string_item = new CborTextStringItem{ true };

  push( text_string_item );
}

void CborItemBuilder::on_definite_text_string( uint8_t *data, size_t size )
{
  auto text_string_item = new CborTextStringItem{ (const char *) data, size };

  auto top_item = _stack.top().get_item();

  if( ( top_item != nullptr ) && ( top_item->get_type() == CBOR_TYPE_STRING ) )
  {
    if( top_item->is_stream() )
    {
      static_cast<CborByteStringItem *>( top_item )->add_chunk( text_string_item );
    }
    else
    {
      _syntax_error = true;
    }
  }
  else
  {
    append( text_string_item );
  }
}

void CborItemBuilder::on_indefinite_array()
{
  auto array_item = new CborArrayItem{ true };

  push( array_item );
}

void CborItemBuilder::on_definite_array( uint32_t size )
{
  auto array_item = new CborArrayItem{ (size_t) size };

  if( size > 0 )
  {
    push( array_item, size );
  }
  else
  {
    append( array_item );
  }
}

void CborItemBuilder::on_indefinite_map()
{
  auto map_item = new CborMapItem{ true };

  push( map_item );
}

void CborItemBuilder::on_definite_map( uint32_t size )
{
  auto map_item = new CborMapItem{ (size_t) size };

  if( size > 0 )
  {
    push( map_item, size * 2 );
  }
  else
  {
    append( map_item );
  }
}

void CborItemBuilder::on_tag( uint64_t tag )
{
  auto tagget_item = new CborTaggedItem{ tag };

  push( tagget_item, 1 );
}

void CborItemBuilder::on_bool( bool value )
{
  auto float_item = new CborSpecialItem{ value ? CBOR_CTRL_TRUE : CBOR_CTRL_FALSE };

  append( float_item );
}

void CborItemBuilder::on_null()
{
  auto float_item = new CborSpecialItem{ CBOR_CTRL_NULL };

  append( float_item );
}

void CborItemBuilder::on_undefined()
{
  auto float_item = new CborSpecialItem{ CBOR_CTRL_UNDEF };

  append( float_item );
}

void CborItemBuilder::on_indefinite_break()
{
  if( !_stack.empty() )
  {
    auto item = _stack.top().get_item();
    _stack.pop();
    append( item );
  }
}

void CborItemBuilder::on_error( const char *error )
{
}
