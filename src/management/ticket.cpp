#include "management/ticket.h"

#include <cstring>

#include "common/priority_queue.hpp"
#include "management/train.h"

namespace sjtu {
struct SortByCost;
struct SortByTime;

Ticket::Ticket(std::string &name, User *user) : user_(user) {
  HashComp hashcomp;
  PairCompare<TrainDate> tdcomp;
  PairDegradedCompare<TrainDate> tdcomp_d;
  PairCompare<OrderTime> odcomp;
  PairDegradedCompare<OrderTime> odcomp_d;
  PairCompare<StationTrain> stcomp;
  PairDegradedCompare<StationTrain> stcomp_d;
  TDOCompare tdocomp;
  TDODegradedCompare tdocomp_d;
  ticket_db_ = std::make_unique<
      BPlusTree<TrainDate, TicketDateInfo, PairCompare<TrainDate>,
                PairDegradedCompare<TrainDate> > >(name + "_ticket_db", tdcomp,
                                                   tdcomp_d, 256);
  order_db_ =
      std::make_unique<BPlusTree<OrderTime, OrderInfo, PairCompare<OrderTime>,
                                 PairDegradedCompare<OrderTime> > >(
          name + "_order_db", odcomp, odcomp_d, 256);
  pending_db_ = std::make_unique<
      BPlusTree<TrainDateOrder, PendingInfo, TDOCompare, TDODegradedCompare> >(
      name + "_pending_db", tdocomp, tdocomp_d, 256);
  station_db_ = std::make_unique<
      BPlusTree<StationTrain, StationTrainInfo, PairCompare<StationTrain>,
                PairDegradedCompare<StationTrain> > >(name + "_station_db",
                                                      stcomp, stcomp_d, 256);
}

void Ticket::QueryTicket(std::string &from, std::string &to, num_t date,
                         std::string comp) {
  auto from_hash = ToHash(from);
  auto to_hash = ToHash(to);
  vector<StationTrainInfo> from_vector, to_vector;
  station_db_->GetAllValue(StationTrain(from_hash, 0), &from_vector);
  station_db_->GetAllValue(StationTrain(to_hash, 0), &to_vector);
  if (from_vector.empty() || to_vector.empty()) {
    std::cout << "0\n";
    return;
  }
  map<hash_t, int> train_map;
  int vec_size = from_vector.size();
  for (int i = 0; i < vec_size; ++i) {
    auto early_date =
        from_vector[i].saleDate.first + from_vector[i].arrivingTime.date;
    auto late_date =
        from_vector[i].saleDate.second + from_vector[i].leavingTime.date;
    if (early_date <= date && date <= late_date) {
      train_map.insert(from_vector[i].trainID_hash, i);
    }
  }

  if (comp == "time") {
    sjtu::priority_queue<TicketComp, SortByTime> queue;

    for (auto &train : to_vector) {
      auto it = train_map.find(train.trainID_hash);
      if (it != train_map.end()) {
        if (from_vector[it->second].station_index >= train.station_index) {
          continue;
        }
        vector<TicketDateInfo> ticketNum;
        if (date - from_vector[it->second].leavingTime.date <
            from_vector[it->second].saleDate.first) {
          continue;
        }
        ticket_db_->GetValue(
            TrainDate(train.trainID_hash,
                      date - from_vector[it->second].leavingTime.date),
            &ticketNum);
        auto t_num = ticketNum[0].getSeat(from_vector[it->second].station_index,
                                          train.station_index);
        TicketComp ticket(
            train.arrivingTime - from_vector[it->second].leavingTime,
            train.price - from_vector[it->second].price, t_num,
            from_vector[it->second].station_index, train.station_index,
            DateTime(date, from_vector[it->second].leavingTime.time),
            DateTime(date + train.arrivingTime.date -
                         from_vector[it->second].leavingTime.date,
                     train.arrivingTime.time),
            train.trainID_hash, train.trainID);
        queue.push(ticket);
      }
    }
    std::cout << queue.size() << '\n';
    while (!queue.empty()) {
      auto ticket = queue.top();
      queue.pop();
      std::cout << ticket.trainID << ' ' << from << ' ' << ticket.leavingTime
                << " -> " << to << ' ' << ticket.arrivingTime << ' '
                << ticket.cost << ' ' << ticket.ticket_num << '\n';
    }
  } else {
    sjtu::priority_queue<TicketComp, SortByCost> queue;
    for (auto &train : to_vector) {
      auto it = train_map.find(train.trainID_hash);
      if (it != train_map.end()) {
        if (from_vector[it->second].station_index >= train.station_index) {
          continue;
        }
        vector<TicketDateInfo> ticketNum;
        if (date - from_vector[it->second].leavingTime.date <
            from_vector[it->second].saleDate.first) {
          continue;
        }
        ticket_db_->GetValue(
            TrainDate(train.trainID_hash,
                      date - from_vector[it->second].leavingTime.date),
            &ticketNum);

        auto t_num = ticketNum[0].getSeat(from_vector[it->second].station_index,
                                          train.station_index);
        TicketComp ticket(
            train.arrivingTime - from_vector[it->second].leavingTime,
            train.price - from_vector[it->second].price, t_num,
            from_vector[it->second].station_index, train.station_index,
            DateTime(date, from_vector[it->second].leavingTime.time),
            DateTime(date + train.arrivingTime.date -
                         from_vector[it->second].leavingTime.date,
                     train.arrivingTime.time),
            train.trainID_hash, train.trainID);
        queue.push(ticket);
      }
    }
    std::cout << queue.size() << '\n';
    while (!queue.empty()) {
      auto ticket = queue.top();
      queue.pop();

      std::cout << ticket.trainID << ' ' << from << ' ' << ticket.leavingTime
                << " -> " << to << ' ' << ticket.arrivingTime << ' '
                << ticket.cost << ' ' << ticket.ticket_num << '\n';
    }
  }
}

// TODO::Fatial bug in date count in transfer and refund
void Ticket::QueryTransfer(Train *train_system, std::string &from,
                           std::string &to, num_t date, std::string comp) {
  auto from_hash = ToHash(from);
  auto to_hash = ToHash(to);
  vector<StationTrainInfo> from_vector, to_vector;
  station_db_->GetAllValue(StationTrain(from_hash, 0), &from_vector);
  station_db_->GetAllValue(StationTrain(to_hash, 0), &to_vector);
  if (from_vector.empty() || to_vector.empty()) {
    std::cout << "0\n";
    return;
  }
  map<hash_t, int> train_map;
  int vec_size = to_vector.size();
  for (int i = 0; i < vec_size; ++i) {
    train_map.insert(to_vector[i].trainID_hash, i);
  }
  list<StationTrainInfo> train_list;
  vec_size = from_vector.size();
  for (int i = 0; i < vec_size; ++i) {
    auto early_date =
        from_vector[i].saleDate.first + from_vector[i].arrivingTime.date;
    auto late_date =
        from_vector[i].saleDate.second + from_vector[i].leavingTime.date;
    if (early_date <= date && date <= late_date) {
      train_list.push_back(from_vector[i]);
    }
  }

  if (comp == "time") {
    priority_queue<TicketTransComp, TranSortByTime> queue;
    for (auto &train : train_list) {
      vector<TrainMeta> train_meta;
      train_system->train_db_->GetValue(train.trainID_hash, &train_meta);
      auto trainInfo =
          train_system->train_manager_->ReadPage(train_meta[0].page_id)
              .As<TrainInfo>();

      auto leaveTime = DateTime(date, train.leavingTime.time);
      auto arriveTime = leaveTime;

      auto init_leaveTime = leaveTime;
      int cost = 0;

      for (int i = train.station_index + 1; i < trainInfo->stationNum; ++i) {
        auto station_hash = ToHash(trainInfo->stations[i]);
        arriveTime = leaveTime;
        arriveTime += trainInfo->travelTimes[i - 1];
        leaveTime = arriveTime;
        if (i != trainInfo->stationNum - 1) {
          leaveTime += trainInfo->stopoverTimes[i - 1];
        } else {
          leaveTime.date += 100;
        }
        cost += trainInfo->prices[i - 1];

        vector<StationTrainInfo> trans_vector;
        station_db_->GetAllValue(StationTrain(station_hash, 0), &trans_vector);
        for (auto &trans : trans_vector) {
          auto it = train_map.find(trans.trainID_hash);
          if (it == train_map.end() ||
              trans.trainID_hash == train.trainID_hash ||
              trans.station_index >= to_vector[it->second].station_index) {
            continue;
          }

          // now trans refers to a train that go from station[i] to Destination
          // next we should check what days are available
          // the only requirement is leavetime_2>=arrivetime_1
          if (trans.saleDate.second + trans.leavingTime.date <
              arriveTime.date) {
            continue;
          }
          DateTime latetime(arriveTime.date, trans.leavingTime.time);

          if (latetime < arriveTime) {
            latetime.date++;
          }
          if (latetime.date > trans.saleDate.second + trans.leavingTime.date) {
            continue;
          }
          if (latetime.date < trans.saleDate.first + trans.leavingTime.date) {
            latetime.date = trans.saleDate.first + trans.leavingTime.date;
          }

          auto &to_train = to_vector[it->second];
          vector<TicketDateInfo> ticketNum_1, ticketNum_2;
          ticket_db_->GetValue(
              TrainDate(train.trainID_hash, date - train.leavingTime.date),
              &ticketNum_1);
          ticket_db_->GetValue(
              TrainDate(trans.trainID_hash,
                        latetime.date - trans.leavingTime.date),
              &ticketNum_2);
          auto t_num_1 = ticketNum_1[0].getSeat(train.station_index, i);
          auto t_num_2 = ticketNum_2[0].getSeat(trans.station_index,
                                                to_train.station_index);
          TicketComp ticket_1(arriveTime - init_leaveTime, cost, t_num_1,
                              train.station_index, i, init_leaveTime,
                              arriveTime, train.trainID_hash, train.trainID);
          TicketComp ticket_2(
              to_train.arrivingTime - latetime, to_train.price - trans.price,
              t_num_2, trans.station_index, to_train.station_index, latetime,
              DateTime(latetime.date - trans.leavingTime.date +
                           to_train.arrivingTime.date,
                       to_train.arrivingTime.time),
              trans.trainID_hash, trans.trainID);
          TicketTransComp ticket(ticket_1, ticket_2, trainInfo->stations[i]);
          queue.push(ticket);
        }
      }
    }
    if (queue.empty()) {
      std::cout << "0\n";
      return;
    } else {
      auto ticket = queue.top();
      std::cout << ticket.ticket_1.trainID << ' ' << from << ' '
                << ticket.ticket_1.leavingTime << " -> " << ticket.station
                << ' ' << ticket.ticket_1.arrivingTime << ' '
                << ticket.ticket_1.cost << ' ' << ticket.ticket_1.ticket_num
                << '\n'
                << ticket.ticket_2.trainID << ' ' << ticket.station << ' '
                << ticket.ticket_2.leavingTime << " -> " << to << ' '
                << ticket.ticket_2.arrivingTime << ' ' << ticket.ticket_2.cost
                << ' ' << ticket.ticket_2.ticket_num << '\n';
    }
  } else {
    priority_queue<TicketTransComp, TranSortByCost> queue;
    for (auto &train : train_list) {
      vector<TrainMeta> train_meta;
      train_system->train_db_->GetValue(train.trainID_hash, &train_meta);
      auto trainInfo =
          train_system->train_manager_->ReadPage(train_meta[0].page_id)
              .As<TrainInfo>();

      auto leaveTime = DateTime(date, train.leavingTime.time);
      auto arriveTime = leaveTime;

      auto init_leaveTime = leaveTime;
      int cost = 0;

      for (int i = train.station_index + 1; i < trainInfo->stationNum; ++i) {
        auto station_hash = ToHash(trainInfo->stations[i]);
        arriveTime = leaveTime;
        arriveTime += trainInfo->travelTimes[i - 1];
        leaveTime = arriveTime;
        if (i != trainInfo->stationNum - 1) {
          leaveTime += trainInfo->stopoverTimes[i - 1];
        } else {
          leaveTime.date += 100;
        }
        cost += trainInfo->prices[i - 1];

        vector<StationTrainInfo> trans_vector;
        station_db_->GetAllValue(StationTrain(station_hash, 0), &trans_vector);
        for (auto &trans : trans_vector) {
          auto it = train_map.find(trans.trainID_hash);
          if (it == train_map.end() ||
              trans.trainID_hash == train.trainID_hash ||
              trans.station_index >= to_vector[it->second].station_index) {
            continue;
          }

          if (trans.saleDate.second + trans.leavingTime.date <
              arriveTime.date) {
            continue;
          }
          DateTime latetime(arriveTime.date, trans.leavingTime.time);

          if (latetime < arriveTime) {
            latetime.date++;
          }
          if (latetime.date > trans.saleDate.second + trans.leavingTime.date) {
            continue;
          }
          if (latetime.date < trans.saleDate.first + trans.leavingTime.date) {
            latetime.date = trans.saleDate.first + trans.leavingTime.date;
          }

          auto &to_train = to_vector[it->second];
          vector<TicketDateInfo> ticketNum_1, ticketNum_2;
          ticket_db_->GetValue(
              TrainDate(train.trainID_hash, date - train.leavingTime.date),
              &ticketNum_1);
          ticket_db_->GetValue(
              TrainDate(trans.trainID_hash,
                        latetime.date - trans.leavingTime.date),
              &ticketNum_2);
          auto t_num_1 = ticketNum_1[0].getSeat(train.station_index, i);
          auto t_num_2 = ticketNum_2[0].getSeat(trans.station_index,
                                                to_train.station_index);
          TicketComp ticket_1(arriveTime - init_leaveTime, cost, t_num_1,
                              train.station_index, i, init_leaveTime,
                              arriveTime, train.trainID_hash, train.trainID);
          TicketComp ticket_2(
              to_train.arrivingTime - latetime, to_train.price - trans.price,
              t_num_2, trans.station_index, to_train.station_index, latetime,
              DateTime(latetime.date - trans.leavingTime.date +
                           to_train.arrivingTime.date,
                       to_train.arrivingTime.time),
              trans.trainID_hash, trans.trainID);
          TicketTransComp ticket(ticket_1, ticket_2, trainInfo->stations[i]);
          queue.push(ticket);
        }
      }
    }
    if (queue.empty()) {
      std::cout << "0\n";
      return;
    } else {
      auto ticket = queue.top();
      std::cout << ticket.ticket_1.trainID << ' ' << from << ' '
                << ticket.ticket_1.leavingTime << " -> " << ticket.station
                << ' ' << ticket.ticket_1.arrivingTime << ' '
                << ticket.ticket_1.cost << ' ' << ticket.ticket_1.ticket_num
                << '\n'
                << ticket.ticket_2.trainID << ' ' << ticket.station << ' '
                << ticket.ticket_2.leavingTime << " -> " << to << ' '
                << ticket.ticket_2.arrivingTime << ' ' << ticket.ticket_2.cost
                << ' ' << ticket.ticket_2.ticket_num << '\n';
    }
  }
}

void Ticket::BuyTicket(int timestamp, std::string &username,
                       std::string &trainID, num_t date, int num,
                       std::string &from, std::string &to, std::string queue) {
  auto user_hash = ToHash(username);
  auto train_hash = ToHash(trainID);
  if (!user_->IsLogged(username)) {
    std::cout << "-1\n";
    return;
  }
  auto from_hash = ToHash(from);
  vector<StationTrainInfo> from_vector;
  station_db_->GetValue(StationTrain(from_hash, train_hash), &from_vector);
  auto to_hash = ToHash(to);
  vector<StationTrainInfo> to_vector;
  station_db_->GetValue(StationTrain(to_hash, train_hash), &to_vector);
  if (from_vector.empty() || to_vector.empty()) {
    std::cout << "-1\n";
    return;
  }
  auto &from_station = from_vector[0], &to_station = to_vector[0];
  if (from_vector[0].saleDate.first > date ||
      from_vector[0].saleDate.second < date ||
      from_vector[0].station_index >= to_vector[0].station_index) {
    std::cout << "-1\n";
    return;
  }
  auto leavingTime = DateTime(date, from_station.leavingTime.time);
  auto arrivingTime = DateTime(
      date + to_station.arrivingTime.date - from_station.leavingTime.date,
      to_station.arrivingTime.time);

  auto init_date = date - from_station.leavingTime.date;
  vector<TicketDateInfo> ticket_vector;
  ticket_db_->GetValue(TrainDate(train_hash, init_date), &ticket_vector);
  if (ticket_vector.empty() || ticket_vector[0].seatMaxNum < num) {
    std::cout << "-1\n";
    return;
  }
  auto ticket_num = ticket_vector[0].getSeat(from_station.station_index,
                                             to_station.station_index);
  if (ticket_num >= num) {
    ticket_vector[0].changeSeat(from_station.station_index,
                                to_station.station_index, -num);
    ticket_db_->Remove(TrainDate(train_hash, init_date));
    ticket_db_->Insert(TrainDate(train_hash, init_date), ticket_vector[0]);
    auto price = to_station.price - from_station.price;
    OrderInfo order(timestamp, TicketStatus::Success, trainID.c_str(),
                    from.c_str(), to.c_str(), from_station.station_index,
                    to_station.station_index, leavingTime, arrivingTime, price,
                    num, init_date);
    order_db_->Insert(OrderTime(user_hash, timestamp), order);
    std::cout << price * num << '\n';
  } else {
    if (queue == "false") {
      std::cout << "-1\n";
      return;
    }
    auto price = to_station.price - from_station.price;
    OrderInfo order(timestamp, TicketStatus::Pending, trainID.c_str(),
                    from.c_str(), to.c_str(), from_station.station_index,
                    to_station.station_index, leavingTime, arrivingTime, price,
                    num, init_date);
    order_db_->Insert(OrderTime(user_hash, timestamp), order);
    PendingInfo pending_info{timestamp,
                             user_hash,
                             train_hash,
                             from_station.station_index,
                             to_station.station_index,
                             num,
                             (num_t)init_date};
    pending_db_->Insert(
        TrainDateOrder(TrainDate(train_hash, init_date), timestamp),
        pending_info);
    std::cout << "queue\n";
  }
}

void Ticket::RefundTicket(std::string &username, int n) {
  auto user_hash = ToHash(username);
  if (!user_->IsLogged(username)) {
    std::cout << "-1\n";
    return;
  }
  vector<OrderInfo> user_order_info;
  order_db_->GetAllValue(OrderTime(user_hash, -1), &user_order_info);
  auto order_size = user_order_info.size();
  if (order_size < n) {
    std::cout << "-1\n";
    return;
  }
  auto &obj_order = user_order_info[order_size - n];
  if (obj_order.status != TicketStatus::Success) {
    if (obj_order.status == TicketStatus::Pending) {
      obj_order.status = TicketStatus::Refunded;
      order_db_->Remove(OrderTime(user_hash, obj_order.timestamp));
      order_db_->Insert(OrderTime(user_hash, obj_order.timestamp), obj_order);
      pending_db_->Remove(TrainDateOrder(
          TrainDate(ToHash(obj_order.trainID), obj_order.init_date),
          obj_order.timestamp));
      std::cout << "0\n";
      return;
    }
    std::cout << "-1\n";
    return;
  }
  auto train_hash = ToHash(obj_order.trainID);
  obj_order.status = TicketStatus::Refunded;
  order_db_->Remove(OrderTime(user_hash, obj_order.timestamp));
  order_db_->Insert(OrderTime(user_hash, obj_order.timestamp), obj_order);
  vector<TicketDateInfo> ticket_vector;
  ticket_db_->GetValue(TrainDate(train_hash, obj_order.init_date),
                       &ticket_vector);
  auto &cur_ticket = ticket_vector[0];
  cur_ticket.changeSeat(obj_order.from_index, obj_order.to_index,
                        obj_order.num);
  vector<PendingInfo> pending_vector;
  pending_db_->GetAllValue(
      TrainDateOrder(TrainDate(train_hash, obj_order.init_date), -1),
      &pending_vector);
  auto pending_size = pending_vector.size();
  if (pending_size == 0) {
    ticket_db_->Remove(TrainDate(train_hash, obj_order.init_date));
    ticket_db_->Insert(TrainDate(train_hash, obj_order.init_date), cur_ticket);
    std::cout << "0\n";
    return;
  }
  for (int i = 0; i < pending_size; ++i) {
    auto &pending_order = pending_vector[i];
    if (pending_order.num <=
        cur_ticket.getSeat(pending_order.from_index, pending_order.to_index)) {
      cur_ticket.changeSeat(pending_order.from_index, pending_order.to_index,
                            -pending_order.num);
      vector<OrderInfo> order_info;
      order_db_->GetValue(
          OrderTime(pending_order.username_hash, pending_order.timestamp),
          &order_info);
      order_info[0].status = TicketStatus::Success;
      order_db_->Remove(
          OrderTime(pending_order.username_hash, pending_order.timestamp));
      order_db_->Insert(
          OrderTime(pending_order.username_hash, pending_order.timestamp),
          order_info[0]);
      pending_db_->Remove(TrainDateOrder(
          TrainDate(train_hash, obj_order.init_date), pending_order.timestamp));
    }
  }
  ticket_db_->Remove(TrainDate(train_hash, obj_order.init_date));
  ticket_db_->Insert(TrainDate(train_hash, obj_order.init_date), cur_ticket);
  std::cout << "0\n";
}

void Ticket::QueryOrder(std::string &username) {
  auto user_hash = ToHash(username);
  if (!user_->IsLogged(username)) {
    std::cout << "-1\n";
    return;
  }
  vector<OrderInfo> order_vector;
  order_db_->GetAllValue(OrderTime(user_hash, -1), &order_vector);
  std::cout << order_vector.size() << '\n';
  for (int i = order_vector.size() - 1; i >= 0; --i) {
    auto &order = order_vector[i];
    if (order.status == TicketStatus::Success) {
      std::cout << "[success] ";
    } else if (order.status == TicketStatus::Pending) {
      std::cout << "[pending] ";
    } else {
      std::cout << "[refunded] ";
    }
    std::cout << order.trainID << ' ' << order.from << ' ' << order.leavingTime
              << " -> " << order.to << ' ' << order.arrivingTime << ' '
              << order.price << ' ' << order.num << '\n';
  }
}
}  // namespace sjtu