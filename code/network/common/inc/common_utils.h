/*
 * common_utils.h
 *
 *  Created on: Jul 11, 2017
 *      Author: psammand
 */

#ifndef COMMON_UTILS_H_
#define COMMON_UTILS_H_

#include <data_types.h>

namespace ja_iot {
namespace network {
template<typename T>
bool IsBitSet( const T value, const T bitmask )
{
  return ( ( value & bitmask ) == bitmask );
}

template<typename T>
void delete_items_and_clear_list(T & list)
{
	for(auto& item : list)
	{
		delete item;
	}

	list.clear();
}

}
}

#endif /* COMMON_UTILS_H_ */
