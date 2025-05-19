#pragma once

#include "user.h"
#include "common/config.h"
#include "common/util.h"
#include "storage/b_plus_tree.h"

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
    char trainID[20];
    int price;
    DateRange saleDate;
    DateTime arrivingTime;
    DateTime leavingTime;
  }; // 48 bytes ,changed when release

  struct TicketComp {


  };

  class Ticket {
    friend Train;

  public:
    Ticket(std::string &name, User *user);

    void QueryTicket(std::string &from, std::string &to, num_t date, std::string comp = "time");

    void QueryTransfer(std::string &from, std::string &to, num_t date, std::string comp = "time");

    void BuyTicket(std::string &username, std::string &trainID, num_t date, num_t num, std::string &from,
                   std::string &to, std::string queue = "false");

    void QueryOrder(std::string &username);

    void RefundTicket(std::string &username, num_t n = 1);

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
