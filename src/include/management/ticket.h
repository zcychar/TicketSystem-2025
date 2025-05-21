#pragma once

#include "common/config.h"
#include "common/util.h"
#include "storage/b_plus_tree.h"
#include "user.h"

namespace sjtu {
class Train;
using TrainDate = std::pair<hash_t, num_t>; // trainID_hash && date
using OrderTime = std::pair<hash_t, int>; // username_hash && timestamp
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

  num_t getSeat(num_t from, num_t to) {
    auto minm = seatNum[from];
    for (int i = from + 1; i <= to; ++i) {
      minm = std::min(seatNum[i], minm);
    }
    return minm;
  }

  void changeSeat(num_t from, num_t to, num_t fig) {
    for (int i = from; i <= to; ++i) {
      seatNum[i] += fig;
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
// searched by train_ID,should check avaibility then change OrderInfo::status

struct StationTrainInfo {
  hash_t trainID_hash;
  char trainID[20]{};
  num_t station_index;
  int price;
  DateRange saleDate;
  DateTime arrivingTime;
  DateTime leavingTime;

  StationTrainInfo() = delete;

  StationTrainInfo(hash_t hash, const char ID[], num_t index, int price,
                   DateRange date, DateTime a_time,
                   DateTime l_time): trainID_hash(hash), station_index(index),
                                     price(price), saleDate(date),
                                     arrivingTime(a_time),
                                     leavingTime(l_time) {
    strncpy(trainID, ID, 20);
  }


  StationTrainInfo(const StationTrainInfo &other): arrivingTime(
                                                       other.arrivingTime),
                                                   leavingTime(
                                                       other.leavingTime) {
    trainID_hash = other.trainID_hash;
    strncpy(trainID, other.trainID, 20);
    station_index = other.station_index;
    price = other.price;
    saleDate = other.saleDate;
  }

  StationTrainInfo &operator=(const StationTrainInfo &other) {
    trainID_hash = other.trainID_hash;
    strncpy(trainID, other.trainID, 20);
    station_index = other.station_index;
    price = other.price;
    saleDate = other.saleDate;
    arrivingTime = other.arrivingTime;
    leavingTime = other.leavingTime;
    return *this;
  }
}; // 48 bytes ,changed when release

struct TicketComp {
  num_t time;
  int cost;
  num_t station_index_1;
  num_t station_index_2;
  DateTime leavingTime;
  DateTime arrivingTime;
  hash_t trainID_hash;
  char trainID[20]{};

  TicketComp(num_t time, int cost, num_t index_1, num_t index_2,
             DateTime l_time, DateTime a_time, hash_t hash,
             char ID[]): time(time),
                         cost(cost), station_index_1(index_1),
                         station_index_2(index_2), leavingTime(l_time),
                         arrivingTime(a_time), trainID_hash(hash) {
    strncpy(trainID, ID, 20);
  }
}; //56bytes

struct TicketTransComp {
  num_t time;
  int cost;
  char station[30]{};
  TicketComp ticket_1;
  TicketComp ticket_2;

  TicketTransComp(TicketComp &ticket_1,
                  TicketComp &ticket_2, const char st[]): ticket_1(ticket_1),
    ticket_2(ticket_2) {
    time = ticket_2.arrivingTime - ticket_1.leavingTime;
    cost = ticket_1.cost + ticket_2.cost;
    strncpy(station, st, 30);
  }
};


struct SortByTime {
  bool operator()(TicketComp &lhs, TicketComp &rhs) {
    if (lhs.time != rhs.time) {
      return lhs.time < rhs.time;
    }
    return strcmp(lhs.trainID, rhs.trainID) < 0;
  }
};

struct SortByCost {
  bool operator()(TicketComp &lhs, TicketComp &rhs) {
    if (lhs.cost != rhs.cost) {
      return lhs.cost < rhs.cost;
    }
    return strcmp(lhs.trainID, rhs.trainID) < 0;
  }
};

struct TranSortByTime {
  bool operator()(TicketTransComp &lhs, TicketTransComp &rhs) {
    if (lhs.time != rhs.time) {
      return lhs.time < rhs.time;
    }
    if (lhs.cost != rhs.cost) {
      return lhs.cost < rhs.cost;
    }
    auto flag_1 = strcmp(lhs.ticket_1.trainID, rhs.ticket_1.trainID);
    if (flag_1 != 0) {
      return flag_1 < 0;
    }
    return strcmp(lhs.ticket_2.trainID, rhs.ticket_2.trainID) < 0;
  }
};

struct TranSortByCost {
  bool operator()(TicketTransComp &lhs, TicketTransComp &rhs) {
    if (lhs.cost != rhs.cost) {
      return lhs.cost < rhs.cost;
    }
    if (lhs.time != rhs.time) {
      return lhs.time < rhs.time;
    }
    auto flag_1 = strcmp(lhs.ticket_1.trainID, rhs.ticket_1.trainID);
    if (flag_1 != 0) {
      return flag_1 < 0;
    }
    return strcmp(lhs.ticket_2.trainID, rhs.ticket_2.trainID) < 0;
  }
};

inline bool TransferPossible(const DateTime arr_1, const DateTime &lft_1, const DateTime &arr_2,
                             const DateTime &lft_2) -> bool {
  return arr_2 <= lft_1 && arr_1 <= lft_2;
}

class Ticket {
  friend Train;

public:
  Ticket(std::string &name, User *user);

  void QueryTicket(std::string &from, std::string &to, num_t date,
                   std::string comp = "time");

  void QueryTransfer(Train *train, std::string &from, std::string &to,
                     num_t date,
                     std::string comp = "time");

  void BuyTicket(std::string &username, std::string &trainID, num_t date,
                 num_t num, std::string &from, std::string &to,
                 std::string queue = "false");

  void QueryOrder(std::string &username);

  void RefundTicket(std::string &username, int n = 1);

private:
  std::unique_ptr<BPlusTree<TrainDate, TicketDateInfo, PairCompare<TrainDate>,
                            PairDegradedCompare<TrainDate> > >
  ticket_db_;

  std::unique_ptr<BPlusTree<OrderTime, OrderInfo, PairCompare<OrderTime>,
                            PairDegradedCompare<OrderTime> > >
  order_db_;

  std::unique_ptr<BPlusTree<TrainDate, PendingInfo, PairCompare<TrainDate>,
                            PairCompare<TrainDate> > >
  pending_db_;

  std::unique_ptr<BPlusTree<hash_t, StationTrainInfo, HashComp, HashComp> >
  station_db_;

  User *user_;
};
} // namespace sjtu