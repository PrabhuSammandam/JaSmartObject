/*
 * string_list_test.cpp
 *
 *  Created on: Oct 15, 2017
 *      Author: psammand
 */
#include "string_list_test.h"
#include "utils/string_list.h"
#include "utils/string_node.h"

using namespace ja_iot::stack;

void string_list_test_constructor()
{
  /* empty constructor */
  StringList string_list{};

  string_list.insert_back( new StringNode{ "a" } );
  string_list.insert_back( new StringNode{ "b" } );

  auto list_count = string_list.count();

  /* copy constructor */
  StringList string_list_b{ string_list };

  list_count = string_list_b.count();

  /* move assignment operator */
  StringList string_list_c{};

  string_list_c = std::move(string_list_b);

  /* move constructor */
  StringList string_list_d{std::move(string_list_c)};

  list_count = string_list_d.count();
}

void string_list_split_test()
{
	const char* pc_input_string = "rt,if,name";
	CharArray cz_input_string{pc_input_string};
	StringList cz_token_list{};

	cz_token_list.split(",", cz_input_string);

	auto list_count = cz_token_list.count();

	cz_token_list.destroy_all_nodes();

	cz_input_string = "";

	cz_token_list.split(",", cz_input_string);

	list_count = cz_token_list.count();

	cz_input_string = ",";

	cz_token_list.split(",", cz_input_string);

	list_count = cz_token_list.count();

	cz_input_string = ",,,,";

	cz_token_list.split(",", cz_input_string);

	list_count = cz_token_list.count();

	cz_input_string = ",a,b,c,d";

	cz_token_list.split(",", cz_input_string);

	list_count = cz_token_list.count();

}

void string_list_join_test()
{
	const char* pc_input_string = "rt,if,name";
	CharArray cz_input_string{pc_input_string};
	StringList cz_token_list{};

	cz_token_list.split(",", cz_input_string);

	CharArray cz_joined_string{};

	cz_token_list.join(cz_joined_string);
}

cute::suite make_suite_string_list_test()
{
  cute::suite s{};
  s.push_back( CUTE( string_list_test_constructor ) );
  s.push_back( CUTE( string_list_split_test ) );
  s.push_back( CUTE( string_list_join_test ) );

  return ( s );
}
