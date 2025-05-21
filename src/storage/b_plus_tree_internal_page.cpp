#include <iostream>
#include <sstream>
#include "storage/b_plus_tree_internal_page.h"
#include "management/user.h"
#include "management/train.h"
#include "management/ticket.h"

namespace sjtu {
/*****************************************************************************
 * HELPER METHODS AND UTILITIES
 *****************************************************************************/

/**
 * @brief Init method after creating a new internal page.
 *
 * Writes the necessary header information to a newly created page,
 * including set page type, set current size, set page id, set parent id and set max page size,
 * must be called after the creation of a new page to make a valid BPlusTreeInternalPage.
 *
 * @param max_size Maximal size of the page
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::Init(int max_size) {
  SetSize(0);
  SetMaxSize(max_size);
  SetPageType(IndexPageType::INTERNAL_PAGE);
}

/**
 * @param value The value to search for
 * @return The index that corresponds to the specified value
 */
INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::ValueIndex(
    const ValueType& value) const -> int {
  auto size = GetSize();
  for (int i = 0; i < size; ++i) {
    if (value == page_id_array_[i]) {
      return i;
    }
  }
  return INVALID_PAGE_ID;
}

/**
 * @brief Helper method to get/set the key associated with input "index"(a.k.a
 * array offset).
 *
 * @param index The index of the key to get. Index must be non-zero.
 * @return Key at index
 */
INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::KeyAt(int index) const -> KeyType {
  return key_array_[index];
}

/**
 * @brief Set key at the specified index.
 *
 * @param index The index of the key to set. Index must be non-zero.
 * @param key The new value for key
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::SetKeyAt(int index, const KeyType& key) {
  key_array_[index] = key;
}

/**
 * @brief Helper method to get the value associated with input "index"(a.k.a array
 * offset)
 *
 * @param index The index of the value to get.
 * @return Value at index
 */
INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::ValueAt(int index) const -> ValueType {
  return page_id_array_[index];
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::SetValueAt(
    int index, const ValueType& value) { page_id_array_[index] = value; }

// valuetype for internalNode should be page id_t
template class BPlusTreeInternalPage<hash_t, page_id_t, HashComp, HashComp>;
template class BPlusTreeInternalPage<
  TrainDate, page_id_t, PairCompare<TrainDate>, PairDegradedCompare<
    TrainDate> >;
template class BPlusTreeInternalPage<
  OrderTime, page_id_t, PairCompare<OrderTime>, PairDegradedCompare<
    OrderTime> >;
template class BPlusTreeInternalPage<
  TrainDateOrder, page_id_t,TDOCompare, TDODegradedCompare >;
template class BPlusTreeInternalPage<
  StationTrain, page_id_t, PairCompare<StationTrain>, PairDegradedCompare<StationTrain> >;
} // namespace sjtu