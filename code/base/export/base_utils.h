/*
 * utils.h
 *
 *  Created on: Sep 25, 2017
 *      Author: psammand
 */

#pragma once

#include <vector>
#include <algorithm>

namespace ja_iot {
namespace base {
template<typename T>
bool is_bit_set( const T value, const T bitmask )
{
  return ( ( value & bitmask ) == bitmask );
}

template<typename T1, typename T2>
auto set_bit( T1 &&value, T2 &&bitmask )->decltype( ( value |= bitmask ) )
{
  return ( ( value |= bitmask ) );
}

template<typename T1, typename T2>
auto clear_bit( T1 &&value, T2 &&bitmask )->decltype( ( value &= ( ~bitmask ) ) )
{
  return ( ( value &= ( ~bitmask ) ) );
}

template<typename T>
void delete_and_clear( T * &ptr_to_delete )
{
  if( ptr_to_delete != nullptr )
  {
    delete ptr_to_delete;
    ptr_to_delete = nullptr;
  }
}

template<typename T>
bool find_in_list(std::vector<T>& list, T& value)
{
	return std::find(list.cbegin(), list.cend(), value) != list.cend();
}

template<typename T>
bool find_in_list(std::vector<T>& list, const T& value)
{
	return std::find(list.cbegin(), list.cend(), value) != list.cend();
}
}
}
