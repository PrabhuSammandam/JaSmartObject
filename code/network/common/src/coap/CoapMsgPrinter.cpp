#include <coap/coap_consts.h>
#include <coap/coap_option.h>
#include <coap/coap_options_set.h>
#include <coap/CoapMsgPrinter.h>
#include <coap/coap_msg.h>
#include <cstdint>
#include <stdio.h>
#include <ip_addr.h>

using namespace ja_iot::base;
using namespace ja_iot::network;

namespace ja_iot {
namespace network {
const char *MSG_TYPE_STRING_CON = "CON";
const char *MSG_TYPE_STRING_NON = "NON";
const char *MSG_TYPE_STRING_ACK = "ACK";
const char *MSG_TYPE_STRING_RST = "RST";

const char *MSG_CODE_STRING_EMPTY                = "EMPTY:0";
const char *MSG_CODE_STRING_GET                  = "GET:1";
const char *MSG_CODE_STRING_POST                 = "POST:2";
const char *MSG_CODE_STRING_PUT                  = "PUT:3";
const char *MSG_CODE_STRING_DEL                  = "DEL:4";
const char *MSG_CODE_STRING_CREATED              = "CREATED:2.01";
const char *MSG_CODE_STRING_DELETED              = "DELETED:2.02";
const char *MSG_CODE_STRING_VALID                = "VALID:2.03";
const char *MSG_CODE_STRING_CHANGED              = "CHANGED:2.04";
const char *MSG_CODE_STRING_CONTENT              = "CONTENT:2.05";
const char *MSG_CODE_STRING_CONTINUE             = "CONTINUE:2.31";
const char *MSG_CODE_STRING_BAD_REQ              = "BAD_REQ:4.00";
const char *MSG_CODE_STRING_UNAUTHORIZED         = "UNAUTHORIZED:4.01";
const char *MSG_CODE_STRING_BAD_OPT              = "BAD_OPT:4.02";
const char *MSG_CODE_STRING_FORBIDDEN            = "FORBIDDEN:4.03";
const char *MSG_CODE_STRING_NOT_FOUND            = "NOT_FOUND:4.04";
const char *MSG_CODE_STRING_METHOD_NOT_ALLOWED   = "METHOD_NOT_ALLOWED:4.05";
const char *MSG_CODE_STRING_NOT_ACCEPTABLE       = "NOT_ACCEPTABLE:4.06";
const char *MSG_CODE_STRING_REQ_ENTITY_INCOM     = "REQ_ENTITY_INCOM:4.08";
const char *MSG_CODE_STRING_PRECOND_FAILED       = "PRECOND_FAILED:4.12";
const char *MSG_CODE_STRING_REQ_ENTITY_LARGE     = "REQ_ENTITY_LARGE:4.13";
const char *MSG_CODE_STRING_UNSUP_CONTENT_FORMAT = "UNSUP_CONTENT_FORMAT:4.15";
const char *MSG_CODE_STRING_SERVER_ERROR         = "SERVER_ERROR:5.00";
const char *MSG_CODE_STRING_NOT_IMPL             = "NOT_IMPL:5.01";
const char *MSG_CODE_STRING_BAD_GATEWAY          = "BAD_GATEWAY:5.02";
const char *MSG_CODE_STRING_SERVICE_UNAVAIL      = "SERVICE_UNAVAIL:5.03";
const char *MSG_CODE_STRING_GATEWAY_TIMEOUT      = "GATEWAY_TIMEOUT:5.04";
const char *MSG_CODE_STRING_PROXY_NOT_SUPP       = "PROXY_NOT_SUPP:5.05";


static const char* get_msg_type_string( uint8_t msg_type )
{
  switch( msg_type )
  {
    case COAP_MSG_TYPE_CON: return ( MSG_TYPE_STRING_CON );
    case COAP_MSG_TYPE_NON: return ( MSG_TYPE_STRING_NON );
    case COAP_MSG_TYPE_ACK: return ( MSG_TYPE_STRING_ACK );
    case COAP_MSG_TYPE_RST: return ( MSG_TYPE_STRING_RST );
  }

  return ( "" );
}

static const char* get_msg_code_string( uint8_t msg_code )
{
  switch( msg_code )
  {
    case COAP_MSG_CODE_EMPTY: return ( MSG_CODE_STRING_EMPTY );
    case COAP_MSG_CODE_GET: return ( MSG_CODE_STRING_GET );
    case COAP_MSG_CODE_POST: return ( MSG_CODE_STRING_POST );
    case COAP_MSG_CODE_PUT: return ( MSG_CODE_STRING_PUT );
    case COAP_MSG_CODE_DEL: return ( MSG_CODE_STRING_DEL );
    case COAP_MSG_CODE_CREATED_201: return ( MSG_CODE_STRING_CREATED );
    case COAP_MSG_CODE_DELETED_202: return ( MSG_CODE_STRING_DELETED );
    case COAP_MSG_CODE_VALID_203: return ( MSG_CODE_STRING_VALID );
    case COAP_MSG_CODE_CHANGED_204: return ( MSG_CODE_STRING_CHANGED );
    case COAP_MSG_CODE_CONTENT_205: return ( MSG_CODE_STRING_CONTENT );
    case COAP_MSG_CODE_CONTINUE_231: return ( MSG_CODE_STRING_CONTINUE );
    case COAP_MSG_CODE_BAD_REQUEST_400: return ( MSG_CODE_STRING_BAD_REQ );
    case COAP_MSG_CODE_UNAUTHORIZED_401: return ( MSG_CODE_STRING_UNAUTHORIZED );
    case COAP_MSG_CODE_BAD_OPTION_402: return ( MSG_CODE_STRING_BAD_OPT );
    case COAP_MSG_CODE_FORBIDDEN_403: return ( MSG_CODE_STRING_FORBIDDEN );
    case COAP_MSG_CODE_NOT_FOUND_404: return ( MSG_CODE_STRING_NOT_FOUND );
    case COAP_MSG_CODE_METHOD_NOT_ALLOWED_405: return ( MSG_CODE_STRING_METHOD_NOT_ALLOWED );
    case COAP_MSG_CODE_NOT_ACCEPTABLE_406: return ( MSG_CODE_STRING_NOT_ACCEPTABLE );
    case COAP_MSG_CODE_REQUEST_ENTITY_INCOMPLETE_408: return ( MSG_CODE_STRING_REQ_ENTITY_INCOM );
    case COAP_MSG_CODE_PRECONDITION_FAILED_412: return ( MSG_CODE_STRING_PRECOND_FAILED );
    case COAP_MSG_CODE_REQUEST_ENTITY_TOO_LARGE_413: return ( MSG_CODE_STRING_REQ_ENTITY_LARGE );
    case COAP_MSG_CODE_UNSUPPORTED_CONTENT_FORMAT_415: return ( MSG_CODE_STRING_UNSUP_CONTENT_FORMAT );
    case COAP_MSG_CODE_INTERNAL_SERVER_ERROR_500: return ( MSG_CODE_STRING_SERVER_ERROR );
    case COAP_MSG_CODE_NOT_IMPLEMENTED_501: return ( MSG_CODE_STRING_NOT_IMPL );
    case COAP_MSG_CODE_BAD_GATEWAY_502: return ( MSG_CODE_STRING_BAD_GATEWAY );
    case COAP_MSG_CODE_SERVICE_UNAVAILABLE_503: return ( MSG_CODE_STRING_SERVICE_UNAVAIL );
    case COAP_MSG_CODE_GATEWAY_TIMEOUT_504: return ( MSG_CODE_STRING_GATEWAY_TIMEOUT );
    case COAP_MSG_CODE_PROXYING_NOT_SUPPORTED_505: return ( MSG_CODE_STRING_PROXY_NOT_SUPP );
  }

  return ( "" );
}

void CoapMsgPrinter::print_coap_msg( ja_iot::network::CoapMsg &coap_msg, uint8_t direction )
{
  printf( "%s ", direction == 0 ? "[INP]" : "[OUT]" );

  uint8_t ascii_addr[70] = { 0 };
  coap_msg.get_endpoint().get_addr().to_string( &ascii_addr[0], 70 );

  printf( "[%s]", &ascii_addr[0] );

  auto pc_type_str = get_msg_type_string( coap_msg.get_type() );
  auto pc_code_str = get_msg_code_string( coap_msg.get_code() );

  if( ( pc_type_str != nullptr ) && ( pc_code_str != nullptr ) )
  {
    printf( "t:%s c:%s i:0x%04X t:", pc_type_str, pc_code_str, coap_msg.get_id() );

    auto &token = coap_msg.get_token();

    for( int i = 0; i < token.get_length(); i++ )
    {
      uint8_t byte = token.get_token()[i];
      printf( "%02X", byte );
    }

    printf( " " );
  }

  auto        options_list = coap_msg.get_option_set().get_sorted_options_list();
  static char buffer[256];

  for( auto &option : options_list )
  {
    coap_msg.get_option_set().get_option_value_as_string( (Option &) option, buffer, 256 );
    printf( "%s:%s ", CoapOptionsSet::get_option_no_string( option.get_no() ), &buffer[0] );
    buffer[0] = '\0';
  }

  printf( "\n" );
}
}
}
