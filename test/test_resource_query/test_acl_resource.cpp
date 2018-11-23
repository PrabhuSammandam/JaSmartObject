/*
 * test_acl_resource.cpp
 *
 *  Created on: Nov 21, 2018
 *      Author: psammand
 */

#include "pch.h"
#include "security/AccessControlListRes.h"
#include "security/ProvisioningStatusRes.h"
#include "Interaction.h"
#include "common/inc/ServerInteraction.h"
#include "coap/coap_msg.h"
#include "end_point.h"
#include "StackMessage.h"
#include "common/inc/TokenProvider.h"
#include "ip_addr.h"
#include "CborCodec.h"
#include "ResourceMgr.h"

using namespace ja_iot::resources;
using namespace ja_iot::stack;
using namespace ja_iot::network;

IpAddress ip_address{ "192.168.0.106" };

ServerRequest* create_server_request()
{
  auto server_request = new ServerRequest{};

  server_request->set_code( COAP_MSG_CODE_GET );
  auto &ep = server_request->get_endpoint();
  new(&ep) Endpoint{ k_adapter_type_ip, k_network_flag_ipv4, 56855, 0, ip_address };

  TokenProvider::assign_next_token( *server_request );

  return ( server_request );
}

void test_get_method()
{
  auto server_request     = create_server_request();
  auto server_interaction = new ServerInteraction{ server_request->get_token(), server_request->get_endpoint() };

  server_interaction->set_server_request( server_request );

  auto acl_res = new AccessControlListRes{};

  acl_res->handle_request( server_interaction );

  auto server_response = server_interaction->get_server_response();
  ResRepresentation representation{};

  if( CborCodec::decode( server_response->get_payload(), server_response->get_payload_len(), representation ) == ErrCode::OK )
  {
    representation.print();
  }

  delete server_interaction;
}

void test_get_method_with_aceid()
{
  auto server_request     = create_server_request();
  auto server_interaction = new ServerInteraction{ server_request->get_token(), server_request->get_endpoint() };

  server_interaction->set_server_request( server_request );
  server_request->get_option_set().add_uri_query( "aceid=1" );

  auto acl_res = new AccessControlListRes{};

  acl_res->handle_request( server_interaction );

  auto server_response = server_interaction->get_server_response();
  ResRepresentation representation{};

  if( CborCodec::decode( server_response->get_payload(), server_response->get_payload_len(), representation ) == ErrCode::OK )
  {
    representation.print();
  }

  delete server_interaction;
}

void test_delete_method()
{
  auto server_request     = create_server_request();
  auto server_interaction = new ServerInteraction{ server_request->get_token(), server_request->get_endpoint() };

  server_interaction->set_server_request( server_request );
  server_request->set_code( COAP_MSG_CODE_DEL );

  auto acl_res = new AccessControlListRes{};

  acl_res->handle_request( server_interaction );

  auto server_response = server_interaction->get_server_response();
  ResRepresentation representation{};

  if( CborCodec::decode( server_response->get_payload(), server_response->get_payload_len(), representation ) == ErrCode::OK )
  {
    representation.print();
  }

  delete server_interaction;
}

void test_delete_method_with_single_aceid()
{
  auto server_request     = create_server_request();
  auto server_interaction = new ServerInteraction{ server_request->get_token(), server_request->get_endpoint() };

  server_interaction->set_server_request( server_request );
  server_request->set_code( COAP_MSG_CODE_DEL );
  server_request->get_option_set().add_uri_query( "aceid=1" );

  auto acl_res = new AccessControlListRes{};
  /* delete with known aceid */
  acl_res->handle_request( server_interaction );
  auto server_response = server_interaction->get_server_response();

  /* delete with unknown aceid */
  server_request->get_option_set().clear();
  server_request->get_option_set().add_uri_query( "aceid=6" );
  acl_res->handle_request( server_interaction );
  server_response = server_interaction->get_server_response();

  delete server_interaction;
}

