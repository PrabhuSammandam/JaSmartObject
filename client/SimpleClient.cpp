#include <stdio.h>
#include <iostream>

#include <regex>
#include <tclap/CmdLine.h>
#include "Client.h"

#define DBG_INFO( format, ... ) printf( format "\n", ## __VA_ARGS__ )
#define DBG_WARN( format, ... ) printf( format "\n", ## __VA_ARGS__ )
#define DBG_ERROR( format, ... ) printf( format "\n", ## __VA_ARGS__ )
#define DBG_FATAL( format, ... ) printf( format "\n", ## __VA_ARGS__ )

#include "IMemAllocator.h"
#include "OsalMgr.h"
#include "i_nwk_platform_factory.h"
#include "config_mgr.h"
#include "adapter_mgr.h"
#include "MsgStack.h"
#include "ip_addr.h"
#include "base_utils.h"
#include "ip_addr.h"
#include "ResPropValue.h"
#include "CborCodec.h"

using namespace std;
using namespace ja_iot::base;
using namespace ja_iot::memory;
using namespace ja_iot::network;
using namespace ja_iot::osal;
using namespace ja_iot::stack;
using namespace TCLAP;

class CommandLineArgs
{
  public:
    bool           is_non           = false;
    bool           is_observe       = false;
    uint16_t       transport_scheme = 0;
    uint16_t       network_flag     = 0;
    IpAddress      ip_addr;
    uint16_t       port        = 0;
    uint16_t       block2_size = 0;
    std::string    path;
    std::string    query;
    uint8_t        method;
    CoapMsgToken   token;
    uint16_t       accept_format;
    std::string    payload;
};

void init_adapter_mgr()
{
  const auto mem_allocator = MemAllocatorFactory::create_set_mem_allocator( MemAlloctorType::kLinux );

  if( mem_allocator == nullptr )
  {
    DBG_ERROR( "main:%d# Failed to allocate the mem allocator", __LINE__ );
    return;
  }

  OsalMgr::Inst()->Init();

  const auto platform_factory = INetworkPlatformFactory::create_set_factory( NetworkPlatform::kLinux );

  if( platform_factory == nullptr )
  {
    DBG_ERROR( "main:%d# INetworkPlatformFactory NULL for WINDOWS platform", __LINE__ );
    return;
  }

  auto ip_adapter_config = ConfigManager::Inst().get_ip_adapter_config();

  // ip_adapter_config->set_port( IP_ADAPTER_CONFIG_IPV4_UCAST, 56775 );

  ip_adapter_config->set_flag( IP_ADAPTER_CONFIG_IPV4_UCAST, true );
  // ip_adapter_config->set_flag( IP_ADAPTER_CONFIG_IPV4_MCAST, true );
  // ip_adapter_config->set_flag(IP_ADAPTER_CONFIG_IPV4_UCAST_SECURE, true);
  // ip_adapter_config->set_flag(IP_ADAPTER_CONFIG_IPV4_MCAST_SECURE, true);

  ip_adapter_config->set_flag( IP_ADAPTER_CONFIG_IPV6_UCAST, true );
  // ip_adapter_config->set_flag( IP_ADAPTER_CONFIG_IPV6_MCAST, true );
  // ip_adapter_config->set_flag(IP_ADAPTER_CONFIG_IPV6_UCAST_SECURE, true);
  // ip_adapter_config->set_flag(IP_ADAPTER_CONFIG_IPV6_MCAST_SECURE, true);
}

uint8_t get_method_code_from_string( std::string &method_code_string )
{
  if( method_code_string == "GET" )
  {
    return ( COAP_MSG_CODE_GET );
  }
  else if( method_code_string == "PUT" )
  {
    return ( COAP_MSG_CODE_PUT );
  }
  else if( method_code_string == "POST" )
  {
    return ( COAP_MSG_CODE_POST );
  }
  else if( method_code_string == "DELETE" )
  {
    return ( COAP_MSG_CODE_DEL );
  }

  return ( COAP_MSG_CODE_EMPTY );
}

uint8_t get_accept_format_from_string( std::string &accept_format_string )
{
  if( accept_format_string == "plain" )
  {
    return ( COAP_CONTENT_FORMAT_PLAIN );
  }
  else if( accept_format_string == "link" )
  {
    return ( COAP_CONTENT_FORMAT_LINK_FORMAT );
  }
  else if( accept_format_string == "xml" )
  {
    return ( COAP_CONTENT_FORMAT_XML );
  }
  else if( accept_format_string == "json" )
  {
    return ( COAP_CONTENT_FORMAT_JSON );
  }
  else if( accept_format_string == "cbor" )
  {
    return ( COAP_CONTENT_FORMAT_CBOR );
  }

  return ( COAP_CONTENT_FORMAT_PLAIN );
}


