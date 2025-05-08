#include "storage/b_plus_tree.h"
#include "common/container.h"

namespace sjtu {
  INDEX_TEMPLATE_ARGUMENTS
  BPLUSTREE_TYPE::BPlusTree(std::string name, page_id_t header_page_id, BufferPoolManager *buffer_pool_manager,
                            const KeyComparator &comparator, const DegradedKeyComparator &degraded_comparator,
                            int leaf_max_size, int internal_max_size)
    : index_name_(std::move(name)),
      bpm_(buffer_pool_manager),
      comparator_(std::move(comparator)),
      degraded_comparator_(std::move(degraded_comparator)),
      leaf_max_size_(leaf_max_size),
      internal_max_size_(internal_max_size),
      header_page_id_(header_page_id) {
    WritePageGuard guard = bpm_->WritePage(header_page_id_);
    auto root_page = guard.AsMut<BPlusTreeHeaderPage>();
    if(root_page->root_page_id_==0&&root_page->next_page_id_==0) {
      root_page->root_page_id_ = INVALID_PAGE_ID;
    }else {
      buffer_pool_manager->SetNextPageId(root_page->next_page_id_);
    }

  }

  /**
   * @brief Helper function to decide whether current b+tree is empty
   * @return Returns true if this B+ tree has no keys and values.
   */
  INDEX_TEMPLATE_ARGUMENTS
  auto BPLUSTREE_TYPE::IsEmpty() const -> bool {
    ReadPageGuard guard = bpm_->ReadPage(header_page_id_);
    auto root_page = guard.As<BPlusTreeHeaderPage>();
    return root_page->root_page_id_ == INVALID_PAGE_ID;
  }

  /*****************************************************************************
   * SEARCH
   *****************************************************************************/
  /**
   * @brief Return the only value that associated with input key
   *
   * This method is used for point query
   *
   * @param key input key
   * @param[out] result vector that stores the only value that associated with input key, if the value exists
   * @return : true means key exists
   */
  INDEX_TEMPLATE_ARGUMENTS
  auto BPLUSTREE_TYPE::GetValue(const KeyType &key, std::vector<ValueType> *result) -> bool {
    // Declaration of context instance.
    auto head_guard = bpm_->ReadPage(header_page_id_);
    auto head_page = head_guard.As<BPlusTreeHeaderPage>();
    if (head_page->root_page_id_ == INVALID_PAGE_ID) {
      return false;
    }
    auto cur_guard = bpm_->ReadPage(head_page->root_page_id_);
    auto cur_page = cur_guard.As<BPlusTreePage>();

    while (!cur_page->IsLeafPage()) {
      auto page = cur_guard.As<InternalPage>();
      auto page_size = page->GetSize();
      auto slot = page_size - 1;
      for (int i = 1; i < page_size; ++i) {
        if (comparator_(page->KeyAt(i), key) >= 0) {
          slot = i;
          break;
        }
      }
      if (comparator_(key, page->KeyAt(slot)) < 0) {
        --slot;
      }
      cur_guard = bpm_->ReadPage(page->ValueAt(slot));
      cur_page = cur_guard.As<BPlusTreePage>();
    }

    auto leaf_page = cur_guard.As<LeafPage>();
    auto leaf_size = leaf_page->GetSize();
    for (int i = 0; i < leaf_size; ++i) {
      if (comparator_(key, leaf_page->KeyAt(i)) == 0) {
        result->emplace_back(leaf_page->RidAt(i));
        return true;
      }
    }
    return false;
  }

