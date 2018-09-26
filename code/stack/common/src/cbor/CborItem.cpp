#include "cbor/CborItem.h"
#include "cbor/CborEncoding.h"

CborItem & CborItem::operator = ( CborItem &other )
{
  if( this != &other )
  {
    this->_type       = other._type;
    this->_is_stream = other._is_stream;
  }

  return ( *this );
}
CborIntItem::CborIntItem( const CborIntItem &other ) : CborItem{}
{
  *this = other;
}

CborIntItem & CborIntItem::operator = ( const CborIntItem &other )
{
  if( this != &other )
  {
    this->_width = other._width;
    this->_value = other._value;
    this->_type   = other._type;
  }

  return ( *this );
}

size_t CborIntItem::encode( uint8_t *buffer, size_t buffer_len )
{
  if( get_type() == CBOR_TYPE_UINT )
  {
    return ( cbor_encode_uint( _value, buffer, buffer_len ) );
  }
  else
  {
    return ( cbor_encode_negint( _value, buffer, buffer_len ) );
  }
}

/****************************************************************************************************/
/****************************************************************************************************/
/****************************************************************************************************/
CborByteStringItem::CborByteStringItem( bool is_stream ) : CborItem( CBOR_TYPE_BYTESTRING, is_stream )
{
}
CborByteStringItem::CborByteStringItem( CborByteStringItem &other )
{
  *this = other;
}

CborByteStringItem::~CborByteStringItem()
{
	if (is_stream())
	{
		for (auto &loop_item : _items)
		{
			delete loop_item;
		}
		_items.clear();
	}
	else
	{
		if (this->data != nullptr)
		{
			delete[] this->data;
		}
		this->data = nullptr;
	}
}

CborByteStringItem & CborByteStringItem::operator = ( CborByteStringItem &other )
{
  if( this != &other )
  {
    CborItem::operator = ( other );

    if( other.is_stream() )
    {
      for( auto &loop_item : other.get_chunks_list() )
      {
        auto new_item = new CborByteStringItem( *( (CborByteStringItem *) loop_item ) );
        add_chunk( new_item );
      }
    }
    else
    {
      if( this->data != nullptr )
      {
        delete[] this->data;
      }

      add_data( other.get_data(), other.get_length() );
    }
  }

  return ( *this );
}

size_t CborByteStringItem::encode(uint8_t * buffer, size_t buffer_size)
{
	if (!is_stream())
	{
		size_t length = get_length();
		size_t written = cbor_encode_bytestring_start(length, buffer, buffer_size);

		if (written && (buffer_size - written >= length))
		{
			memcpy(buffer + written, get_data(), length);
			return (written + length);
		}
		else
		{
			return (0);
		}
	}
	else
	{
		size_t written = cbor_encode_indef_bytestring_start(buffer, buffer_size);

		if (written == 0)
		{
			return (0);
		}

		for (auto& loop_item : get_chunks_list())
		{
			size_t chunk_written = loop_item->encode(buffer + written, buffer_size - written);

			if (chunk_written == 0)
			{
				return (0);
			}
			else
			{
				written += chunk_written;
			}
		}

		if (cbor_encode_break(buffer + written, buffer_size - written) > 0)
		{
			return (written + 1);
		}
		else
		{
			return (0);
		}
	}
}

bool CborByteStringItem::add_chunk( CborItem *chunk )
{
  _items.push_back( chunk );
  return ( true );
}

void CborByteStringItem::set_data( uint8_t *data, size_t length )
{
  this->data    = data;
  this->_length = length;
}

bool CborByteStringItem::add_data( uint8_t *data, size_t length )
{
  auto new_handle = new uint8_t[length];

  memcpy( new_handle, data, length );
  set_data( new_handle, length );
  return ( true );
}
/****************************************************************************************************/
/****************************************************************************************************/
/****************************************************************************************************/
CborTextStringItem::CborTextStringItem( bool is_stream ) : CborItem( CBOR_TYPE_STRING, is_stream )
{
}
CborTextStringItem::CborTextStringItem( const char *val ) : CborTextStringItem{ (const char *) val, strlen( val ) }
{
}
CborTextStringItem::CborTextStringItem( const char *val, size_t length ) : CborItem( CBOR_TYPE_STRING )
{
  if( val != nullptr )
  {
    add_data( (uint8_t *) val, length );
  }
}

