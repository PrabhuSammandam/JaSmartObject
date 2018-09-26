/*
 * coap_options_set_test.cpp
 *
 *  Created on: Nov 12, 2017
 *      Author: psammand
 */

#include <set>
#include <iostream>
#include "cute.h"
#include "coap_options_set_test.h"
#include "coap/coap_options_set.h"
#include "coap/coap_option.h"

using namespace ja_iot::network;
using namespace std;

void test_coap_options_set_uri_query()
{
  CoapOptionsSet options_set{};

  /* uri query test */

  cout << "Testing URI QUERY " << endl << endl;

  options_set.add_uri_query( "" );
  cout << "uri query count : " << options_set.get_uri_querys_count() << endl;
  cout << "uri query string : " << options_set.get_uri_query_string() << endl << endl;
  options_set.clear_uri_querys_list();

  options_set.add_uri_query( "a" );
  cout << "uri query count : " << options_set.get_uri_querys_count() << endl;
  cout << "uri query string : " << options_set.get_uri_query_string() << endl << endl;
  options_set.clear_uri_querys_list();

  options_set.add_uri_query( "a" );
  options_set.add_uri_query( "b" );
  options_set.add_uri_query( "c" );
  cout << "uri query count : " << options_set.get_uri_querys_count() << endl;
  cout << "uri query string : " << options_set.get_uri_query_string() << endl << endl;

  options_set.clear_uri_querys_list();

  options_set.set_uri_query_string( "" );
  cout << "uri query string input " << "" << endl;
  cout << "uri query count : " << options_set.get_uri_querys_count() << endl;
  cout << "uri query string : " << options_set.get_uri_query_string() << endl << endl;

  options_set.set_uri_query_string( "&" );
  cout << "uri query string input " << "&" << endl;
  cout << "uri query count : " << options_set.get_uri_querys_count() << endl;
  cout << "uri query string : " << options_set.get_uri_query_string() << endl << endl;

  options_set.set_uri_query_string( "&a" );
  cout << "uri query string input " << "&a" << endl;
  cout << "uri query count : " << options_set.get_uri_querys_count() << endl;
  cout << "uri query string : " << options_set.get_uri_query_string() << endl << endl;

  options_set.set_uri_query_string( "a&" );
  cout << "uri query string input " << "a&" << endl;
  cout << "uri query count : " << options_set.get_uri_querys_count() << endl;
  cout << "uri query string : " << options_set.get_uri_query_string() << endl << endl;

  options_set.set_uri_query_string( "a&&&&" );
  cout << "uri query string input " << "a&&&&" << endl;
  cout << "uri query count : " << options_set.get_uri_querys_count() << endl;
  cout << "uri query string : " << options_set.get_uri_query_string() << endl << endl;

  options_set.set_uri_query_string( "a&b" );
  cout << "uri query string input " << "a&b" << endl;
  cout << "uri query count : " << options_set.get_uri_querys_count() << endl;
  cout << "uri query string : " << options_set.get_uri_query_string() << endl << endl;

  options_set.set_uri_query_string( "ab" );
  cout << "uri query string input " << "ab" << endl;
  cout << "uri query count : " << options_set.get_uri_querys_count() << endl;
  cout << "uri query string : " << options_set.get_uri_query_string() << endl << endl;
}