  /**
 * @brief Return all the value that associated with input key
 *
 * This method is used for range query
 *
 * @param key input key
 * @param[out] result vector that stores all the  value that associated with input key, if the value exists
 * @return : true means key exists
 */
  INDEX_TEMPLATE_ARGUMENTS
  auto BPLUSTREE_TYPE::GetAllValue(const KeyType &key, std::vector<ValueType> *result) -> bool {
    // Declaration of context instance.
    Context ctx;
    auto head_guard = bpm_->ReadPage(header_page_id_);
    auto head_page = head_guard.As<BPlusTreeHeaderPage>();
    if (head_page->root_page_id_ == INVALID_PAGE_ID) {
      return false;
    }
    ctx.read_set_.emplace_back(bpm_->ReadPage(head_page->root_page_id_)) ;
    auto cur_page = ctx.read_set_.back().As<BPlusTreePage>();

    while (!cur_page->IsLeafPage()) {
      auto page =  ctx.read_set_.back().As<InternalPage>();
      auto page_size = page->GetSize();
      auto slot = page_size - 1;
      for (int i = 1; i < page_size; ++i) {
        if (degraded_comparator_(page->KeyAt(i), key) >= 0) {
          slot = i;
          break;
        }
      }
      if (degraded_comparator_(key, page->KeyAt(slot)) < 0) {
        --slot;
      }
      ctx.read_set_.emplace_back(bpm_->ReadPage(page->ValueAt(slot))) ;
      cur_page = ctx.read_set_.back().As<BPlusTreePage>();
    }

    auto leaf_page = ctx.read_set_.back().As<LeafPage>();
    auto leaf_size = leaf_page->GetSize();
    for (int i = 0; i < leaf_size; ++i) {
      auto flag=degraded_comparator_(key, leaf_page->KeyAt(i));
      if (flag < 0) {
        return true;
      }
      if (flag == 0) {
        result->emplace_back(leaf_page->RidAt(i));
      }
    }
    auto next_page_id=leaf_page->GetNextPageId();
    while(next_page_id!=-1) {
      auto next_guard=bpm_->ReadPage(next_page_id);
      auto next_page=next_guard.template As<LeafPage>();
      auto next_page_size=next_page->GetSize();
      for(int i=0;i<next_page_size;++i) {
        auto flag=degraded_comparator_(key, next_page->KeyAt(i));
        if (flag < 0) {
          return true;
        }
        if (flag == 0) {
          result->emplace_back(next_page->RidAt(i));
        }
      }
      next_page_id=next_page->GetNextPageId();
    }
    return false;
  }
  /*****************************************************************************
   * INSERTION
   *****************************************************************************/
  /**
   * @brief Insert constant key & value pair into b+ tree
   *
   * if current tree is empty, start new tree, update root page id and insert
   * entry, otherwise insert into leaf page.
   *
   * @param key the key to insert
   * @param value the value associated with key
   * @return: since we only support unique key, if user try to insert duplicate
   * keys return false, otherwise return true.
   */
  INDEX_TEMPLATE_ARGUMENTS
  auto BPLUSTREE_TYPE::Insert(const KeyType &key, const ValueType &value) -> bool {
    // Declaration of context instance.
    Context ctx;
    auto root_id = GetRootPageId();
    ctx.header_page_ = bpm_->WritePage(header_page_id_);
    if (root_id == INVALID_PAGE_ID) {
      auto head_page = ctx.header_page_.value().AsMut<BPlusTreeHeaderPage>();
      head_page->root_page_id_ = bpm_->NewPage();
      auto cur_guard = bpm_->WritePage(head_page->root_page_id_);
      auto cur_page = cur_guard.AsMut<LeafPage>();
      cur_page->Init(leaf_max_size_);
      cur_page->ChangeSizeBy(1);
      cur_page->SetKeyAt(0, key);
      cur_page->SetRidAt(0, value);
      return true;
    }
    ctx.root_page_id_ = root_id;
    ctx.write_set_.emplace_back(bpm_->WritePage(ctx.root_page_id_));

    while (true) {
      auto cur_page = ctx.write_set_.back().AsMut<BPlusTreePage>();
      if (cur_page->IsLeafPage()) {
        break;
      }
      auto page = ctx.write_set_.back().AsMut<InternalPage>();
      auto page_size = page->GetSize();
      auto slot = page_size - 1;
      for (int i = 1; i < page_size; ++i) {
        if (comparator_(page->KeyAt(i), key) >= 0) {
          slot = i;
          break;
        }
      }
      if (comparator_(key, page->KeyAt(slot)) < 0) {
        --slot;
      }
      ctx.write_set_.emplace_back(bpm_->WritePage(page->ValueAt(slot)));
    }

    auto leaf_page = ctx.write_set_.back().AsMut<LeafPage>();

    if (leaf_page->GetSize() < leaf_max_size_) {
      int size = leaf_page->GetSize();
      if (comparator_(key, leaf_page->KeyAt(0)) < 0) {
        for (int i = size - 1; i >= 0; --i) {
          leaf_page->SetKeyAt(i + 1, leaf_page->KeyAt(i));
          leaf_page->SetRidAt(i + 1, leaf_page->RidAt(i));
        }
        leaf_page->SetSize(size + 1);
        leaf_page->SetKeyAt(0, key);
        leaf_page->SetRidAt(0, value);
        return true;
      }
      for (int i = size - 1; i >= 0; --i) {
        if (comparator_(key, leaf_page->KeyAt(i)) >= 0) {
          if (comparator_(leaf_page->KeyAt(i), key) == 0) {
            return false;
          }
          for (int j = size - 1; j >= i + 1; --j) {
            leaf_page->SetKeyAt(j + 1, leaf_page->KeyAt(j));
            leaf_page->SetRidAt(j + 1, leaf_page->RidAt(j));
          }
          leaf_page->SetSize(size + 1);
          leaf_page->SetKeyAt(i + 1, key);
          leaf_page->SetRidAt(i + 1, value);
          return true;
        }
      }
      return false;
    }

    std::vector<KeyType> leaf_keys;
    std::vector<ValueType> leaf_values;
    for (int i = 0; i < leaf_max_size_; ++i) {
      leaf_keys.emplace_back(leaf_page->KeyAt(i));
      leaf_values.emplace_back(leaf_page->RidAt(i));
    }
    if (comparator_(key, leaf_page->KeyAt(0)) < 0) {
      leaf_keys.insert(leaf_keys.begin(), key);
      leaf_values.insert(leaf_values.begin(), value);
    } else {
      for (int i = leaf_max_size_ - 1; i >= 0; --i) {
        if (comparator_(key, leaf_keys[i]) >= 0) {
          if (comparator_(leaf_keys[i], key) == 0) {
            return false;
          }
          leaf_keys.insert(leaf_keys.begin() + i + 1, key);
          leaf_values.insert(leaf_values.begin() + i + 1, value);
          break;
        }
      }
    }
    auto new_leaf_page_id = bpm_->NewPage();
    auto new_leaf_page_guard = bpm_->WritePage(new_leaf_page_id);
    auto new_leaf_page = new_leaf_page_guard.AsMut<LeafPage>();
    new_leaf_page->Init(leaf_max_size_);
    new_leaf_page->SetNextPageId(leaf_page->GetNextPageId());
    leaf_page->SetNextPageId(new_leaf_page_id);

    auto new_leaf_size = (leaf_max_size_ + 1) / 2;
    auto remain_leaf_size = leaf_max_size_ + 1 - new_leaf_size;
    new_leaf_page->SetSize(new_leaf_size);
    leaf_page->SetSize(remain_leaf_size);

    for (int i = 0; i < new_leaf_size; ++i) {
      new_leaf_page->SetKeyAt(i, leaf_keys[remain_leaf_size + i]);
      new_leaf_page->SetRidAt(i, leaf_values[remain_leaf_size + i]);
    }
    for (int i = 0; i < remain_leaf_size; ++i) {
      leaf_page->SetKeyAt(i, leaf_keys[i]);
      leaf_page->SetRidAt(i, leaf_values[i]);
    }

    // recursively insert in parent
    auto page_id_to_insert = new_leaf_page_id;
    auto key_to_insert = leaf_keys[remain_leaf_size];
    auto remain_page_id = ctx.write_set_.back().GetPageId();
    ctx.write_set_.pop_back();

    while (!ctx.write_set_.empty()) {
      auto cur_page = ctx.write_set_.back().AsMut<InternalPage>();
      auto position_to_insert = cur_page->ValueIndex(remain_page_id);
      auto cur_size = cur_page->GetSize();
      if (cur_size < internal_max_size_) {
        for (int i = cur_size - 1; i > position_to_insert; --i) {
          cur_page->SetKeyAt(i + 1, cur_page->KeyAt(i));
          cur_page->SetValueAt(i + 1, cur_page->ValueAt(i));
        }
        cur_page->SetSize(cur_size + 1);
        cur_page->SetKeyAt(position_to_insert + 1, key_to_insert);
        cur_page->SetValueAt(position_to_insert + 1, page_id_to_insert);
        return true;
      }

      std::vector<KeyType> internal_key;
      std::vector<page_id_t> internal_value;
      for (int i = 0; i < internal_max_size_; ++i) {
        internal_key.emplace_back(cur_page->KeyAt(i));
        internal_value.emplace_back(cur_page->ValueAt(i));
      }
      internal_key.insert(internal_key.begin() + position_to_insert + 1, key_to_insert);
      internal_value.insert(internal_value.begin() + position_to_insert + 1, page_id_to_insert);

      auto new_internal_size = (internal_max_size_ + 1) / 2;
      auto remain_internal_size = (internal_max_size_ + 1) - new_internal_size;

      auto new_internal_page_id = bpm_->NewPage();
      auto new_internal_guard = bpm_->WritePage(new_internal_page_id);
      auto new_internal_page = new_internal_guard.AsMut<InternalPage>();
      new_internal_page->Init(internal_max_size_);
      new_internal_page->SetSize(new_internal_size);
      cur_page->SetSize(remain_internal_size);
      for (int i = 0; i < new_internal_size; ++i) {
        new_internal_page->SetKeyAt(i, internal_key[remain_internal_size + i]);
        new_internal_page->SetValueAt(i, internal_value[remain_internal_size + i]);
      }
      for (int i = 0; i < remain_internal_size; ++i) {
        cur_page->SetKeyAt(i, internal_key[i]);
        cur_page->SetValueAt(i, internal_value[i]);
      }
      page_id_to_insert = new_internal_page_id;
      key_to_insert = internal_key[remain_internal_size];

      remain_page_id = ctx.write_set_.back().GetPageId();
      ctx.write_set_.pop_back();
    }

    auto new_root_id = bpm_->NewPage();
    auto new_root_guard = bpm_->WritePage(new_root_id);
    auto new_root_page = new_root_guard.AsMut<InternalPage>();
    new_root_page->Init(internal_max_size_);
    new_root_page->SetSize(2);
    new_root_page->SetKeyAt(1, key_to_insert);
    new_root_page->SetValueAt(0, ctx.root_page_id_);
    new_root_page->SetValueAt(1, page_id_to_insert);
    ctx.header_page_->AsMut<BPlusTreeHeaderPage>()->root_page_id_ = new_root_id;
    return true;
  }

