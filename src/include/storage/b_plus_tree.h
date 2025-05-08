/**
 * b_plus_tree.h
 *
 * Implementation of simple b+ tree data structure where internal pages direct
 * the search and leaf pages contain actual data.
 * (1) We only support unique key
 * (2) support insert & remove
 * (3) The structure should shrink and grow dynamically
 * (4) Implement index iterator for range scan
 */
#pragma once

#include <algorithm>
#include <deque>
#include <filesystem>
#include <iostream>
#include <optional>
#include <shared_mutex>
#include <string>

#include "common/config.h"
#include "storage/b_plus_tree_header_page.h"
#include "storage/b_plus_tree_internal_page.h"
#include "storage/b_plus_tree_leaf_page.h"
#include "storage/page_guard.h"
#include "common/container.h"
#include "common/vector.h"

namespace sjtu {
  struct PrintableBPlusTree;

  /**
   * @brief Definition of the Context class.
   *
   * Hint: This class is designed to help you keep track of the pages
   * that you're modifying or accessing.
   */
  class Context {
  public:
    // When you insert into / remove from the B+ tree, store the write guard of header page here.
    // Remember to drop the header page guard and set it to nullopt when you want to unlock all.
    std::optional<WritePageGuard> header_page_{std::nullopt};

    // Save the root page id here so that it's easier to know if the current page is the root page.
    page_id_t root_page_id_{INVALID_PAGE_ID};

    // Store the write guards of the pages that you're modifying here.
    std::deque<WritePageGuard> write_set_;

    // You may want to use this when getting value, but not necessary.
    std::deque<ReadPageGuard> read_set_;

    auto IsRootPage(page_id_t page_id) -> bool { return page_id == root_page_id_; }
  };

#define BPLUSTREE_TYPE BPlusTree<KeyType, ValueType, KeyComparator,DegradedKeyComparator>

  // Main class providing the API for the Interactive B+ Tree.
  INDEX_TEMPLATE_ARGUMENTS
  class BPlusTree {
    using InternalPage = BPlusTreeInternalPage<KeyType, page_id_t, KeyComparator, DegradedKeyComparator>;
    using LeafPage = BPlusTreeLeafPage<KeyType, ValueType, KeyComparator, DegradedKeyComparator>;

  public:
    explicit BPlusTree(std::string name, page_id_t header_page_id, BufferPoolManager *buffer_pool_manager,
                       const KeyComparator &comparator, const DegradedKeyComparator &degraded_comparator,
                       int leaf_max_size = LEAF_PAGE_SLOT_CNT,
                       int internal_max_size = INTERNAL_PAGE_SLOT_CNT);

    // Returns true if this B+ tree has no keys and values.
    auto IsEmpty() const -> bool;

    // Insert a key-value pair into this B+ tree.
    auto Insert(const KeyType &key, const ValueType &value) -> bool;

    // Remove a key and its value from this B+ tree.
    void Remove(const KeyType &key);

    // Return the value associated with a given key
    auto GetValue(const KeyType &key, sjtu::vector<ValueType> *result) -> bool;

    // Return all the value associated with a given key
    auto GetAllValue(const KeyType &key, sjtu::vector<ValueType> *result) -> bool;

    // Return the page id of the root node
    auto GetRootPageId() -> page_id_t;

    void Print(BufferPoolManager *bpm) {
      auto root_page_id = GetRootPageId();
      if (root_page_id != INVALID_PAGE_ID) {
        auto guard = bpm->ReadPage(root_page_id);
        PrintTree(guard.GetPageId(), guard.template As<BPlusTreePage>());
      }
    }

    void PrintTree(page_id_t page_id, const BPlusTreePage *page) {
      if (page->IsLeafPage()) {
        auto *leaf = reinterpret_cast<const LeafPage *>(page);
        std::cout << "Leaf Page: " << page_id << "\tNext: " << leaf->GetNextPageId() << std::endl;

        // Print the contents of the leaf page.
        std::cout << "Contents: ";
        for (int i = 0; i < leaf->GetSize(); i++) {
          std::cout << leaf->KeyAt(i);
          if ((i + 1) < leaf->GetSize()) {
            std::cout << ", ";
          }
        }
        std::cout << std::endl;
        std::cout << std::endl;
      } else {
        auto *internal = reinterpret_cast<const InternalPage *>(page);
        std::cout << "Internal Page: " << page_id << std::endl;

        // Print the contents of the internal page.
        std::cout << "Contents: ";
        for (int i = 0; i < internal->GetSize(); i++) {
          if (i == 0) {
            std::cout << internal->ValueAt(i);
          } else {
            std::cout << internal->KeyAt(i) << ": " << internal->ValueAt(i);
          }
          if ((i + 1) < internal->GetSize()) {
            std::cout << ", ";
          }
        }
        std::cout << std::endl;
        std::cout << std::endl;
        for (int i = 0; i < internal->GetSize(); i++) {
          auto guard = bpm_->ReadPage(internal->ValueAt(i));
          PrintTree(guard.GetPageId(), guard.template As<BPlusTreePage>());
        }
      }
    }

  private:
    // member variable
    std::string index_name_;
    BufferPoolManager *bpm_;
    KeyComparator comparator_;
    DegradedKeyComparator degraded_comparator_;
    std::vector<std::string> log; // NOLINT
    int leaf_max_size_;
    int internal_max_size_;
    page_id_t header_page_id_;
  };
} // namespace bustub
