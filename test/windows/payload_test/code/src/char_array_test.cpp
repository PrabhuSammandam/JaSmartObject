/*
 * char_array_test.cpp
 *
 *  Created on: Oct 15, 2017
 *      Author: psammand
 */

#include <string.h>
#include "cute.h"
#include "char_array_test.h"
#include "utils/char_array.h"

using namespace ja_iot::stack;

void test_char_array_equality()
{
  CharArray a{ "test1" };
  CharArray b{ "test1" };
}

void test_char_array_assignments()
{
  CharArray a{};

  /* move operator with < 10 */
  {
    a = "";
  }

  /* move operator with < 10 */
  {
    a = "test";
  }

  /* move operator with > 10 */
  {
    a = "move operator with > 10";
  }

  CharArray b{ "test1" };
  CharArray c{ b };

  /* assignment operator */
  b = c;
}

void test_char_array_constructor()
{
  /* default constructor */
  CharArray a{};

  /* const string > 10 and owned */
  {
    CharArray aa{ "a very long string greater than 10", false };
  }

  /* const string > 10 and not owned */
  {
    CharArray aa{ "a very long string greater than 10", true };
  }

  /* const string < 10 and owned */
  {
    CharArray aa{ "char", false };
  }

  /* const string < 10 and not owned */
  {
    CharArray aa{ "char", true };
  }

  /* non const string < 10 and not duplicated means do not allocate any memory, just use passed memory*/
  /* expectation - copied to internal buffer
   * should delete passed parameter, since it is less than 10, internally data copied to buffer, so need of allocated passed memory.
   * no deletion of pointer during destruction */
  {
    char *test1 = new char[10]{};

    strcpy( test1, "test1" );
    CharArray a3{ test1, false };
  }

  /* non const string < 10 and duplicate means don't use the passed memory */
  /* expectation - copied to internal buffer
   * should not delete passed parameter
   * no deletion of pointer during destruction */
  {
    char *test1 = new char[10]{};

    strcpy( test1, "test1" );
    CharArray a3{ test1, true };

    delete[] test1;
  }

  /* non const string > 10 and not duplicated means do not allocate any memory, just use passed memory*/
  /* expectation -
   * should not allocate new memory just hold the reference
   * should not delete passed paramater
   * delete pointer during destruction */
  {
    char *test1 = new char[100]{};

    strcpy( test1, "no deletion of pointer during destruction" );
    CharArray a3{ test1 , false};
  }

  /* non const string > 10 and and duplicate means don't use the passed memory */
  /* expectation -
   * allocate new memory and copy the value
   * should delete passed paramater
   * delete pointer during destruction */
  {
    char *test1 = new char[100]{};

    strcpy( test1, "no deletion of pointer during destruction" );
    CharArray a3{ test1, true };
  }
}

cute::suite make_suite_char_array_test()
{
	cute::suite s { };
	s.push_back(CUTE(test_char_array_constructor));
	s.push_back(CUTE(test_char_array_equality));
	s.push_back(CUTE(test_char_array_assignments));

	return s;
}