void test_coap_options_set_uri_path()
{
  CoapOptionsSet options_set{};

  options_set.add_uri_path( "" );
  cout << "uri path count : " << options_set.get_uri_paths_count() << endl;
  cout << "uri path string : " << options_set.get_uri_path_string() << endl;
  options_set.clear_uri_path_list();

  options_set.add_uri_path( "a" );
  cout << "uri path count : " << options_set.get_uri_paths_count() << endl;
  cout << "uri path string : " << options_set.get_uri_path_string() << endl;
  options_set.clear_uri_path_list();

  options_set.add_uri_path( "a" );
  options_set.add_uri_path( "b" );
  options_set.add_uri_path( "c" );
  cout << "uri path count : " << options_set.get_uri_paths_count() << endl;
  cout << "uri path string : " << options_set.get_uri_path_string() << endl;

  options_set.clear_uri_path_list();

  options_set.set_uri_path_string( "" );
  cout << "uri path string input " << "" << endl;
  cout << "uri path count : " << options_set.get_uri_paths_count() << endl;
  cout << "uri path string : " << options_set.get_uri_path_string() << endl;

  options_set.set_uri_path_string( "/" );
  cout << "uri path string input " << "/" << endl;
  cout << "uri path count : " << options_set.get_uri_paths_count() << endl;
  cout << "uri path string : " << options_set.get_uri_path_string() << endl;

  options_set.set_uri_path_string( "/a" );
  cout << "uri path string input " << "/a" << endl;
  cout << "uri path count : " << options_set.get_uri_paths_count() << endl;
  cout << "uri path string : " << options_set.get_uri_path_string() << endl;

  options_set.set_uri_path_string( "a/" );
  cout << "uri path string input " << "a/" << endl;
  cout << "uri path count : " << options_set.get_uri_paths_count() << endl;
  cout << "uri path string : " << options_set.get_uri_path_string() << endl;

  options_set.set_uri_path_string( "a////" );
  cout << "uri path string input " << "a////" << endl;
  cout << "uri path count : " << options_set.get_uri_paths_count() << endl;
  cout << "uri path string : " << options_set.get_uri_path_string() << endl;

  options_set.set_uri_path_string( "a/b" );
  cout << "uri path string input " << "a/b" << endl;
  cout << "uri path count : " << options_set.get_uri_paths_count() << endl;
  cout << "uri path string : " << options_set.get_uri_path_string() << endl;

  options_set.set_uri_path_string( "ab" );
  cout << "uri path string input " << "ab" << endl;
  cout << "uri path count : " << options_set.get_uri_paths_count() << endl;
  cout << "uri path string : " << options_set.get_uri_path_string() << endl;
}

void test_coap_options_set_constructor()
{
  CoapOptionsSet options_set{};

  ASSERT( options_set.has_uri_host() == false );

  cout << "uri host : " << options_set.get_uri_host() << endl;
  options_set.set_uri_host( "192.168.0.0" );
  cout << "uri host : " << options_set.get_uri_host() << endl;
  options_set.remove_uri_host();
  cout << "uri host : " << options_set.get_uri_host() << endl;
}

void test_coap_options_set_options()
{
  CoapOptionsSet options_set{};

  std::array<uint8_t, 4> byte_4{ 0x01, 0x02, 0x03, 04 };

  options_set.add_option( { 12, (uint16_t) byte_4.size(), byte_4.data() } );

  Option option{};

  Option option1{ 11 };
  ASSERT( option1.get_no() == 11 );

  Option options2{ 12, (uint16_t) byte_4.size(), byte_4.data() };
  ASSERT( options2.get_no() == 12 );
  ASSERT( options2.get_len() == 4 );

  Option option3{ 13, "abc" };
  ASSERT( option3.get_no() == 13 );
  ASSERT( option3.get_len() == 3 );

  Option option6{ 14, 0x01020304 };
  ASSERT( option6.get_no() == 14 );
  ASSERT( option6.get_len() == 4 );

  Option option4{ 15, 0x01 };
  ASSERT( option4.get_no() == 15 );
  ASSERT( option4.get_len() == 1 );

  Option option5{ 16 };
  option5.set_string_value( "123" );

  ASSERT( option5.get_no() == 16 );
  ASSERT( option5.get_string_value() == std::string( "123" ) );

  Option option7{};

  option7 = std::move( option5 );


  CoapOptionsSet options{};

  options.set_uri_host( "192.168.0.1" );
  options.set_uri_port( 5683 );
  options.set_uri_path_string( "a/b/c" );
  options.set_uri_query_string( "if=oic.baseline&rt=oic.device" );
  options.set_content_format( 20 ).set_content_version( 2048 )
  .set_accept_format( 10 )
  .set_accept_version( 2048 )
  .set_observe( 1 );

  CoapOptionsSet options_list2{};

  options_list2 = options;

  for( auto &option : options_list2.get_sorted_options_list() )
  {
    ( (Option &) option ).print();
  }

  cout << "Listing the original options list " << endl;
  for( auto &option : options.get_sorted_options_list() )
  {
    ( (Option &) option ).print();
  }

}

cute::suite make_suite_coap_options_set_test()
{
  cute::suite s{};
  // s.push_back( CUTE( test_coap_options_set_constructor ) );
  // s.push_back( CUTE( test_coap_options_set_uri_query ) );
  // s.push_back( CUTE( test_coap_options_set_uri_path ) );
  s.push_back( CUTE( test_coap_options_set_options ) );

  return ( s );
}
