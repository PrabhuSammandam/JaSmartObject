/*
 * Simplelist.h
 *
 *  Created on: Jun 27, 2017
 *      Author: psammand
 */

#ifndef OSAL_EXPORT_SIMPLELIST_H_
#define OSAL_EXPORT_SIMPLELIST_H_

#include <cstdint>

namespace ja_iot {
namespace base {

template <typename T, uint16_t capacity> class SimpleList {
private:
  struct ListEntry {
    T item;
    bool is_item_free;
    ListEntry() : item{}, is_item_free{true} {}
  };

  ListEntry item_list_[capacity];
  uint16_t item_count_ = 0;

public:
  SimpleList() {}

  T *Alloc() {
    for (uint16_t i = 0; i < capacity; i++) {
      if (item_list_[i].is_item_free == true) {
        item_list_[i].is_item_free = false;
        item_count_++;
        return (&item_list_[i].item);
      }
    }

    return (nullptr);
  }

  void Free(T *listEntry) {
    if (listEntry != nullptr) {
      for (uint16_t i = 0; i < capacity; i++) {
        if (&item_list_[i].item == listEntry) {
          item_count_--;
          item_list_[i].is_item_free = true;
        }
      }
    }
  }

  uint16_t Count() { return (item_count_); }

  bool IsFull() { return (capacity == item_count_); }

  uint16_t Capacity() { return (capacity); }
};
}
}

#endif /* OSAL_EXPORT_SIMPLELIST_H_ */