CborTextStringItem::~CborTextStringItem()
{
	if (is_stream())
	{
		for (auto &loop_item : _items)
		{
			delete loop_item;
		}
		_items.clear();
	}
	else
	{
		if (this->data != nullptr)
		{
			delete[] this->data;
		}
		this->data = nullptr;
	}
}

CborTextStringItem & CborTextStringItem::operator = ( CborTextStringItem &other )
{
  if( this != &other )
  {
    this->_codepoint_count = other._codepoint_count;
    CborItem::operator = ( other );

    if( other.is_stream() )
    {
      for( auto &loop_item : other.get_chunk_list() )
      {
        auto new_item = new CborTextStringItem( *( (CborTextStringItem *) loop_item ) );
        add_chunk( new_item );
      }
    }
    else
    {
      if( this->data != nullptr )
      {
        delete[] this->data;
      }

      add_data( other.get_data(), other.get_length() );
    }
  }

  return ( *this );
}

size_t CborTextStringItem::encode(uint8_t * buffer, size_t buffer_size)
{
	if (!is_stream())
	{
		size_t length = get_length();
		size_t written = cbor_encode_string_start(length, buffer, buffer_size);

		if (written && (buffer_size - written >= length))
		{
			memcpy(buffer + written, get_data(), length);
			return (written + length);
		}
		else
		{
			return (0);
		}
	}
	else
	{
		size_t written = cbor_encode_indef_string_start(buffer, buffer_size);

		if (written == 0)
		{
			return (0);
		}

		for (auto& loop_item : get_chunk_list())
		{
			size_t chunk_written = loop_item->encode( buffer + written, buffer_size - written);

			if (chunk_written == 0)
			{
				return (0);
			}
			else
			{
				written += chunk_written;
			}
		}

		if (cbor_encode_break(buffer + written, buffer_size - written) > 0)
		{
			return (written + 1);
		}
		else
		{
			return (0);
		}
	}
}

void CborTextStringItem::set_data( uint8_t *data, size_t length )
{
  this->data    = data;
  this->_length = length;
}

bool CborTextStringItem::add_data( uint8_t *data, size_t length )
{
  uint8_t *handle = new uint8_t[length];

  memcpy( handle, data, length );
  set_data( handle, length );
  return ( true );
}

bool CborTextStringItem::add_chunk( CborItem *chunk )
{
  _items.push_back( chunk );
  return ( true );
}

/****************************************************************************************************/
/****************************************************************************************************/
/****************************************************************************************************/
CborArrayItem::CborArrayItem( bool is_stream ) : CborItem{ CBOR_TYPE_ARRAY, is_stream }
{
}
CborArrayItem::CborArrayItem( size_t size ) : CborItem{ CBOR_TYPE_ARRAY, false }
{
  _items.reserve( size );
}

CborArrayItem::~CborArrayItem()
{
	for (auto &loop_item : _items)
	{
		delete loop_item;
	}
	_items.clear();
}

size_t CborArrayItem::encode(uint8_t * buffer, size_t buffer_size)
{
	size_t        size = get_size();
	size_t        written = 0;

	if (!is_stream())
	{
		written = cbor_encode_array_start(size, buffer, buffer_size);
	}
	else
	{
		written = cbor_encode_indef_array_start(buffer, buffer_size);
	}

	if (written == 0)
	{
		return (0);
	}

	size_t item_written;

	for (auto& loop_item : get_item_list())
	{
		item_written = loop_item->encode( buffer + written, buffer_size - written);

		if (item_written == 0)
		{
			return (0);
		}
		else
		{
			written += item_written;
		}
	}

	if (!is_stream())
	{
		return (written);
	}
	else
	{
		item_written = cbor_encode_break(buffer + written, buffer_size - written);

		if (item_written == 0)
		{
			return (0);
		}
		else
		{
			return (written + 1);
		}
	}
}

bool CborArrayItem::replace_item( size_t index, CborItem *new_item )
{
  _items[index] = new_item;

  return ( true );
}

bool CborArrayItem::add( CborItem *new_item )
{
  _items.push_back( new_item );
  return ( true );
}

/****************************************************************************************************/
/****************************************************************************************************/
/****************************************************************************************************/
CborMapItem::CborMapItem( bool is_stream ) : CborItem{ CBOR_TYPE_MAP, is_stream }
{
}
CborMapItem::CborMapItem( size_t size ) : CborItem{ CBOR_TYPE_MAP }
{
  _items.reserve( size );
}