#define CHECK_STRING_POS( pos, msg ) if( pos == std::string::npos ){ cout << msg << endl; return false; }
bool parse_scheme( std::string &endpoint_string, std::string &ip_addr_string, CommandLineArgs &commands );

bool parse_endpoint( std::string &endpoint_string, CommandLineArgs &commands )
{
  std::string ip_addr_string;

  if( !parse_scheme( endpoint_string, ip_addr_string, commands ) )
  {
    cout << "Scheme parsing failed" << endl;
    return ( false );
  }

  if( is_bit_set( commands.network_flag, k_network_flag_ipv6 ) )
  {
    auto bracket_pos = ip_addr_string.find( ']', 1 );
    CHECK_STRING_POS( bracket_pos, "IPV6 addr not valid" );
    auto ipv6_addr_string = ip_addr_string.substr( 1, bracket_pos - 1 );

    if( !IpAddress::from_string( ipv6_addr_string.c_str(), IpAddrFamily::IPv6, commands.ip_addr ) )
    {
      cout << "IPV6 addr not valid" << endl;
      return ( false );
    }

    if( ip_addr_string[bracket_pos + 1] != ':' )
    {
      cout << "PORT not specified" << endl;
      return ( false );
    }

    auto path_pos = ip_addr_string.find( '/', bracket_pos + 1 );
    CHECK_STRING_POS( path_pos, "PATH not specified" );

    auto port_string = ip_addr_string.substr( bracket_pos + 2, path_pos - bracket_pos - 2 );
    commands.path = std::string{ "/" };

    if( path_pos != ip_addr_string.size() )
    {
      commands.path = ip_addr_string.substr( path_pos );
    }

    commands.port = std::stoi( port_string );

    IpAddress ip_addr{ IpAddrFamily::IPv6 };

    IpAddress::from_string( ipv6_addr_string.c_str(), IpAddrFamily::IPv6, ip_addr );

    cout << "ip : " << ipv6_addr_string << " port : " << port_string << " path : " << commands.path << endl;
  }
  else
  {
    auto colon_pos = ip_addr_string.find( ':' );
    CHECK_STRING_POS( colon_pos, "PORT not specified" );
    auto ip_string = ip_addr_string.substr( 0, colon_pos );

    if( !IpAddress::from_string( ip_string.c_str(), IpAddrFamily::IPv4, commands.ip_addr ) )
    {
      cout << "IP addr not valid" << endl;
      return ( false );
    }

    auto path_pos = ip_addr_string.find( '/', colon_pos + 1 );
    CHECK_STRING_POS( path_pos, "PATH not specified" );

    auto port_string = ip_addr_string.substr( colon_pos + 1, path_pos - colon_pos - 1 );
    commands.path = std::string{ "/" };

    if( path_pos != ip_addr_string.size() )
    {
      commands.path = ip_addr_string.substr( path_pos );
    }

    commands.port = std::stoi( port_string );

    IpAddress ip_addr{};

    IpAddress::from_string( ip_string.c_str(), IpAddrFamily::IPv4, ip_addr );

    cout << "ip : " << ip_string << " port : " << port_string << " path : " << commands.path << endl;
  }

  return ( true );
}


