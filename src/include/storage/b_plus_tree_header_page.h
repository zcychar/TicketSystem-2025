#pragma once

#include "common/config.h"

namespace sjtu {
  /**
   * The header page is just used to retrieve the root page,
   * preventing potential race condition under concurrent environment.
   */
  class BPlusTreeHeaderPage {
  public:
    // Delete all constructor / destructor to ensure memory safety
    BPlusTreeHeaderPage() = delete;

    BPlusTreeHeaderPage(const BPlusTreeHeaderPage &other) = delete;

    page_id_t next_page_id_;

    page_id_t root_page_id_;
  };
} // namespace sjtu
