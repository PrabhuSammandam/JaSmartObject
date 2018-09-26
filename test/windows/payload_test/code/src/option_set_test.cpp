/*
 * option_set_test.cpp
 *
 *  Created on: Nov 11, 2017
 *      Author: psammand
 */
#include <string>
#include <iostream>
#include <string.h>
#include "option_set_test.h"
#include "coap/coap_option_set.h"
#include "coap/coap_option.h"
#include <cute.h>

using namespace ja_iot::network;
using namespace std;

void test_option()
{
  const char *pc_small_data = "1234";
  const char *pc_big_data   = "this is very big string";
  Option option1{};
  Option option2{ 10, 0, nullptr };

  Option option3{ 10, (uint16_t) strlen( pc_small_data ), nullptr };

  Option option4{ 10, 0, (uint8_t *) pc_small_data };

  Option option5{ 10, (uint16_t) strlen( pc_small_data ), (uint8_t *) pc_small_data };

  Option option6{ 10, (uint16_t) strlen( pc_big_data ), (uint8_t *) pc_big_data };

  auto small_data_len = option5.get_len();
  auto big_data_len   = option6.get_len();
}

void test_option_set_constructor()
{
  OptionsSet options_set{};

  options_set.add_option( 11, 0 );
  options_set.add_option( 12, 1 );
  options_set.add_option( 13, 0xF0F0 );
  options_set.add_option( 14, 0xFF00FF00 );
  options_set.add_option( 15, 0xFF00FF00FF00FF00 );

  auto opt_11_value = options_set.get_value( 11 );
  ASSERT(opt_11_value == 0);
  auto opt_12_value = options_set.get_value( 12 );
  ASSERT(opt_12_value == 1);
  auto opt_13_value = options_set.get_value( 13 );
  ASSERT(opt_13_value == 0xF0F0);
  auto opt_14_value = options_set.get_value( 14 );
  ASSERT(opt_14_value == 0xFF00FF00);
  auto opt_15_value = options_set.get_value( 15 );
  ASSERT(opt_15_value == 0xFF00FF00);

  auto observe_option = options_set.HasObserve();
  ASSERT(observe_option == false);

  options_set.SetObserve(1);
  observe_option = options_set.HasObserve();
  ASSERT(observe_option == true);
  auto observe_value = options_set.GetObserve();
  ASSERT(observe_value == 1);

  // options_set.SetContentFormat(0x1234);
  //
  // if(options_set.HasObserve())
  // {
  // options_set.SetObserve(1);
  // }
}

void test_option_uri()
{
  OptionsSet option_set{};

  std::string uri_path{};
  std::string uri_query{};
  std::string full_uri{};

  cout << "Testing no uri path and no uri query" << endl;

  option_set.get_uri( uri_path );
  option_set.get_query( uri_query );
  option_set.get_full_uri( full_uri );

  cout << "uri path : " << uri_path << endl;
  cout << "uri query : " << uri_query << endl;
  cout << "full uri : " << full_uri << endl;

  std::string path1 = "a";
  std::string path2 = "test";
  std::string path3 = "fridge";
  option_set.add_option( (uint16_t) OptionType::URI_PATH, (uint16_t) path1.length(), (uint8_t *) path1.data() );
  option_set.add_option( (uint16_t) OptionType::URI_PATH, (uint16_t) path2.length(), (uint8_t *) path2.data() );
  option_set.add_option( (uint16_t) OptionType::URI_PATH, (uint16_t) path3.length(), (uint8_t *) path3.data() );

  cout << "Testing uri path and no uri query" << endl;

  option_set.get_uri( uri_path );
  option_set.get_query( uri_query );
  option_set.get_full_uri( full_uri );

  cout << "uri path : " << uri_path << endl;
  cout << "uri query : " << uri_query << endl;
  cout << "full uri : " << full_uri << endl;

  std::string q1 = "a=10";
  std::string q2 = "test=string";
  std::string q3 = "fridge=str";

  option_set.add_option( (uint16_t) OptionType::URI_QUERY, (uint16_t) q1.length(), (uint8_t *) q1.data() );
  option_set.add_option( (uint16_t) OptionType::URI_QUERY, (uint16_t) q2.length(), (uint8_t *) q2.data() );
  option_set.add_option( (uint16_t) OptionType::URI_QUERY, (uint16_t) q3.length(), (uint8_t *) q3.data() );

  cout << "Testing uri path and uri query" << endl;

  option_set.get_uri( uri_path );
  option_set.get_query( uri_query );
  option_set.get_full_uri( full_uri );

  cout << "uri path : " << uri_path << endl;
  cout << "uri query : " << uri_query << endl;
  cout << "full uri : " << full_uri << endl;
}

class Sample
{
  public:
    Sample ()
    {
      cout << "calling constructor" << endl;
    }
    Sample( const Sample &other )
    {
      cout << "calling copy constructor" << endl;
    }
    Sample( Sample &&other )
    {
      cout << "calling move constructor" << endl;
    }

    Sample & operator = ( Sample &&other )
    {
      cout << "calling move operator" << endl;
      this->i = other.i;
      return ( *this );
    }

    Sample & operator = ( const Sample &other )
    {
      cout << "calling assign operator" << endl;
      return ( *this );
    }

  public:
    int i{};
};

Sample get_sample()
{
  Sample x{};

  x.i = 20;

  return ( x );
}

void test_return_by_value()
{
  Sample x;

  x = get_sample();

  cout << "sample value " << x.i << endl;
}

cute::suite make_suite_option_set_test()
{
  cute::suite s{};
  // s.push_back( CUTE( test_return_by_value ) );
  // s.push_back( CUTE( test_option ) );
  s.push_back( CUTE( test_option_set_constructor ) );
  // s.push_back( CUTE( test_option_uri ) );

  return ( s );
}
