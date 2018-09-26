#ifdef CBOR_PRETTY_PRINTER

#include "CborPrettyPrinter.h"
#include <inttypes.h>
#include "CborItem.h"
#include "CborItemBuilder.h"
#include "CborStreaming.h"

static int _pow( int b, int ex )
{
  if( ex == 0 )
  {
    return ( 1 );
  }

  int res = b;

  while( --ex > 0 )
  {
    res *= b;
  }

  return ( res );
}

static void _cbor_nested_describe( CborItem *item, FILE *out, int indent )
{
  setlocale( LC_ALL, "" );

  switch( item->get_type())
  {
    case CBOR_TYPE_UINT:
    {
      auto int_item = (CborIntItem *) item;
			printf("%" PRIu64 "", int_item->get_int());
      break;
    };
    case CBOR_TYPE_NEGINT:
    {
      auto int_item = (CborIntItem *) item;
			printf("%" PRIi64 "", (int64_t)int_item->get_int());
      break;
    };
    case CBOR_TYPE_BYTESTRING:
    {
      auto bytestring_item = (CborByteStringItem *) item;
			printf("%*s[BYTE_STRING] len[%d]\n", indent, " ", bytestring_item->get_length());
      break;
    };
    case CBOR_TYPE_STRING:
    {
      auto textstring_item = (CborTextStringItem *) item;
			std::string text{ (const char*)textstring_item->get_data(), textstring_item->get_length() };
			printf("\"%s\"", text.c_str());
      break;
    };
    case CBOR_TYPE_ARRAY:
    {
      auto array_item = (CborArrayItem *) item;
			printf("\n%*s[\n", indent, " ");

      for( auto &loop_item : array_item->get_item_list() )
      {
				printf("%*s", indent + 4, " ");
        _cbor_nested_describe( loop_item, out, indent + 4 );
				printf("%*s\n", indent, " ");
      }
			printf("%*s]\n", indent, " ");

      break;
    };
    case CBOR_TYPE_MAP:
    {
      auto map_item = (CborMapItem *) item;
			printf("\n%*s{\n", indent, " ");

      for( auto &loop_item : map_item->get_data_list() )
      {
				printf("%*s", indent + 4, " ");
        _cbor_nested_describe( loop_item.key, out, indent + 4 );
				printf(" : ");
        _cbor_nested_describe( loop_item.value, out, indent + 4 );
				printf("%*s\n", indent, " ");
      }
			printf("%*s}\n", indent, " ");
      break;
    };
    case CBOR_TYPE_TAG:
    {
      auto t_item = (CborTaggedItem *) item;
			printf("%*s[TAG] val[%" PRIu64 "]\n", indent, " ", t_item->get_value());
      _cbor_nested_describe( t_item->get_tagged_item(), out, indent + 4 );
      break;
    };
    case CBOR_TYPE_FLOAT_CTRL:
    {
      auto float_ctrl_item = (CborSpecialItem *) item;

      if( float_ctrl_item->is_ctrl() )
      {
				auto ctrl_value = float_ctrl_item->get_ctrl();

				if (ctrl_value == CBOR_CTRL_TRUE || ctrl_value == CBOR_CTRL_FALSE)
        {
					printf("%s", float_ctrl_item->get_ctrl() == CBOR_CTRL_TRUE ? "true" : "false");
        }
        else if(ctrl_value == CBOR_CTRL_UNDEF)
        {
          fprintf( out, "Undefined\n" );
        }
        else if(ctrl_value == CBOR_CTRL_NULL)
        {
          fprintf( out, "Null\n" );
        }
        else
        {
          fprintf( out, "Simple value %d\n", float_ctrl_item->get_ctrl() );
        }
      }
      else
      {
				printf("%lf", float_ctrl_item->get_float());
      }

      break;
    };
  }
}

void cbor_describe( CborItem *item, FILE *out )
{
  _cbor_nested_describe( item, out, 0 );
}

#endif