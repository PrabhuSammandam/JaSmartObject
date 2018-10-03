/*
 * utils_test.cpp
 *
 *  Created on: Oct 23, 2017
 *      Author: psammand
 */

#include "utils_test.h"
#include "utils/uri_parser.h"
#include "cute.h"

using namespace ja_iot::stack;

void utils_test()
{
  /* without query */
  UriParser uriparser = "/a/b/";

  uriparser.parse();

  ASSERT( uriparser.get_uri() != nullptr );
  ASSERT( uriparser.get_uri_length() == 5 );
  ASSERT( uriparser.get_query() == nullptr );
  ASSERT( uriparser.get_query_length() == 0 );
  ASSERT( uriparser.is_url_valid() == true );
  ASSERT( uriparser.is_query_valid() == false );

  /* with empty query */
  uriparser = "/a/b/?";
  uriparser.parse();

  ASSERT( uriparser.get_uri() != nullptr );
  ASSERT( uriparser.get_uri_length() == 5 );
  ASSERT( uriparser.get_query() != nullptr );
  ASSERT( uriparser.get_query_length() == 0 );
  ASSERT( uriparser.is_url_valid() == true );
  ASSERT( uriparser.is_query_valid() == false );

  /* with query */
  uriparser = "/a/b/?/ex=text";
  uriparser.parse();

  ASSERT( uriparser.get_uri() != nullptr );
  ASSERT( uriparser.get_uri_length() == 5 );
  ASSERT( uriparser.get_query() != nullptr );
  ASSERT( uriparser.get_query_length() == 8 );
  ASSERT( uriparser.is_url_valid() == true );
  ASSERT( uriparser.is_query_valid() == true );

  /* with empty url*/
  uriparser = "?/ex=text";
  uriparser.parse();

  ASSERT( uriparser.get_uri() != nullptr );
  ASSERT( uriparser.get_uri_length() == 0 );
  ASSERT( uriparser.get_query() != nullptr );
  ASSERT( uriparser.get_query_length() == 8 );
  ASSERT( uriparser.is_url_valid() == false );
  ASSERT( uriparser.is_query_valid() == true );
}

cute::suite make_suite_utils_test()
{
  cute::suite s{};
  s.push_back( CUTE( utils_test ) );

  return ( s );
}
