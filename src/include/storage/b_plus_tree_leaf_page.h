#pragma once

#include <string>
#include <utility>
#include <vector>

#include "storage/b_plus_tree_page.h"

namespace sjtu {
#define B_PLUS_TREE_LEAF_PAGE_TYPE BPlusTreeLeafPage<KeyType, ValueType, KeyComparator, DegradedKeyComparator>
#define LEAF_PAGE_HEADER_SIZE 24
#define LEAF_PAGE_SLOT_CNT ((SJTU_PAGE_SIZE - LEAF_PAGE_HEADER_SIZE) / (sizeof(KeyType) + sizeof(ValueType)))

  /**
   * Store indexed key and record id (record id = page id combined with slot id,
   * see `include/common/rid.h` for detailed implementation) together within leaf
   * page. Only support unique key.
   *
   * Leaf page format (keys are stored in order):
   *  ---------
   * | HEADER |
   *  ---------
   *  ---------------------------------
   * | KEY(1) | KEY(2) | ... | KEY(n) |
   *  ---------------------------------
   *  ---------------------------------
   * | RID(1) | RID(2) | ... | RID(n) |
   *  ---------------------------------
   *
   *  Header format (size in byte, 16 bytes in total):
   *  -----------------------------------------------
   * | PageType (4) | CurrentSize (4) | MaxSize (4) |
   *  -----------------------------------------------
   *  -----------------
   * | NextPageId (4) |
   *  -----------------
   */
  INDEX_TEMPLATE_ARGUMENTS
  class BPlusTreeLeafPage : public BPlusTreePage {
  public:
    // Delete all constructor / destructor to ensure memory safety
    BPlusTreeLeafPage() = delete;

    BPlusTreeLeafPage(const BPlusTreeLeafPage &other) = delete;

    void Init(int max_size = LEAF_PAGE_SLOT_CNT);

    // Helper methods
    auto GetNextPageId() const -> page_id_t;

    void SetNextPageId(page_id_t next_page_id);

    auto KeyAt(int index) const -> KeyType;

    auto RidAt(int index) const -> ValueType;

    void SetKeyAt(int index, const KeyType &key);

    void SetRidAt(int index, const ValueType &value);

  private:
    page_id_t next_page_id_;
    // Array members for page data.
    KeyType key_array_[LEAF_PAGE_SLOT_CNT];
    ValueType rid_array_[LEAF_PAGE_SLOT_CNT];
  };
} // namespace sjtu
