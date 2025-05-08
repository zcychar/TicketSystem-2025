#pragma once

#include <string>

#include "storage/b_plus_tree_page.h"

namespace sjtu {
#define B_PLUS_TREE_INTERNAL_PAGE_TYPE BPlusTreeInternalPage<KeyType, ValueType, KeyComparator, DegradedKeyComparator>
#define INTERNAL_PAGE_HEADER_SIZE 12
#define INTERNAL_PAGE_SLOT_CNT \
  ((SJTU_PAGE_SIZE - INTERNAL_PAGE_HEADER_SIZE) / ((int)(sizeof(KeyType) + sizeof(ValueType))))  // NOLINT

  /**
   * Store `n` indexed keys and `n + 1` child pointers (page_id) within internal page.
   * Pointer PAGE_ID(i) points to a subtree in which all keys K satisfy:
   * K(i) <= K < K(i+1).
   * NOTE: Since the number of keys does not equal to number of child pointers,
   * the first key in key_array_ always remains invalid. That is to say, any search / lookup
   * should ignore the first key.
   *
   * Internal page format (keys are stored in increasing order):
   *  ---------
   * | HEADER |
   *  ---------
   *  ------------------------------------------
   * | KEY(1)(INVALID) | KEY(2) | ... | KEY(n) |
   *  ------------------------------------------
   *  ---------------------------------------------
   * | PAGE_ID(1) | PAGE_ID(2) | ... | PAGE_ID(n) |
   *  ---------------------------------------------
   */
  INDEX_TEMPLATE_ARGUMENTS
  class BPlusTreeInternalPage : public BPlusTreePage {
  public:
    // Delete all constructor / destructor to ensure memory safety
    BPlusTreeInternalPage() = delete;

    BPlusTreeInternalPage(const BPlusTreeInternalPage &other) = delete;

    void Init(int max_size = INTERNAL_PAGE_SLOT_CNT);

    auto KeyAt(int index) const -> KeyType;

    void SetKeyAt(int index, const KeyType &key);

    /**
     * @param value The value to search for
     * @return The index that corresponds to the specified value
     */
    auto ValueIndex(const ValueType &value) const -> int;

    auto ValueAt(int index) const -> ValueType;

    void SetValueAt(int index, const ValueType &value);

  private:
    // Array members for page data.
    KeyType key_array_[INTERNAL_PAGE_SLOT_CNT];
    ValueType page_id_array_[INTERNAL_PAGE_SLOT_CNT];
  };
} // namespace sjtu