void test_post_method_pnly_rowneruuid()
{
  auto server_request     = create_server_request();
  auto server_interaction = new ServerInteraction{ server_request->get_token(), server_request->get_endpoint() };

  server_interaction->set_server_request( server_request );
  server_request->set_code( COAP_MSG_CODE_POST );

  std::string uuid = "adsasd-dasds-dasd";
  CborEncoder cz_encoder{ 1024 };

  cz_encoder.start_map();
  cz_encoder.write_map_entry( "rowneruuid", uuid );
  cz_encoder.end_map();
  server_request->get_option_set().set_content_format( COAP_CONTENT_FORMAT_CBOR );
  server_request->set_payload( cz_encoder.get_buf(), cz_encoder.get_buf_len() );

  auto acl_res = new AccessControlListRes{};
  acl_res->handle_request( server_interaction );
  auto server_response = server_interaction->get_server_response();

  delete server_interaction;
}

void test_post_method_update_existing()
{
  auto server_request     = create_server_request();
  auto server_interaction = new ServerInteraction{ server_request->get_token(), server_request->get_endpoint() };

  server_interaction->set_server_request( server_request );
  server_request->set_code( COAP_MSG_CODE_POST );

  CborEncoder cz_encoder{ 1024 };

  cz_encoder.start_map();
  cz_encoder.write_string( "aclist2" );
  cz_encoder.start_array();
  cz_encoder.start_map();
  cz_encoder.write_map_entry( "aceid", 1 );
  cz_encoder.write_map_entry( "permission", CRUDN_PERMISSION_RETRIEVE );
  cz_encoder.end_map();
  cz_encoder.end_array();
  cz_encoder.end_map();

  server_request->get_option_set().set_content_format( COAP_CONTENT_FORMAT_CBOR );
  server_request->set_payload( cz_encoder.get_buf(), cz_encoder.get_buf_len() );

  auto acl_res = new AccessControlListRes{};
  acl_res->handle_request( server_interaction );
  auto server_response = server_interaction->get_server_response();

  delete server_interaction;
}

void test_post_method_add_new()
{
  auto server_request     = create_server_request();
  auto server_interaction = new ServerInteraction{ server_request->get_token(), server_request->get_endpoint() };

  server_interaction->set_server_request( server_request );
  server_request->set_code( COAP_MSG_CODE_POST );

  CborEncoder cz_encoder{ 1024 };

  cz_encoder.start_map();
  cz_encoder.write_string( "aclist2" );
  cz_encoder.start_array();

  auto ace = new AccessControlEntry{ 5 };
  ace->set_permission( CRUDN_PERMISSION_RETRIEVE );
  auto ace_resource = new AceResource{};
  ace_resource->set_href( "/light" );
  ace_resource->get_types().push_back( "oic.r.light" );
  ace_resource->get_interfaces().push_back( "oic.if.baseline" );
  ace_resource->get_interfaces().push_back( "oic.if.a" );
  ace->get_resources_array().push_back( ace_resource );

  ace_resource = new AceResource{};
  ace_resource->set_href( "/door" );
  ace_resource->get_types().push_back( "oic.r.door" );
  ace_resource->get_interfaces().push_back( "oic.if.baseline" );
  ace_resource->get_interfaces().push_back( "oic.if.a" );
  ace->get_resources_array().push_back( ace_resource );

  auto ace_subject_role = new AceSubjectRole{};
  ace_subject_role->set_authority( "484b8a51-cb23-46c0-a5f1-b4aebef50ebe" );
  ace_subject_role->set_role( "SOME_STRING" );

  ace->set_subject( ace_subject_role );

  ace->encode_to_cbor( cz_encoder );

  cz_encoder.end_array();
  cz_encoder.end_map();

  server_request->get_option_set().set_content_format( COAP_CONTENT_FORMAT_CBOR );
  server_request->set_payload( cz_encoder.get_buf(), cz_encoder.get_buf_len() );

  auto acl_res = new AccessControlListRes{};
  acl_res->handle_request( server_interaction );
  auto server_response = server_interaction->get_server_response();

  delete server_interaction;
}

int main()
{
  auto pstat_res = new ProvisioningStatusRes{};

  pstat_res->set_state( DEVICE_OPERATION_STATE_OWNER_TRANSFER_METHOD );
  ResourceMgr::inst().add_resource( pstat_res );
  // test_get_method();
  test_get_method_with_aceid();
  // test_delete_method();
  // test_delete_method_with_single_aceid();
  // test_post_method_pnly_rowneruuid();
  // test_post_method_update_existing();
  // test_post_method_add_new();

  return ( 0 );
}