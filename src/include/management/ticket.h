#pragma once

#include "common/config.h"
#include "common/util.h"
#include "storage/b_plus_tree.h"
#include "user.h"

namespace sjtu {
class Train;
using TrainDate = std::pair<hash_t, num_t>; // trainID_hash && date
using OrderTime = std::pair<hash_t, int>;   // username_hash && timestamp
using TrainDate = std::pair<hash_t, num_t>;

enum class TicketStatus { Success, Pending, Refunded };

struct TicketDateInfo {
  num_t stationNum = 0;
  num_t seatNum[99] = {};

  TicketDateInfo(num_t seatnum, num_t stationnum) : stationNum(stationnum) {
    for (int i = 0; i < stationNum - 1; ++i) {
      seatNum[i] = seatnum;
    }
  }

  num_t getSeat(num_t from,num_t to){
    auto minm=seatNum[from];
    for(int i=from+1;i<=to;++i){
      minm=std::min(seatNum[i],minm);
    }
    return minm;
  }

  void changeSeat(num_t from,num_t to,num_t fig){
    for(int i=from;i<=to;++i){
      seatNum[i]+=fig;
    }
  }
}; // 200bytes,changed when release

struct OrderInfo {
  TicketStatus status;
  char trainID[20];
  char from[30];
  char to[30];
  DateTime leavingTime;
  DateTime arrivingTime;
  int price;
  num_t num;
}; // 100bytes, static info besides status

struct PendingInfo {
  int timestamp;
  hash_t username_hash;
  hash_t trainID_hash;
  hash_t from;
  hash_t to;
  num_t num;
}; // 48 bytes
// searched by train_ID,l,,,,,should check avaibility then change OrderInfo::status

struct StationTrainInfo {
  hash_t trainID_hash;
  char trainID[20];
  num_t station_index;
  int price;
  DateRange saleDate;
  DateTime arrivingTime;
  DateTime leavingTime;
}; // 48 bytes ,changed when release

struct TicketComp {
  num_t time;
  int cost;
  num_t station_index_1;
  num_t station_index_2;
  DateTime leavingTime;
  DateTime arrivingTime;
  hash_t trainID_hash;
  char trainID[20];
};//56bytes

struct SortByTime{
  bool operator()(TicketComp& lhs,TicketComp& rhs){
    return lhs.time<rhs.time;
  }
};

struct SortByCost{
  bool operator()(TicketComp& lhs,TicketComp& rhs){
    return lhs.cost<rhs.cost;
  }
};

class Ticket {
  friend Train;

public:
  Ticket(std::string &name, User *user);

  void QueryTicket(std::string &from, std::string &to, num_t date,
                   std::string comp = "time");

  void QueryTransfer(std::string &from, std::string &to, num_t date,
                     std::string comp = "time");

  void BuyTicket(std::string &username, std::string &trainID, num_t date,
                 num_t num, std::string &from, std::string &to,
                 std::string queue = "false");

  void QueryOrder(std::string &username);

  void RefundTicket(std::string &username, num_t n = 1);

private:
  std::unique_ptr<BPlusTree<TrainDate, TicketDateInfo, PairCompare<TrainDate>,
                            PairDegradedCompare<TrainDate>>>
      ticket_db_;

  std::unique_ptr<BPlusTree<OrderTime, OrderInfo, PairCompare<OrderTime>,
                            PairDegradedCompare<OrderTime>>>
      order_db_;

  std::unique_ptr<BPlusTree<TrainDate, PendingInfo, PairCompare<TrainDate>,
                            PairCompare<TrainDate>>>
      pending_db_;

  std::unique_ptr<BPlusTree<hash_t, StationTrainInfo, HashComp, HashComp>>
      station_db_;

  User *user_;
};
} // namespace sjtu
