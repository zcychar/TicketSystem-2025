#pragma once

#include <cassert>
#include <climits>
#include <cstdlib>
#include <string>

#include "buffer/buffer_pool_manager.h"

namespace sjtu {
#define MappingType std::pair<KeyType, ValueType>

#define INDEX_TEMPLATE_ARGUMENTS template <typename KeyType, typename ValueType, typename KeyComparator, typename DegradedKeyComparator>

  // define page type enum
  enum class IndexPageType { INVALID_INDEX_PAGE = 0, LEAF_PAGE, INTERNAL_PAGE };

  /**
   * Both internal and leaf page are inherited from this page.
   *
   * It actually serves as a header part for each B+ tree page and
   * contains information shared by both leaf page and internal page.
   *
   * Header format (size in byte, 12 bytes in total):
   * ---------------------------------------------------------
   * | PageType (4) | CurrentSize (4) | MaxSize (4) |  ...   |
   * ---------------------------------------------------------
   */
  class BPlusTreePage {
  public:
    // Delete all constructor / destructor to ensure memory safety
    BPlusTreePage() = delete;

    BPlusTreePage(const BPlusTreePage &other) = delete;

    ~BPlusTreePage() = delete;

    auto IsLeafPage() const -> bool;

    void SetPageType(IndexPageType page_type);

    auto GetSize() const -> int;

    void SetSize(int size);

    void ChangeSizeBy(int amount);

    auto GetMaxSize() const -> int;

    void SetMaxSize(int max_size);

    auto GetMinSize() const -> int;

  private:
    // Member variables, attributes that both internal and leaf page share
    IndexPageType page_type_;
    // Number of key & value pairs in a page
    int size_;
    // Max number of key & value pairs in a page
    int max_size_;
  };
} // namespace sjtu
