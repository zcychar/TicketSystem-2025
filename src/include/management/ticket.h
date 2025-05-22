#pragma once

#include "common/config.h"
#include "common/util.h"
#include "storage/b_plus_tree.h"
#include "user.h"

namespace sjtu {
class Train;
using TrainDate = std::pair<hash_t, num_t>; // trainID_hash && date
using OrderTime = std::pair<hash_t, int>; // username_hash && timestamp
using TrainDateOrder = std::pair<std::pair<hash_t, num_t>, int>;
using TrainDate = std::pair<hash_t, num_t>;
using StationTrain = std::pair<hash_t, hash_t>;

enum class TicketStatus { Success, Pending, Refunded };

struct TDOCompare {
  int operator()(const TrainDateOrder &lhs, const TrainDateOrder &rhs) const {
    if (lhs.first.first != rhs.first.first) {
      if (lhs.first.first < rhs.first.first) {
        return -1;
      }
      return 1;
    }
    if (lhs.first.second != rhs.first.second) {
      if (lhs.first.second < rhs.first.second) {
        return -1;
      }
      return 1;
    }
    if (lhs.second < rhs.second) {
      return -1;
    } else if (lhs.second > rhs.second) {
      return 1;
    }
    return 0;
  }
};

struct TDODegradedCompare {
  int operator()(const TrainDateOrder &lhs, const TrainDateOrder &rhs) const {
    if (lhs.first.first != rhs.first.first) {
      if (lhs.first.first < rhs.first.first) {
        return -1;
      }
      return 1;
    }
    if (lhs.first.second != rhs.first.second) {
      if (lhs.first.second < rhs.first.second) {
        return -1;
      }
      return 1;
    }
    return 0;
  }
};

struct TicketDateInfo {
  num_t seatMaxNum = 0;
  int seatNum[99] = {};

  TicketDateInfo(int seatnum, num_t stationnum) : seatMaxNum(seatnum) {
    for (int i = 0; i < stationnum - 1; ++i) {
      seatNum[i] = seatnum;
    }
  }

  int getSeat(num_t from, num_t to) {
    auto minm = seatNum[from];
    for (int i = from + 1; i < to; ++i) {
      minm = std::min(seatNum[i], minm);
    }
    return minm;
  }

  void changeSeat(num_t from, num_t to, int fig) {
    for (int i = from; i < to; ++i) {
      seatNum[i] += fig;
    }
  }
}; // 200bytes,changed when release

struct OrderInfo {
  int timestamp;
  TicketStatus status;
  char trainID[21]{};
  char from[31]{};
  char to[31]{};
  num_t from_index;
  num_t to_index;
  DateTime leavingTime;
  DateTime arrivingTime;
  int price;
  int num;
  num_t init_date;

  OrderInfo(int stamp, TicketStatus s, const char ID[], const char f[],
            const char t[], num_t index_1, num_t index_2, DateTime &l_time,
            DateTime &a_time, int p, int n,num_t init_d): timestamp(stamp), status(s),
                                               leavingTime(l_time),
                                               arrivingTime(a_time), price(p),
                                               num(n), from_index(index_1),
                                               to_index(index_2),init_date(init_d) {
    strncpy(trainID, ID, 20);
    strncpy(from, f, 30);
    strncpy(to, t, 30);
  }
}; // 100bytes, static info besides status

struct PendingInfo {
  int timestamp;
  hash_t username_hash;
  hash_t trainID_hash;
  num_t from_index;
  num_t to_index;
  int num;
  num_t init_date;
}; // 48 bytes
// searched by train_ID,should check avaibility then change OrderInfo::status

struct StationTrainInfo {
  hash_t trainID_hash;
  char trainID[21]{};
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
  int ticket_num;
  num_t station_index_1;
  num_t station_index_2;
  DateTime leavingTime;
  DateTime arrivingTime;
  hash_t trainID_hash;
  char trainID[21]{};

  TicketComp(num_t time, int cost,int t_num, num_t index_1, num_t index_2,
             DateTime l_time, DateTime a_time, hash_t hash,
             char ID[]): time(time),
                         cost(cost),ticket_num(t_num), station_index_1(index_1),
                         station_index_2(index_2), leavingTime(l_time),
                         arrivingTime(a_time), trainID_hash(hash) {
    strncpy(trainID, ID, 20);
  }
}; //56bytes

struct TicketTransComp {
  num_t time;
  int cost;
  char station[31]{};
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
      return lhs.time > rhs.time;
    }
    return strcmp(lhs.trainID, rhs.trainID) > 0;
  }
};

struct SortByCost {
  bool operator()(TicketComp &lhs, TicketComp &rhs) {
    if (lhs.cost != rhs.cost) {
      return lhs.cost > rhs.cost;
    }
    return strcmp(lhs.trainID, rhs.trainID) > 0;
  }
};

struct TranSortByTime {
  bool operator()(TicketTransComp &lhs, TicketTransComp &rhs) {
    if (lhs.time != rhs.time) {
      return lhs.time > rhs.time;
    }
    if (lhs.cost != rhs.cost) {
      return lhs.cost > rhs.cost;
    }
    auto flag_1 = strcmp(lhs.ticket_1.trainID, rhs.ticket_1.trainID);
    if (flag_1 != 0) {
      return flag_1 > 0;
    }
    return strcmp(lhs.ticket_2.trainID, rhs.ticket_2.trainID) > 0;
  }
};

struct TranSortByCost {
  bool operator()(TicketTransComp &lhs, TicketTransComp &rhs) {
    if (lhs.cost != rhs.cost) {
      return lhs.cost > rhs.cost;
    }
    if (lhs.time != rhs.time) {
      return lhs.time > rhs.time;
    }
    auto flag_1 = strcmp(lhs.ticket_1.trainID, rhs.ticket_1.trainID);
    if (flag_1 != 0) {
      return flag_1 > 0;
    }
    return strcmp(lhs.ticket_2.trainID, rhs.ticket_2.trainID) > 0;
  }
};

inline auto TransferPossible(const DateTime arr_1, const DateTime &lft_1,
                             const DateTime &arr_2,
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

  void BuyTicket(int timestamp, std::string &username, std::string &trainID,
                 num_t date,
                 int num, std::string &from, std::string &to,
                 std::string queue = "false");

  void QueryOrder(std::string &username);

  void RefundTicket(std::string &username, int n = 1);

private:
  //the date of train start
  std::unique_ptr<BPlusTree<TrainDate, TicketDateInfo, PairCompare<TrainDate>,
                            PairDegradedCompare<TrainDate> > >
  ticket_db_;

  std::unique_ptr<BPlusTree<OrderTime, OrderInfo, PairCompare<OrderTime>,
                            PairDegradedCompare<OrderTime> > >
  order_db_;

  //also should be the date train start
  std::unique_ptr<BPlusTree<TrainDateOrder, PendingInfo, TDOCompare,
                            TDODegradedCompare > >
  pending_db_;

  std::unique_ptr<BPlusTree<StationTrain, StationTrainInfo,
                            PairCompare<StationTrain>, PairDegradedCompare<
                              StationTrain> > >
  station_db_;

  User *user_;
};
} // namespace sjtu