#include "disk/disk_scheduler.h"
#include "disk/disk_manager.h"

namespace sjtu {

DiskScheduler::DiskScheduler(DiskManager *disk_manager) : disk_manager_(disk_manager) {
  // Spawn the background thread
  background_thread_.emplace([&] { StartWorkerThread(); });
}

DiskScheduler::~DiskScheduler() {
  // Put a `std::nullopt` in the queue to signal to exit the loop
  request_queue_.Put(std::nullopt);
  if (background_thread_.has_value()) {
    background_thread_->join();
  }
}

/**
 *
 * @brief Schedules a request for the DiskManager to execute.
 *
 * @param r The request to be scheduled.
 */
void DiskScheduler::Schedule(DiskRequest r) { request_queue_.Put(std::optional<DiskRequest>(std::move(r))); }

/**
 *
 * @brief Background worker thread function that processes scheduled requests.
 *
 * The background thread needs to process requests while the DiskScheduler exists, i.e., this function should not
 * return until ~DiskScheduler() is called. At that point you need to make sure that the function does return.
 */
void DiskScheduler::StartWorkerThread() {
  while (true) {
    auto front = request_queue_.Get();
    if (!front.has_value()) {
      break;
    }
    if (front.value().is_write_) {
      disk_manager_->WritePage(front.value().page_id_, front.value().data_);
      front.value().callback_.set_value(true);
    } else {
      disk_manager_->ReadPage(front.value().page_id_, front.value().data_);
      front.value().callback_.set_value(true);
    }
  }
}

}  // namespace sjtu
