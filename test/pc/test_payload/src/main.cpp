/*
 * main.cpp
 *
 *  Created on: Oct 12, 2017
 *      Author: psammand
 */

#include <string.h>
#include <stdio.h>
#include <utility>
#include <cstdint>

#include "cute.h"
#include "ide_listener.h"
#include "xml_listener.h"
#include "cute_runner.h"

#include "char_array_test.h"
#include "string_list_test.h"
#include "utils_test.h"
#include "representation_payload_test.h"
#include "option_set_test.h"
#include "coap_options_set_test.h"
#include "coap_msg_test.h"

int main( int argc, char const *argv[] )
{
  cute::xml_file_opener                     xmlfile( argc, argv );
  cute::xml_listener<cute::ide_listener<> > lis( xmlfile.out );

  auto                                      runner                = cute::makeRunner( lis, argc, argv );
  auto                                      char_array_test_suite = make_suite_char_array_test();
  // runner( char_array_test_suite, "char_array_test" );

  auto                                      string_list_test_suite = make_suite_string_list_test();
  // runner( string_list_test_suite, "string_list_test" );

  auto                                      representation_payload_test_suite = make_suite_representation_payload_test();
  // runner( representation_payload_test_suite, "representation_payload_test" );

  // auto                                      utils_test_suite = make_suite_utils_test();
  // runner( utils_test_suite, "utils_test" );
  // auto option_set_test_suite = make_suite_option_set_test();
  // runner( option_set_test_suite, "option_set_test" );

//  auto coap_options_set_test_suite = make_suite_coap_options_set_test();
//  runner( coap_options_set_test_suite, "coap_options_set_test" );

  auto coap_msg_test_suite = make_suite_coap_msg_test();
  runner( coap_msg_test_suite, "coap_msg_test" );

  return ( 1 );
}
