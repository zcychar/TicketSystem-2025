#include <sstream>


#include "storage/b_plus_tree_leaf_page.h"

#include "management/user.h"

namespace sjtu {
  /*****************************************************************************
   * HELPER METHODS AND UTILITIES
   *****************************************************************************/

  /**
   * @brief Init method after creating a new leaf page
   *
   * After creating a new leaf page from buffer pool, must call initialize method to set default values,
   * including set page type, set current size to zero, set page id/parent id, set
   * next page id and set max size.
   *
   * @param max_size Max size of the leaf node
   */
  INDEX_TEMPLATE_ARGUMENTS
  void B_PLUS_TREE_LEAF_PAGE_TYPE::Init(int max_size) {
    SetPageType(IndexPageType::LEAF_PAGE);
    SetSize(0);
    SetMaxSize(max_size);
    SetNextPageId(INVALID_PAGE_ID);
  }

  /**
   * Helper methods to set/get next page id
   */
  INDEX_TEMPLATE_ARGUMENTS
  auto B_PLUS_TREE_LEAF_PAGE_TYPE::GetNextPageId() const -> page_id_t { return next_page_id_; }

  INDEX_TEMPLATE_ARGUMENTS
  void B_PLUS_TREE_LEAF_PAGE_TYPE::SetNextPageId(page_id_t next_page_id) { next_page_id_ = next_page_id; }

  /*
   * Helper method to find and return the key associated with input "index" (a.k.a
   * array offset)
   */
  INDEX_TEMPLATE_ARGUMENTS
  auto B_PLUS_TREE_LEAF_PAGE_TYPE::KeyAt(int index) const -> KeyType { return key_array_[index]; }

  INDEX_TEMPLATE_ARGUMENTS
  auto B_PLUS_TREE_LEAF_PAGE_TYPE::RidAt(int index) const -> ValueType { return rid_array_[index]; }

  INDEX_TEMPLATE_ARGUMENTS
  void B_PLUS_TREE_LEAF_PAGE_TYPE::SetKeyAt(int index, const KeyType &key) { key_array_[index] = key; }

  INDEX_TEMPLATE_ARGUMENTS
  void B_PLUS_TREE_LEAF_PAGE_TYPE::SetRidAt(int index, const ValueType &value) { rid_array_[index] = value; }

 template class BPlusTreeLeafPage<hash_t, UserInfo, UserInfoComp, UserInfoComp>;
} // namespace sjtu
