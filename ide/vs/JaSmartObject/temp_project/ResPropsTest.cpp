// temp_project.cpp : Defines the entry point for the console application.
//

#include <string>
#include <algorithm>
#include <deque>
#include <vector>
#include "ResPropValue.h"

using namespace std;
using namespace ja_iot::stack;

void test_res_prop_value_bool()
{
  /* value constructor */
  const ResPropValue v1{ false };

  /* copy constructor */
  auto v2{ v1 };

  /* move constructor */
  auto v3{ std::move(v2) };

  /* get */
  auto val = v3.get<bool>();

  /* set */
  v3.set(true);

  /* move value */

  ResPropValue v4{};
  v4.move<bool>(false);
  /* destroy */
}

void test_res_prop_value_int()
{
  /* value constructor */
  const ResPropValue v1{ static_cast<long>(656535) };

  /* copy constructor */
  auto v2{ v1 };

  /* move constructor */
  auto v3{ std::move(v2) };

  /* get */
  auto val = v3.get<long>();

  /* set */
  v3.set<long>(10);

  /* destroy */

}

void test_res_prop_value_double()
{
  /* value constructor */
  ResPropValue v1{ 245.00 };

  /* copy constructor */
  ResPropValue v2{ v1 };

  /* move constructor */
  ResPropValue v3{ std::move(v2) };

  /* get */
  auto val = v3.get<double>();

  /* set */
  v3.set<double>(10);

  /* destroy */
}

void test_res_prop_value_string()
{
  /* value constructor */
  ResPropValue v1{ std::string("hello") };

  /* copy constructor */
  ResPropValue v2{ v1 };

  /* move constructor */
  ResPropValue v3{ std::move(v2) };

  /* get */
  auto val = v3.get<std::string>();

  /* set */
  v3.set(std::string("world"));

  /* move value */
  std::string str1("move");
  ResPropValue v4{};

  v4.move(str1);

  /* destroy */
}

void test_res_prop_value_object()
{
  ResRepresentation value{};

  value.add("a", static_cast<long>(10));

  /* value constructor */
  ResPropValue v1{ value };
  /* copy constructor */
  ResPropValue v2{ v1 };

  /* move constructor */
  ResPropValue v3{ std::move(v2) };

  /* get */
  auto val = v3.get<ResRepresentation>();

  ResRepresentation value1{  };

  /* set */
  v3.set(value1);

  /* move value */
  ResPropValue v4{};
  v4.move(value1);

  /* destroy */
}

void test_res_prop_value_bool_array()
{
  std::vector<bool> values{true, false, true};

  /* value constructor */
  ResPropValue v1{ values };

  /* copy constructor */
  ResPropValue v2{ v1 };

  /* move constructor */
  ResPropValue v3{ std::move(v2) };

  /* get */
  auto val = v3.get<std::vector<bool>>();

  /* set */
  std::vector<bool> values2{ true, true, true };
  v3.set(values2);

  /* move assignmet */
  ResPropValue v4{};
  v4 = std::move(v3);

  /* destroy */

}

void test_res_prop_value_int_array()
{
  std::vector<long> values{ 100, 1000, 10000 };

  /* value constructor */
  ResPropValue v1{ values };

  /* copy constructor */
  ResPropValue v2{ v1 };

  /* move constructor */
  ResPropValue v3{ std::move(v2) };

  /* get */
  auto val = v3.get<std::vector<long>>();

  /* set */
  std::vector<long> values2{ 200, 2000, 20000 };
  v3.set(values2);

  /* move assignmet */
  ResPropValue v4{};
  v4 = std::move(v3);

  /* destroy */
}

void test_res_prop_value_double_array()
{
  std::vector<double> values{ 100.00, 1000.00, 10000.00 };

  /* value constructor */
  ResPropValue v1{ values };

  /* copy constructor */
  ResPropValue v2{ v1 };

  /* move constructor */
  ResPropValue v3{ std::move(v2) };

  /* get */
  auto val = v3.get<std::vector<double>>();

  /* set */
  std::vector<double> values2{ 200.0, 2000.0, 20000.0 };
  v3.set(values2);

  /* move assignmet */
  ResPropValue v4{};
  v4 = std::move(v3);

  /* destroy */
}

void test_res_prop_value_string_array()
{
  std::vector<std::string> values{ "hello", "world", "goodbye" };

  /* value constructor */
  ResPropValue v1{ values };

  /* copy constructor */
  ResPropValue v2{ v1 };

  /* move constructor */
  ResPropValue v3{ std::move(v2) };

  /* get */
  auto val = v3.get<std::vector<std::string>>();

  /* set */
  std::vector<std::string> values2{ "hi", "test" };
  v3.set(values2);

  /* move assignmet */
  ResPropValue v4{};
  v4 = std::move(v3);

  /* destroy */

}

void test_res_prop_value_object_array()
{
  std::vector<std::string> if_types{ "oic.if.baseline", "oic.if.ll", "oic.if.b", "oic.if.lb", "oic.if.rw", "oic.if.r", "oic.if.a", "oic.if.s" };

  ResRepresentation props{};

  props.add(std::string("precision"), (long)200);
  props.add(std::string("step"), (long)2);
  props.add(std::string("if"), std::move(if_types));

  std::vector<std::string> rt_types2{ "oic.if.baseline", "oic.if.ll" };

  ResRepresentation props2{};

  props2.add(std::string("precision"), (long)300);
  props2.add(std::string("step"), (long)4);
  props2.add(std::string("if"), std::move(rt_types2));

  std::vector<ResRepresentation> obj_array{};

  obj_array.push_back(std::move(props));
  obj_array.push_back(std::move(props2));

  ResPropValue obj_arr_val{ obj_array };
}

void test_res_prop_value()
{
  test_res_prop_value_bool();
  test_res_prop_value_int();
  test_res_prop_value_double();
  test_res_prop_value_string();
  test_res_prop_value_object();
  test_res_prop_value_bool_array();
  test_res_prop_value_int_array();
  test_res_prop_value_double_array();
  test_res_prop_value_string_array();
  test_res_prop_value_object_array();
}