bool parse( std::string &endpoint_string, CommandLineArgs &commands )
{
  std::string regex_pattern( "(.*)://(.*):(\\d+)(/.*)" );
  std::cmatch m;
  std::regex  scheme_ipv4( regex_pattern );

  /* check whether passed string is in valid URI format ie coap://XXX.XXX.XXX.XXX:<port>/<resource path> for IPV4 */
  if( std::regex_match( endpoint_string.c_str(), m, scheme_ipv4 ) )
  {
    /* check for the first group in the reg exp string */
    auto scheme = m.str( 1 );// get the scheme

    if( !scheme.empty() )
    {
      if( scheme == "coap" )
      {
        commands.transport_scheme |= k_transport_scheme_coap;
      }
      else if( scheme == "coaps" )
      {
        commands.network_flag     |= ( k_network_flag_secure );
        commands.transport_scheme |= k_transport_scheme_coaps;
      }
      else
      {
        /* invalid scheme passed ie it is not "coap" or "coaps" */
        return ( false );
      }
    }

    auto ip = m.str( 2 );// get the IP address

    if( !ip.empty() )
    {
      std::cmatch m1;
      std::regex  ip6_regex_pattern( "\\[(.*)\\]" );

      /* check for IPV6 address. */
      if( std::regex_match( ip.c_str(), m1, ip6_regex_pattern ) )
      {
        auto ipv6_addr = m1.str( 1 );// get the IPV6 address
        commands.network_flag |= ( k_network_flag_ipv6 );
        commands.ip_addr.set_addr_family( IpAddrFamily::IPv6 );
        auto success = IpAddress::from_string( ipv6_addr.c_str(), IpAddrFamily::IPv6, commands.ip_addr );

        /* check if it is multicast address */
        if( success )
        {
          if( commands.ip_addr.is_multicast() )
          {
            commands.network_flag |= ( k_network_flag_multicast );
          }
        }
        else
        {
          return ( false );
        }
      }
      else
      {
        commands.network_flag |= ( k_network_flag_ipv4 );
        commands.ip_addr.set_addr_family( IpAddrFamily::IPv4 );
        auto success = IpAddress::from_string( ip.c_str(), IpAddrFamily::IPv4, commands.ip_addr );

        if( success )
        {
          if( commands.ip_addr.is_multicast() )
          {
            commands.network_flag |= ( k_network_flag_multicast );
          }
        }
        else
        {
          return ( false );
        }
      }
    }

    auto port = m.str( 3 );// get the port

    if( !port.empty() )
    {
      commands.port = std::stoi( port );
    }

    auto path_string = m.str( 4 );// get the resource pathl

    std::replace( path_string.begin(), path_string.end(), ';', '&' );

    cout << "Path string " << path_string << endl;

    if( !path_string.empty() )
    {
      const auto start_eq = path_string.find_first_of( '?' );

      if( ( start_eq == std::string::npos ) || ( start_eq == 0 ) || ( start_eq == ( path_string.length() - 1 ) ) )
      {
        commands.path = std::move( path_string );
      }
      else
      {
        commands.path  = path_string.substr( 0, start_eq );
        commands.query = path_string.substr( start_eq + 1 );
      }
    }

    return ( true );
  }

  return ( false );
}

bool parse_scheme( std::string &endpoint_string, std::string &ip_addr_string, CommandLineArgs &commands )
{
  if( endpoint_string.size() == 0 )
  {
    cout << "invalid endpoint" << endl;
    return ( false );
  }

  if( parse( endpoint_string, commands ) )
  {
    return ( true );
  }

  return ( false );
}

bool get_commands( int argc, char *argv[], CommandLineArgs &commands )
{
  try
  {
    CmdLine cmd_line{ "Simple Client", ' ', "0.0" };

    SwitchArg non_msg{ "", "non", "Use NON message type", cmd_line };
    SwitchArg obs{ "", "obs", "Observe the resource", cmd_line };

    std::vector<std::string> method_values{};
    method_values.push_back( "GET" );
    method_values.push_back( "PUT" );
    method_values.push_back( "POST" );
    method_values.push_back( "DELETE" );
    ValuesConstraint<string> method_allowed_values( method_values );


    ValueArg<std::string> method{ "m",
                                  "method",
                                  "Method to send",
                                  true,
                                  "GET",
                                  &method_allowed_values,
                                  cmd_line };

    ValueArg<std::string> token{ "t",
                                 "token",
                                 "Token to be used for communication",
                                 false,
                                 "",
                                 "string",
                                 cmd_line };

    ValueArg<std::string> payload{ "",
                                   "payload",
                                   "Payload to send",
                                   false,
                                   "",
                                   "string",
                                   cmd_line };

    std::vector<uint16_t> block2_values{ 32, 64, 128, 256, 512, 1024 };
    ValuesConstraint<uint16_t> block2_allowed_values( block2_values );

    ValueArg<uint16_t> block2_size{ "",
                                    "b2",
                                    "Response block size",
                                    false,
                                    0,
                                    &block2_allowed_values,
                                    cmd_line };

    std::vector<std::string> accept_format_values{};
    accept_format_values.push_back( "plain" );
    accept_format_values.push_back( "link" );
    accept_format_values.push_back( "xml" );
    accept_format_values.push_back( "json" );
    accept_format_values.push_back( "cbor" );
    ValuesConstraint<std::string> accept_format_allowed_values( accept_format_values );

    ValueArg<std::string> accept_format{ "",
                                         "accept",
                                         "Accept format for content",
                                         false,
                                         "none",
                                         &accept_format_allowed_values,
                                         cmd_line };

    UnlabeledValueArg<std::string> endpoint{ "endpoint", "Endpoint to send message", true, "coap://", "endpoint", cmd_line };

    cmd_line.parse( argc, argv );

    commands.is_non        = non_msg.getValue();
    commands.is_observe    = obs.getValue();
    commands.method        = get_method_code_from_string( method.getValue() );
    commands.block2_size   = block2_size.getValue();
    commands.accept_format = get_accept_format_from_string( accept_format.getValue() );
    commands.payload       = payload.getValue();

    if( token.getValue().size() != 0 )
    {
      commands.token.set_token( (int8_t *) token.getValue().data(), token.getValue().size() );
    }

    if( !parse( endpoint.getValue(), commands ) )
    {
      return ( false );
    }

    return ( true );
  }
  catch( const std::exception &e )
  {
  }

  return ( false );
}

