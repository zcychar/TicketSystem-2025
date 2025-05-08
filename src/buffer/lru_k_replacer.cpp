#include "buffer/lru_k_replacer.h"


namespace sjtu {

/**
 *
 *
 *
 * @brief a new LRUKReplacer.
 * @param num_frames the maximum number of frames the LRUReplacer will be required to store
 */
LRUKReplacer::LRUKReplacer(size_t num_frames, size_t k) : replacer_size_(num_frames), k_(k) {}

/**
 *
 * @brief Find the frame with largest backward k-distance and evict that frame. Only frames
 * that are marked as 'evictable' are candidates for eviction.
 *
 * A frame with less than k historical references is given +inf as its backward k-distance.
 * If multiple frames have inf backward k-distance, then evict frame whose oldest timestamp
 * is furthest in the past.
 *
 * Successful eviction of a frame should decrement the size of replacer and remove the frame's
 * access history.
 *
 * @return true if a frame is evicted successfully, false if no frames can be evicted.
 */
auto LRUKReplacer::Evict() -> std::optional<frame_id_t> {
  // std::unique_lock<std::mutex> lk(latch_);
  if (curr_size_ == 0) {
    return std::nullopt;
  }
  if (!history_list_.empty()) {
    auto cur = history_list_.front();
    history_list_.pop_front();
    node_store_.erase(cur);
    curr_size_--;
    return cur;
  }
  auto fst = *cache_list_.begin();
  auto fstt = node_store_.find(fst)->second;
  size_t min_val = fstt.Distance();
  auto min_pos = cache_list_.begin();
  for (auto i = cache_list_.begin(); i != cache_list_.end(); ++i) {
    auto cur = node_store_.find(*i)->second;
    if (cur.Distance() < min_val) {
      min_pos = i;
      min_val = cur.Distance();
    }
  }
  auto cur = *min_pos;
  cache_list_.erase(min_pos);
  node_store_.erase(cur);
  curr_size_--;
  return cur;
}

/**
 * @brief Record the event that the given frame id is accessed at current timestamp.
 * Create a new entry for access history if frame id has not been seen before.
 *
 * If frame id is invalid (ie. larger than replacer_size_), throw an exception. You can
 * also use BUSTUB_ASSERT to abort the process if frame id is invalid.
 *
 * @param frame_id id of frame that received a new access.
 * @param access_type type of access that was received. This parameter is only needed for
 * leaderboard tests.
 */
void LRUKReplacer::RecordAccess(frame_id_t frame_id, [[maybe_unused]] AccessType access_type) {
  // std::unique_lock<std::mutex> lk(latch_);
  if (frame_id >= static_cast<frame_id_t>(replacer_size_) || frame_id < 0) {
    throw std::runtime_error("LRU-K_record_access");
  }
  auto cur = node_store_.find(frame_id);
  if (cur == node_store_.end()) {
    node_store_.emplace(frame_id, LRUKNode(++current_timestamp_, k_, frame_id));
    return;
  }
  auto &curr = cur->second;
  if (!curr.Evictable()) {
    curr.Access(++current_timestamp_);
    return;
  }
  if (curr.Distance() != 0) {
    curr.Access(++current_timestamp_);
    return;
  }
  curr.Access(++current_timestamp_);
  if (curr.Distance() != 0) {
    history_list_.erase(curr.Place());
    curr.Replace(cache_list_.emplace(cache_list_.end(), curr.GetId()));
  }
}

/**
 *
 * @brief Toggle whether a frame is evictable or non-evictable. This function also
 * controls replacer's size. Note that size is equal to number of evictable entries.
 *
 * If a frame was previously evictable and is to be set to non-evictable, then size should
 * decrement. If a frame was previously non-evictable and is to be set to evictable,
 * then size should increment.
 *
 * If frame id is invalid, throw an exception or abort the process.
 *
 * For other scenarios, this function should terminate without modifying anything.
 *
 * @param frame_id id of frame whose 'evictable' status will be modified
 * @param set_evictable whether the given frame is evictable or not
 */
void LRUKReplacer::SetEvictable(frame_id_t frame_id, bool set_evictable) {
  // std::unique_lock<std::mutex> lk(latch_);
  if (frame_id >= static_cast<frame_id_t>(replacer_size_) || frame_id < 0) {
    throw std::runtime_error("LRU-K_set_evitable");
  }
  auto cur = node_store_.find(frame_id);
  if (cur == node_store_.end()) {
    return;
  }
  auto &curr = cur->second;
  if (curr.Evictable() == set_evictable) {
    return;
  }
  if (!curr.Evictable() && set_evictable) {
    curr.Setevictable(set_evictable);
    if (curr.Distance() != 0) {
      curr.Replace(cache_list_.emplace(cache_list_.end(), curr.GetId()));
    } else {
      curr.Replace(history_list_.emplace(history_list_.end(), curr.GetId()));
    }
    curr_size_++;
  } else {
    curr.Setevictable(set_evictable);
    if (curr.Distance() != 0) {
      cache_list_.erase(curr.Place());
    } else {
      history_list_.erase(curr.Place());
    }
    curr_size_--;
  }
}

/**
 *
 *
 * @brief Remove an evictable frame from replacer, along with its access history.
 * This function should also decrement replacer's size if removal is successful.
 *
 * Note that this is different from evicting a frame, which always remove the frame
 * with largest backward k-distance. This function removes specified frame id,
 * no matter what its backward k-distance is.
 *
 * If Remove is called on a non-evictable frame, throw an exception or abort the
 * process.
 *
 * If specified frame is not found, directly return from this function.
 *
 * @param frame_id id of frame to be removed
 */
void LRUKReplacer::Remove(frame_id_t frame_id) {
  // std::unique_lock<std::mutex> lk(latch_);
  if (frame_id >= static_cast<frame_id_t>(replacer_size_) || frame_id < 0) {
    throw std::runtime_error("LRU-K_remove");
  }
  auto cur = node_store_.find(frame_id);
  if (cur == node_store_.end()) {
    return;
  }
  auto &curr = cur->second;
  if (!curr.Evictable()) {
    throw std::runtime_error("LRU-K_remove");
  }
  if (curr.Distance() != 0) {
    cache_list_.erase(curr.Place());
  } else {
    history_list_.erase(curr.Place());
  }
  node_store_.erase(cur);
  curr_size_--;
}

/**
 *
 *
 * @brief Return replacer's size, which tracks the number of evictable frames.
 *
 * @return size_t
 */
auto LRUKReplacer::Size() -> size_t { return curr_size_; }

}  // namespace sjtu
