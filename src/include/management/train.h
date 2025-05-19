#pragma once

#include <cstring>
#include "storage/b_plus_tree.h"
#include "buffer/buffer_pool_manager.h"
#include "common/config.h"
#include "management/ticket.h"


namespace sjtu {
  struct TrainInfo {
    hash_t train_id_hash;
    char trainID[20];
    num_t stationNum;
    char stations[100][30];
    num_t seatNum;
    int prices[99];
    num_t startTime;
    num_t travelTimes[99];
    num_t stopoverTimes[98];
    DateRange saleDate;
    char type;

    TrainInfo &operator=(const TrainInfo &other) {
      train_id_hash = other.train_id_hash;
      stationNum = other.stationNum;
      seatNum = other.seatNum;
      startTime = other.startTime;
      strcpy(trainID, other.trainID);
      memcpy(prices, other.prices, (stationNum - 1) * sizeof(num_t));
      memcpy(travelTimes, other.travelTimes, (stationNum - 1) * sizeof(num_t));
      memcpy(stopoverTimes, other.stopoverTimes, (stationNum - 2) * sizeof(num_t));
      return *this;
    }
  }; //3832 bytes, stored as single page
  //Static info only

  struct TrainMeta {
    page_id_t page_id;
    DateRange saleDate;
    bool is_released = false;
  };


  class Train {
  public:
    explicit Train(std::string &name);

    ~Train();

    auto AddTrain(TrainInfo &train) -> bool;

    auto DeleteTrain(std::string &trainID) -> bool;

    void QueryTrain(std::string &trainID, num_t date);

    auto ReleaseTrain(std::string &trainID) -> bool;

  private:
    std::unique_ptr<BufferPoolManager> train_manager_;

    std::unique_ptr<BPlusTree<hash_t, TrainMeta, HashComp, HashComp> > train_db_;

    Ticket *ticket_;
  };
}
