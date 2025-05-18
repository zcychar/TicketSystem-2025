#pragma once

#include "storage/b_plus_tree.h"
#include "buffer/buffer_pool_manager.h"
#include "common/config.h"


namespace sjtu {
  struct TrainInfo {
    hash_t train_id_hash;
    char trainid[21];
    num_t stationNum;
    char stations[101][41];
    hash_t stations_hash[101];
    num_t seatNum;
    num_t prices[100];
    num_t startTime;
    num_t travelTimes[100];
    num_t stopoverTimes[100];
    DateRange saleDate;
    char type;
  }; //5589 bytes, stored as single page

  struct


  class Train {
  private:
    BufferPoolManager train_manager_;
  };
}