CborMapItem::~CborMapItem()
{
	for (auto &loop_item : _items)
	{
		delete loop_item.key;
		delete loop_item.value;
	}
	_items.clear();
}

size_t CborMapItem::encode(uint8_t * buffer, size_t buffer_size)
{
	size_t            size = get_size();
	size_t            written = 0;

	if (!is_stream())
	{
		written = cbor_encode_map_start(size, buffer, buffer_size);
	}
	else
	{
		written = cbor_encode_indef_map_start(buffer, buffer_size);
	}

	if (written == 0)
	{
		return (0);
	}

	size_t item_written;

	for (auto& loop_item : get_data_list())
	{
		item_written = loop_item.key->encode(buffer + written, buffer_size - written);

		if (item_written == 0)
		{
			return (0);
		}
		else
		{
			written += item_written;
		}

		item_written = loop_item.value->encode(buffer + written, buffer_size - written);

		if (item_written == 0)
		{
			return (0);
		}
		else
		{
			written += item_written;
		}
	}

	if (!is_stream())
	{
		return (written);
	}
	else
	{
		item_written = cbor_encode_break(buffer + written, buffer_size - written);

		if (item_written == 0)
		{
			return (0);
		}
		else
		{
			return (written + 1);
		}
	}
}

bool CborMapItem::add_key( CborItem *key )
{
  _items.push_back( { key, nullptr } );
  return ( true );
}

bool CborMapItem::add_value( CborItem *value )
{
  _items.back().value = value;
  return ( true );
}

bool CborMapItem::add_data( cbor_pair pair )
{
  _items.push_back( pair );
  return ( true );
}

/****************************************************************************************************/
/****************************************************************************************************/
/****************************************************************************************************/
CborTaggedItem::CborTaggedItem () : CborItem( CBOR_TYPE_TAG )
{
}
CborTaggedItem::CborTaggedItem( uint64_t value ) : CborItem( CBOR_TYPE_TAG ), _tagged_item{nullptr}, _value{ value }
{
}
CborTaggedItem::CborTaggedItem( CborItem *tagged_item, uint64_t value ) : CborItem( CBOR_TYPE_TAG ), _tagged_item{ tagged_item }, _value{ value }
{
}

size_t CborTaggedItem::encode(uint8_t * buffer, size_t buffer_size)
{
	size_t written = cbor_encode_tag(get_value(), buffer, buffer_size);

	if (written == 0)
	{
		return (0);
	}

	size_t item_written = get_tagged_item()->encode(buffer + written, buffer_size - written);

	if (item_written == 0)
	{
		return (0);
	}
	else
	{
		return (written + item_written);
	}
}

void CborTaggedItem::set_tagged_item( CborItem *tagged_item )
{
  _tagged_item = tagged_item;
}


/****************************************************************************************************/
/****************************************************************************************************/
/****************************************************************************************************/
CborSpecialItem::CborSpecialItem () : CborSpecialItem{ CBOR_FLOAT_0 }
{
}
CborSpecialItem::CborSpecialItem( cbor_float_width width ) : CborItem( CBOR_TYPE_FLOAT_CTRL ), _width{ width }
{
}
CborSpecialItem::CborSpecialItem( _cbor_ctrl ctrl_value ) : CborItem( CBOR_TYPE_FLOAT_CTRL ), _width{ CBOR_FLOAT_0 }, _ctrl{ (uint8_t) ctrl_value }
{
}
CborSpecialItem::CborSpecialItem( const CborSpecialItem &other )
{
  *this = other;
}

size_t CborSpecialItem::encode( uint8_t *buffer, size_t buffer_size )
{
  switch( get_width() )
  {
    case CBOR_FLOAT_0:              return ( cbor_encode_ctrl( get_ctrl(), buffer, buffer_size ) );
    case CBOR_FLOAT_16:             return ( cbor_encode_half( get<float>(), buffer, buffer_size ) );
    case CBOR_FLOAT_32:             return ( cbor_encode_single( get<float>(), buffer, buffer_size ) );
    case CBOR_FLOAT_64:             return ( cbor_encode_double( get<double>(), buffer, buffer_size ) );
  }

  /* Should never happen - make the compiler happy */
  return ( 0 );
}

CborSpecialItem & CborSpecialItem::operator = ( const CborSpecialItem &other )
{
  if( this != &other )
  {
    this->_width = other._width;
    this->_ctrl  = other._ctrl;
    this->_value = other._value;
    this->_type   = other._type;
  }

  return ( *this );
}
