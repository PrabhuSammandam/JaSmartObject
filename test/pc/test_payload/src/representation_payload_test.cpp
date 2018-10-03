/*
 * representation_payload_test.cpp
 *
 *  Created on: Oct 21, 2017
 *      Author: psammand
 */

#include "representation_payload_test.h"
#include "payload/representation_payload.h"

using namespace ja_iot::stack;
using namespace ja_iot::base;

void test_representation_payload_constructor()
{
  RepresentationPayload representation_payload{};

  representation_payload.set_resource_uri( "/a/light", false );

  representation_payload.add_resource_interface( "oic.if.baseline", false );
  representation_payload.add_resource_interface( "oic.if.ll", false );

  representation_payload.get_resource_interface_list()->print();

  {
    auto pcz_if_name = new CharArray{ "oic.if.a", false };

    representation_payload.add_resource_interface( *pcz_if_name );

    delete pcz_if_name;
  }

  representation_payload.get_resource_interface_list()->print();

  StringList cz_string_list{};

  cz_string_list.add_string( "oic.if.b", false );
  cz_string_list.add_string( "oic.if.c", false );

  representation_payload.add_resource_interface( cz_string_list, true );

  representation_payload.get_resource_interface_list()->print();
}

void test_representation_payload_string_list()
{
  RepresentationPayload representation_payload{};

  representation_payload.set_resource_uri( "/a/light", false );

  representation_payload.add_resource_interface( "oic.if.baseline", false );
  representation_payload.add_resource_interface( "oic.if.ll", false );

  representation_payload.get_resource_interface_list()->print();

  {
    auto pcz_if_name = new CharArray{ "oic.if.a", false };

    representation_payload.add_resource_interface( *pcz_if_name );

    delete pcz_if_name;
  }

  representation_payload.get_resource_interface_list()->print();

  StringList cz_string_list{};

  cz_string_list.add_string( "oic.if.b", false );
  cz_string_list.add_string( "oic.if.c", false );

  representation_payload.add_resource_interface( cz_string_list, true );

  representation_payload.get_resource_interface_list()->print();
}

void test_representation_payload_payload_field()
{
  RepresentationPayload representation_payload{};

  representation_payload.set_resource_uri("/a/light", false);

  representation_payload.add_resource_interface("oic.if.baseline", false);
  representation_payload.add_resource_interface("oic.if.ll", false);

  representation_payload.add_resource_type("oic.rt.batch", false);

  auto e_payload_field_type = representation_payload.get_payload_field_type( "test" );
  auto pcz_payload          = representation_payload.get_payload_field_by_name( "test" );

  /* adding first property */
  representation_payload.set_property( "range", 100.00 );

  /* setting different value with same name */
  representation_payload.set_property( "precision", 105.00 );

  /* changing the field type */
  representation_payload.set_property( "precision", false );

  representation_payload.set_property( "step", (int64_t) 10 );

  CharArray rt_string{ "oic.if.baseline.test", false };

  representation_payload.set_property( "rt", rt_string );

  RepresentationPayload* sub_representation_payload = new RepresentationPayload{};

  sub_representation_payload->set_resource_uri( "sub_payload" );
  sub_representation_payload->set_property( "a", 10.00 );

  representation_payload.set_property( "sub_payload", sub_representation_payload );

  representation_payload.print();
}

void test_representation_payload_base_payload_field_value()
{
  BasePayloadFieldValue base_pld_fld_val{ ePayloadFieldType::UNDEF };
}

/***
 * Following classes are tested,
 * BoolPayloadFieldValue
 * StringPayloadFieldValue
 */
void test_representation_payload_payload_field_value()
{
  BoolPayloadFieldValue bool_pld_fld_val{};

  BoolPayloadFieldValue bool_pld_fld_val1{ true };

  bool_pld_fld_val1.set_value( false );

  if( bool_pld_fld_val1 == bool_pld_fld_val )
  {
    BoolPayloadFieldValue pld_val3{ false };

    pld_val3 = bool_pld_fld_val1;
  }

  StringPayloadFieldValue str_pld{};
  StringPayloadFieldValue str_pld1{ "teststring" };
  StringPayloadFieldValue str_pld2{ "teststring2", false };

  char *pc_array = new char[100];

  strcpy( pc_array, "qwertyuiopasdfghjk" );

  StringPayloadFieldValue str_pld3{ pc_array };
  StringPayloadFieldValue str_pld4{ pc_array, false };

  StringPayloadFieldValue str_pld5{};

  /* copy assignment operator */
  str_pld5 = str_pld3;

  StringPayloadFieldValue str_pld6{};

  /* move assignment operator */
  str_pld6 = std::move( str_pld5 );
}

void test_representation_payload_object_payload_field_value()
{
  RepresentationPayload *rep_pld = new RepresentationPayload{};
  ObjectPayloadFieldValue obj_pld{};

  /* copy constructor */
  ObjectPayloadFieldValue obj_pld1{ rep_pld };

  /* move assignment operator */
  obj_pld = std::move( obj_pld1 );

  delete rep_pld;
}

void test_representation_payload_bool_array_payload_field_value()
{
  ArrayPayloadFieldValue arr_pld{};

  ArrayPayloadFieldValue arr_pld1{ ePayloadFieldType::BOOL };

  auto bool_pld_arr = new BoolPayloadFieldValue[10] {};

  for( auto i = 0; i < 10; ++i )
  {
    bool_pld_arr[i].set_value( ( i % 2 ) ? true : false );
  }

  arr_pld1.set_value( ePayloadFieldType::BOOL, bool_pld_arr, 10 );

  ArrayPayloadFieldValue arr_pld2{ ePayloadFieldType::BOOL, bool_pld_arr, 10 };
  arr_pld2.print();

  /* testing the assignment operator */
  auto arr_pld3 = new ArrayPayloadFieldValue{};
  *arr_pld3 = arr_pld2;
  arr_pld3->print();

  delete arr_pld3;

  /* testing the move assignment operator */
  ArrayPayloadFieldValue arr_pld4{};
  arr_pld4 = std::move( arr_pld2 );
  arr_pld4.print();

  printf( "after move\n" );
  arr_pld2.print();
}

cute::suite make_suite_representation_payload_test()
{
  cute::suite s{};
  // s.push_back( CUTE( test_representation_payload_base_payload_field_value ) );
  // s.push_back( CUTE( test_representation_payload_payload_field_value ) );
  // s.push_back( CUTE( test_representation_payload_object_payload_field_value ) );
  // s.push_back( CUTE( test_representation_payload_bool_array_payload_field_value ) );
  // s.push_back( CUTE( test_representation_payload_constructor ) );
  // s.push_back( CUTE( test_representation_payload_string_list ) );
  s.push_back( CUTE( test_representation_payload_payload_field ) );


  return ( s );
}
