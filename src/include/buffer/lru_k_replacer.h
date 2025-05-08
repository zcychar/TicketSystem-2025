#pragma once

#include <limits>
#include <mutex>  // NOLINT
#include <optional>

#include "common/config.h"
#include "common/vector.h"
#include "common/map.h"
#include "common/list.h"

namespace sjtu {
  enum class AccessType { Unknown = 0, Lookup, Scan, Index };

  class LRUKNode {
  private:
    /** History of last seen K timestamps of this page. Least recent timestamp stored in front. */
    // Remove maybe_unused if you start using them. Feel free to change the member variables as you want.

    sjtu::list<size_t> history_;
    size_t k_;
    frame_id_t fid_;
    sjtu::list<frame_id_t>::iterator place_;
    bool is_evictable_{false};

  public:
    LRUKNode(size_t first_stamp, size_t k, frame_id_t fid) : k_(k), fid_(fid) { history_.push_back(first_stamp); };

    auto GetId() -> frame_id_t { return fid_; }

    void Access(const size_t timestamp_) {
      history_.push_back(timestamp_);
      if (history_.size() > k_) {
        history_.pop_front();
      }
    }

    auto Distance() -> size_t {
      if (history_.size() >= k_) {
        return history_.front();
      }
      return 0;
    }

    auto Place() -> sjtu::list<frame_id_t>::iterator { return place_; }
    void Replace(const sjtu::list<frame_id_t>::iterator &place) { place_ = place; }
    void Setevictable(bool status) { is_evictable_ = status; }
    auto Evictable() -> bool { return is_evictable_; }
  };

  /**
   * LRUKReplacer implements the LRU-k replacement policy.
   *
   * The LRU-k algorithm evicts a frame whose backward k-distance is maximum
   * of all frames. Backward k-distance is computed as the difference in time between
   * current timestamp and the timestamp of kth previous access.
   *
   * A frame with less than k historical references is given
   * +inf as its backward k-distance. When multiple frames have +inf backward k-distance,
   * classical LRU algorithm is used to choose victim.
   */
  class LRUKReplacer {
  public:
    explicit LRUKReplacer(size_t num_frames, size_t k);

    LRUKReplacer(const LRUKReplacer &) = delete;

    auto operator=(const LRUKReplacer &) -> LRUKReplacer & = delete;

    LRUKReplacer(LRUKReplacer &&) = delete;

    auto operator=(LRUKReplacer &&) -> LRUKReplacer & = delete;

    /**
     *
     * @brief Destroys the LRUReplacer.
     */
    ~LRUKReplacer() = default;

    auto Evict() -> std::optional<frame_id_t>;

    void RecordAccess(frame_id_t frame_id, AccessType access_type = AccessType::Unknown);

    void SetEvictable(frame_id_t frame_id, bool set_evictable);

    void Remove(frame_id_t frame_id);

    auto Size() -> size_t;

  private:
    // Remove maybe_unused if you start using them.
    sjtu::map<frame_id_t, LRUKNode> node_store_;
    size_t current_timestamp_{0};
    size_t curr_size_{0};
    size_t replacer_size_;
    size_t k_;
    sjtu::list<frame_id_t> history_list_;
    sjtu::list<frame_id_t> cache_list_;
    // std::mutex latch_;
  };
} // namespace sjtu