ja_iot::osal::Semaphore *cb_sem = nullptr;

void client_response_callback( ClientResponse *client_response, uint8_t status )
{
  if( status == CLIENT_RESPONSE_STATUS_OK )
  {
    cout << "got response" << endl;

    if( ( client_response != nullptr )
      && ( client_response->get_payload() != nullptr )
      && client_response->get_payload_len() )
    {
      auto &option_set = client_response->get_option_set();

      if( option_set.get_content_format() == COAP_CONTENT_FORMAT_CBOR )
      {
        ResRepresentation representation{};

        if( CborCodec::decode( client_response->get_payload(), client_response->get_payload_len(), representation ) == ErrCode::OK )
        {
          cout << "respresentation print" << endl;

          representation.print();
        }
      }
      else
      {
        std::string response{ (char *) client_response->get_payload(), client_response->get_payload_len() };
        printf( "Got Response :\n" );
        printf( "--------------\n" );
        printf( "%s\n", response.c_str() );
      }
    }
  }
  else if( status == CLIENT_RESPONSE_STATUS_REJECTED )
  {
    cout << "request rejected" << endl;
  }

  // cb_sem->Post();
}

int main( int argc, char *argv[] )
{
  CommandLineArgs commands;

  if( !get_commands( argc, argv, commands ) )
  {
    cout << "Error in argument parsing" << endl;
    return ( 1 );
  }

  init_adapter_mgr();
  MsgStack::inst().initialize( k_adapter_type_ip );

  Endpoint endpoint{ k_adapter_type_ip, commands.network_flag, commands.port, 0, commands.ip_addr };
  Client client{};

  if( commands.method == COAP_MSG_CODE_GET )
  {
    GetRequestInfo get_request_info{ endpoint, commands.path, client_response_callback };
    get_request_info._msg_type      = commands.is_non ? COAP_MSG_TYPE_NON : COAP_MSG_TYPE_CON;
    get_request_info._block2_size   = commands.block2_size;
    get_request_info._accept_format = commands.accept_format;

    if( !commands.query.empty() )
    {
      get_request_info._uri_query = commands.query;
    }

    client.get( get_request_info );
  }
  else if( commands.method == COAP_MSG_CODE_PUT )
  {
    PutRequestInfo put_request_info{ endpoint, commands.path, client_response_callback };
    put_request_info._msg_type       = commands.is_non ? COAP_MSG_TYPE_NON : COAP_MSG_TYPE_CON;
    put_request_info._block2_size    = commands.block2_size;
    put_request_info._accept_format  = commands.accept_format;
    put_request_info._content_format = COAP_CONTENT_FORMAT_PLAIN;

    if( !commands.query.empty() )
    {
      put_request_info._uri_query = commands.query;
    }

    // std::string payload = "CoAP makes use of two message types, requests and responses, using a simple, binary, base header format. The base header may be followed by options in an optimized Type-Length-Value format. CoAP is by default bound to UDP and optionally to DTLS, providing a high level of communications security.";
    put_request_info.set_payload( commands.payload );

    client.put( put_request_info );
  }
  else if( commands.method == COAP_MSG_CODE_POST )
  {
    PostRequestInfo put_request_info{ endpoint, commands.path, client_response_callback };
    put_request_info._msg_type       = commands.is_non ? COAP_MSG_TYPE_NON : COAP_MSG_TYPE_CON;
    put_request_info._block2_size    = commands.block2_size;
    put_request_info._accept_format  = commands.accept_format;
    put_request_info._content_format = COAP_CONTENT_FORMAT_CBOR;

    if( !commands.query.empty() )
    {
      put_request_info._uri_query = commands.query;
    }

    uint8_t *buffer;
    uint16_t buffer_length;

    ResRepresentation rep{};
    rep.add( "value", true );

    ResRepresentation representation{};
    representation.add( "", std::move( rep ) );

    CborCodec::encode( representation, buffer, buffer_length );

    // std::string payload = "CoAP makes use of two message types, requests and responses, using a simple, binary, base header format. The base header may be followed by options in an optimized Type-Length-Value format. CoAP is by default bound to UDP and optionally to DTLS, providing a high level of communications security.";
    put_request_info._payload_buf     = (int8_t *) buffer;
    put_request_info._payload_buf_len = buffer_length;

    client.post( put_request_info );
  }

  cb_sem = OsalMgr::Inst()->alloc_semaphore();

  // DBG_INFO( "main:%d# Going to wait", __LINE__ );

  cb_sem->Init( 0, 1 );
  cb_sem->Wait();

  // DBG_INFO( "main:%d# EXIT", __LINE__ );

  return ( 0 );
}