  /*****************************************************************************
   * REMOVE
   *****************************************************************************/
  /**
   * @brief Delete key & value pair associated with input key
   * If current tree is empty, return immediately.
   * If not, User needs to first find the right leaf page as deletion target, then
   * delete entry from leaf page. Remember to deal with redistribute or merge if
   * necessary.
   *
   * @param key input key
   */
  INDEX_TEMPLATE_ARGUMENTS
  void BPLUSTREE_TYPE::Remove(const KeyType &key) {
    // Declaration of context instance.
    Context ctx;
    auto root_id = GetRootPageId();
    if (root_id == INVALID_PAGE_ID) {
      return;
    }
    ctx.header_page_ = bpm_->WritePage(header_page_id_);
    ctx.root_page_id_ = root_id;
    ctx.write_set_.emplace_back(bpm_->WritePage(ctx.root_page_id_));

    while (true) {
      auto cur_page = ctx.write_set_.back().AsMut<BPlusTreePage>();
      if (cur_page->IsLeafPage()) {
        break;
      }
      auto page = ctx.write_set_.back().AsMut<InternalPage>();
      auto page_size = page->GetSize();
      auto slot = page_size - 1;
      for (int i = 1; i < page_size; ++i) {
        if (comparator_(page->KeyAt(i), key) >= 0) {
          slot = i;
          break;
        }
      }
      if (comparator_(key, page->KeyAt(slot)) < 0) {
        --slot;
      }
      ctx.write_set_.emplace_back(bpm_->WritePage(page->ValueAt(slot)));
    }
    // Delete the key in leaf-page
    auto leaf_page = ctx.write_set_.back().AsMut<LeafPage>();
    auto leaf_size = leaf_page->GetSize();
    auto position = -1;
    for (int i = 0; i < leaf_size; ++i) {
      if (comparator_(key, leaf_page->KeyAt(i)) == 0) {
        position = i;
        break;
      }
    }
    if (position == -1) {
      return;
    }
    for (int i = position; i < leaf_size - 1; ++i) {
      leaf_page->SetKeyAt(i, leaf_page->KeyAt(i + 1));
      leaf_page->SetRidAt(i, leaf_page->RidAt(i + 1));
    }
    --leaf_size;
    leaf_page->SetSize(leaf_size);
    // Special case:leaf-page as root ,if it has no key, just delete whole tree
    if (ctx.root_page_id_ == ctx.write_set_.back().GetPageId()) {
      if (leaf_size == 0) {
        ctx.write_set_.back().Drop();
        bpm_->DeletePage(ctx.root_page_id_);
        ctx.header_page_->AsMut<BPlusTreeHeaderPage>()->root_page_id_ = INVALID_PAGE_ID;
      }
      return;
    }
    // If leaf-page has enough keys,just return
    if (leaf_size >= leaf_page->GetMinSize()) {
      return;
    }
    // Else we have two options: borrow or coalesce
    // First we only execute on the leaf, execution on internal page is similar
    auto leaf_parent_page = ctx.write_set_[ctx.write_set_.size() - 2].AsMut<InternalPage>();
    auto leaf_position = leaf_parent_page->ValueIndex(ctx.write_set_.back().GetPageId());
    // Borrow situation
    if (leaf_position > 0) {
      auto left_sib_pos = leaf_position - 1;
      auto left_sib_guard = bpm_->WritePage(leaf_parent_page->ValueAt(left_sib_pos));
      auto left_sib_page = left_sib_guard.template AsMut<LeafPage>();
      if (left_sib_page->GetSize() > left_sib_page->GetMinSize()) {
        auto borrowed_key = left_sib_page->KeyAt(left_sib_page->GetSize() - 1);
        auto borrowed_value = left_sib_page->RidAt(left_sib_page->GetSize() - 1);
        left_sib_page->ChangeSizeBy(-1);
        for (int i = leaf_size - 1; i >= 0; --i) {
          leaf_page->SetKeyAt(i + 1, leaf_page->KeyAt(i));
          leaf_page->SetRidAt(i + 1, leaf_page->RidAt(i));
        }
        ++leaf_size;
        leaf_page->ChangeSizeBy(1);
        leaf_page->SetKeyAt(0, borrowed_key);
        leaf_page->SetRidAt(0, borrowed_value);

        leaf_parent_page->SetKeyAt(leaf_position, borrowed_key);
        return;
      }
    }
    if (leaf_position < leaf_parent_page->GetSize() - 1) {
      auto right_sib_pos = leaf_position + 1;
      auto right_sib_guard = bpm_->WritePage(leaf_parent_page->ValueAt(right_sib_pos));
      auto right_sib_page = right_sib_guard.template AsMut<LeafPage>();
      auto right_sib_size = right_sib_page->GetSize();
      if (right_sib_page->GetSize() > right_sib_page->GetMinSize()) {
        auto borrowed_key = right_sib_page->KeyAt(0);
        auto borrowed_value = right_sib_page->RidAt(0);
        for (int i = 0; i < right_sib_size - 1; ++i) {
          right_sib_page->SetKeyAt(i, right_sib_page->KeyAt(i + 1));
          right_sib_page->SetRidAt(i, right_sib_page->RidAt(i + 1));
        }
        --right_sib_size;
        right_sib_page->SetSize(right_sib_size);
        ++leaf_size;
        leaf_page->SetSize(leaf_size);
        leaf_page->SetKeyAt(leaf_size - 1, borrowed_key);
        leaf_page->SetRidAt(leaf_size - 1, borrowed_value);
        leaf_parent_page->SetKeyAt(right_sib_pos, right_sib_page->KeyAt(0));
        return;
      }
    }
    // Coalesce situation
    auto position_to_delete = leaf_position;
    std::vector<KeyType> leaf_keys;
    std::vector<ValueType> leaf_values;
    if (leaf_position > 0) {
      auto left_sib_pos = leaf_position - 1;
      auto left_sib_guard = bpm_->WritePage(leaf_parent_page->ValueAt(left_sib_pos));
      auto left_sib_page = left_sib_guard.template AsMut<LeafPage>();
      auto left_sib_size = left_sib_page->GetSize();
      if (left_sib_size + leaf_size <= left_sib_page->GetMaxSize()) {
        for (auto i = 0; i < left_sib_size; ++i) {
          leaf_keys.emplace_back(left_sib_page->KeyAt(i));
          leaf_values.emplace_back(left_sib_page->RidAt(i));
        }
        for (auto i = 0; i < leaf_size; ++i) {
          leaf_keys.emplace_back(leaf_page->KeyAt(i));
          leaf_values.emplace_back(leaf_page->RidAt(i));
        }
        left_sib_size += leaf_size;
        left_sib_page->SetSize(left_sib_size);
        for (auto i = 0; i < left_sib_size; ++i) {
          left_sib_page->SetKeyAt(i, leaf_keys[i]);
          left_sib_page->SetRidAt(i, leaf_values[i]);
        }
      }
      left_sib_page->SetNextPageId(leaf_page->GetNextPageId());
      bpm_->DeletePage(leaf_parent_page->ValueAt(leaf_position));
    } else {
      position_to_delete = leaf_position + 1;

      auto right_sib_pos = leaf_position + 1;
      auto right_sib_guard = bpm_->WritePage(leaf_parent_page->ValueAt(right_sib_pos));
      auto right_sib_page = right_sib_guard.template AsMut<LeafPage>();
      auto right_sib_size = right_sib_page->GetSize();
      for (auto i = 0; i < leaf_size; ++i) {
        leaf_keys.emplace_back(leaf_page->KeyAt(i));
        leaf_values.emplace_back(leaf_page->RidAt(i));
      }
      for (auto i = 0; i < right_sib_size; ++i) {
        leaf_keys.emplace_back(right_sib_page->KeyAt(i));
        leaf_values.emplace_back(right_sib_page->RidAt(i));
      }

      leaf_size += right_sib_size;
      leaf_page->SetSize(leaf_size);
      for (auto i = 0; i < leaf_size; ++i) {
        leaf_page->SetKeyAt(i, leaf_keys[i]);
        leaf_page->SetRidAt(i, leaf_values[i]);
      }
      leaf_page->SetNextPageId(right_sib_page->GetNextPageId());
      bpm_->DeletePage(leaf_parent_page->ValueAt(right_sib_pos));
    }
    ctx.write_set_.pop_back();

    while (!ctx.write_set_.empty()) {
      auto cur_page = ctx.write_set_.back().template AsMut<InternalPage>();
      auto cur_size = cur_page->GetSize();
      // First delete the invalid key
      for (int i = position_to_delete; i < cur_size - 1; ++i) {
        cur_page->SetKeyAt(i, cur_page->KeyAt(i + 1));
        cur_page->SetValueAt(i, cur_page->ValueAt(i + 1));
      }
      --cur_size;
      cur_page->SetSize(cur_size);
      if (ctx.write_set_.back().GetPageId() == ctx.root_page_id_) {
        if (cur_size == 1) {
          ctx.header_page_->AsMut<BPlusTreeHeaderPage>()->root_page_id_ = cur_page->ValueAt(0);
          bpm_->DeletePage(ctx.root_page_id_);
        }
        return;
      }
      if (cur_size >= cur_page->GetMinSize()) {
        return;
      }
      auto cur_parent_page = ctx.write_set_[ctx.write_set_.size() - 2].AsMut<InternalPage>();
      auto cur_position = cur_parent_page->ValueIndex(ctx.write_set_.back().GetPageId());
      // Borrow situation
      // Note: If we can borrow we can immediately return
      if (cur_position > 0) {
        auto left_sib_pos = cur_position - 1;
        auto left_sib_guard = bpm_->WritePage(cur_parent_page->ValueAt(left_sib_pos));
        auto left_sib_page = left_sib_guard.template AsMut<InternalPage>();
        if (left_sib_page->GetSize() > left_sib_page->GetMinSize()) {
          // Node:We will borrow parent key in this part and replace it by the key we deleted to keep balance
          auto update_key = left_sib_page->KeyAt(left_sib_page->GetSize() - 1);

          auto borrowed_key = cur_parent_page->KeyAt(cur_position);
          auto borrowed_value = left_sib_page->ValueAt(left_sib_page->GetSize() - 1);
          left_sib_page->ChangeSizeBy(-1);
          for (int i = cur_size - 1; i >= 0; --i) {
            cur_page->SetKeyAt(i + 1, cur_page->KeyAt(i));
            cur_page->SetValueAt(i + 1, cur_page->ValueAt(i));
          }
          ++cur_size;
          cur_page->ChangeSizeBy(1);
          cur_page->SetKeyAt(1, borrowed_key);
          cur_page->SetValueAt(0, borrowed_value);
          cur_parent_page->SetKeyAt(cur_position, update_key);
          return;
        }
      }
      if (cur_position < cur_parent_page->GetSize() - 1) {
        auto right_sib_pos = cur_position + 1;
        auto right_sib_guard = bpm_->WritePage(cur_parent_page->ValueAt(right_sib_pos));
        auto right_sib_page = right_sib_guard.template AsMut<InternalPage>();
        auto right_sib_size = right_sib_page->GetSize();
        if (right_sib_page->GetSize() > right_sib_page->GetMinSize()) {
          // Node:We will borrow parent key in this part and replace it by the key we deleted to keep balance
          auto update_key = right_sib_page->KeyAt(1);

          auto borrowed_key = cur_parent_page->KeyAt(right_sib_pos);
          auto borrowed_value = right_sib_page->ValueAt(0);
          for (int i = 0; i < right_sib_size - 1; ++i) {
            right_sib_page->SetKeyAt(i, right_sib_page->KeyAt(i + 1));
            right_sib_page->SetValueAt(i, right_sib_page->ValueAt(i + 1));
          }
          --right_sib_size;
          right_sib_page->ChangeSizeBy(-1);
          ++cur_size;
          cur_page->ChangeSizeBy(1);
          cur_page->SetKeyAt(cur_size - 1, borrowed_key);
          cur_page->SetValueAt(cur_size - 1, borrowed_value);
          cur_parent_page->SetKeyAt(right_sib_pos, update_key);
          return;
        }
      }

      std::vector<KeyType> internal_keys;
      std::vector<page_id_t> internal_values;
      if (cur_position > 0) {
        position_to_delete = cur_position;
        auto left_sib_pos = cur_position - 1;
        auto left_sib_guard = bpm_->WritePage(cur_parent_page->ValueAt(left_sib_pos));
        auto left_sib_page = left_sib_guard.template AsMut<InternalPage>();
        auto left_sib_size = left_sib_page->GetSize();
        if (left_sib_size + cur_size <= left_sib_page->GetMaxSize()) {
          for (int i = 0; i < left_sib_size; ++i) {
            internal_keys.emplace_back(left_sib_page->KeyAt(i));
            internal_values.emplace_back(left_sib_page->ValueAt(i));
          }
          internal_keys.emplace_back(cur_parent_page->KeyAt(cur_position));
          internal_values.emplace_back(cur_page->ValueAt(0));
          for (int i = 1; i < cur_size; ++i) {
            internal_keys.emplace_back(cur_page->KeyAt(i));
            internal_values.emplace_back(cur_page->ValueAt(i));
          }
          left_sib_size += cur_size;
          left_sib_page->SetSize(left_sib_size);
          for (int i = 0; i < left_sib_size; ++i) {
            left_sib_page->SetKeyAt(i, internal_keys[i]);
            left_sib_page->SetValueAt(i, internal_values[i]);
          }
          bpm_->DeletePage(cur_parent_page->ValueAt(cur_position));
        }
      } else {
        position_to_delete = cur_position + 1;
        auto right_sib_pos = cur_position + 1;
        auto right_sib_guard = bpm_->WritePage(cur_parent_page->ValueAt(right_sib_pos));
        auto right_sib_page = right_sib_guard.template AsMut<InternalPage>();
        auto right_sib_size = right_sib_page->GetSize();
        for (int i = 0; i < cur_size; ++i) {
          internal_keys.emplace_back(cur_page->KeyAt(i));
          internal_values.emplace_back(cur_page->ValueAt(i));
        }
        internal_keys.emplace_back(cur_parent_page->KeyAt(right_sib_pos));
        internal_values.emplace_back(right_sib_page->ValueAt(0));
        for (int i = 1; i < right_sib_size; ++i) {
          internal_keys.emplace_back(right_sib_page->KeyAt(i));
          internal_values.emplace_back(right_sib_page->ValueAt(i));
        }
        cur_size += right_sib_size;
        cur_page->SetSize(cur_size);
        for (int i = 0; i < cur_size; ++i) {
          cur_page->SetKeyAt(i, internal_keys[i]);
          cur_page->SetValueAt(i, internal_values[i]);
        }
        bpm_->DeletePage(cur_parent_page->ValueAt(right_sib_pos));
      }
      ctx.write_set_.pop_back();
    }
  }

  /**
   * @return Page id of the root of this tree
   */
  INDEX_TEMPLATE_ARGUMENTS
  auto BPLUSTREE_TYPE::GetRootPageId() -> page_id_t {
    return bpm_->ReadPage(header_page_id_).As<BPlusTreeHeaderPage>()->root_page_id_;
  }

  template class BPlusTree<Key, int, Comparator, DegradedComparator>;
} // namespace sjtu
