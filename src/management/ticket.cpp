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
  ticket_db_ = std::make_unique<
    BPlusTree<TrainDate, TicketDateInfo, PairCompare<TrainDate>,
              PairDegradedCompare<TrainDate> > >(name + "_ticket_db",
                                                 tdcomp, tdcomp_d, 512);
  order_db_ =
      std::make_unique<BPlusTree<OrderTime, OrderInfo, PairCompare<OrderTime>,
                                 PairDegradedCompare<OrderTime> > >(
          name + "_order_db", odcomp, odcomp_d, 256);
  pending_db_ = std::make_unique<
    BPlusTree<TrainDate, PendingInfo, PairCompare<TrainDate>,
              PairCompare<TrainDate> > >(name + "_pending_db", tdcomp,
                                         tdcomp, 128);
  station_db_ = std::make_unique<
    BPlusTree<hash_t, StationTrainInfo, HashComp, HashComp> >(
      name + "_station_db", hashcomp, hashcomp, 256);
}

void Ticket::QueryTicket(std::string &from, std::string &to, num_t date,
                         std::string comp) {
  auto from_hash = ToHash(from);
  auto to_hash = ToHash(to);
  vector<StationTrainInfo> from_vector, to_vector;
  station_db_->GetValue(from_hash, &from_vector);
  station_db_->GetValue(to_hash, &to_vector);
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
        TicketComp ticket(
            train.arrivingTime - from_vector[it->second].arrivingTime,
            train.price - from_vector[it->second].price,
            from_vector[it->second].station_index, train.station_index,
            DateTime(from_vector[it->second].leavingTime.date + date,
                     from_vector[it->second].leavingTime.time),
            DateTime(train.arrivingTime.date + date,
                     train.arrivingTime.time),
            train.trainID_hash, train.trainID);
        queue.push(ticket);
      }
      std::cout << queue.size() << '\n';
      while (!queue.empty()) {
        auto ticket = queue.top();
        queue.pop();
        vector<TicketDateInfo> ticketNum;
        ticket_db_->GetValue(TrainDate(ticket.trainID_hash, date),
                             &ticketNum);
        std::cout << ticket.trainID << ' ' << from << ' '
            << ticket.leavingTime << " -> " << to << ' '
            << ticket.arrivingTime << ' ' << ticket.cost << ' '
            << ticketNum[0].getSeat(ticket.station_index_1,
                                    ticket.station_index_2)
            << '\n';
      }
    }
  } else {
    sjtu::priority_queue<TicketComp, SortByCost> queue;
    for (auto train : to_vector) {
      auto it = train_map.find(train.trainID_hash);
      if (it != train_map.end()) {
        TicketComp ticket(
            train.arrivingTime - from_vector[it->second].arrivingTime,
            train.price - from_vector[it->second].price,
            from_vector[it->second].station_index, train.station_index,
            DateTime(from_vector[it->second].leavingTime.date + date,
                     from_vector[it->second].leavingTime.time),
            DateTime(train.arrivingTime.date + date,
                     train.arrivingTime.time),
            train.trainID_hash, train.trainID);
        queue.push(ticket);
      }
      std::cout << queue.size() << '\n';
      while (!queue.empty()) {
        auto ticket = queue.top();
        queue.pop();
        vector<TicketDateInfo> ticketNum;
        ticket_db_->GetValue(TrainDate(ticket.trainID_hash, date),
                             &ticketNum);
        std::cout << ticket.trainID << ' ' << from << ' '
            << ticket.leavingTime << " -> " << to << ' '
            << ticket.arrivingTime << ' ' << ticket.cost << ' '
            << ticketNum[0].getSeat(ticket.station_index_1,
                                    ticket.station_index_2)
            << '\n';
      }
    }
  }
}

void Ticket::QueryTransfer(Train *train_system, std::string &from,
                           std::string &to, num_t date, std::string comp) {
  auto from_hash = ToHash(from);
  auto to_hash = ToHash(to);
  vector<StationTrainInfo> from_vector, to_vector;
  station_db_->GetValue(from_hash, &from_vector);
  station_db_->GetValue(to_hash, &to_vector);
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
      auto trainInfo = train_system->train_manager_->ReadPage(
          train_meta[0].page_id).As<TrainInfo>();

      auto leaveTime = DateTime(train.leavingTime.date + date,
                                train.leavingTime.time);
      auto arriveTime = leaveTime;

      auto init_leaveTime = leaveTime;
      int cost = 0;

      for (int i = train.station_index + 1; i < trainInfo->stationNum; ++i) {
        auto station_hash = ToHash(trainInfo->stations[i]);
        arriveTime += trainInfo->travelTimes[i - 1];
        leaveTime = arriveTime;
        if (i != trainInfo->stationNum - 1) {
          leaveTime += trainInfo->stopoverTimes[i - 1];
        } else {
          leaveTime.date += 100;
        }
        cost += trainInfo->prices[i - 1];

        vector<StationTrainInfo> trans_vector;
        station_db_->GetValue(station_hash, &trans_vector);
        for (auto &trans : trans_vector) {
          auto it = train_map.find(trans.trainID_hash);
          if (it == train_map.end() || trans.trainID_hash == train.trainID_hash
              ||
              trans.station_index >= to_vector[it->second].station_index) {
            continue;
          }

          //now trans refers to a train that go from station[i] to Destination
          //next we should check what days are available
          if (trans.saleDate.first + trans.arrivingTime.date > leaveTime.date ||
              trans.saleDate.second + trans.leavingTime.date < arriveTime.
              date) {
            continue;
          }
          DateTime earlytime(trans.saleDate.first + trans.arrivingTime.date,
                             trans.arrivingTime.time);
          DateTime latetime(trans.saleDate.first + trans.leavingTime.date,
                            trans.leavingTime.time);

          if (earlytime.time > arriveTime.time) {
            earlytime.date = arriveTime.date - 1;
            latetime.date = earlytime.date - trans.arrivingTime.date + trans.
                            leavingTime.date;
          } else {
            earlytime.date = arriveTime.date;
            latetime.date = earlytime.date - trans.arrivingTime.date + trans.
                            leavingTime.date;
          }

          while (TransferPossible(arriveTime, leaveTime, earlytime, latetime)) {
            auto trans_date = earlytime.date - trans.arrivingTime.date;
            if (trans_date > trans.saleDate.second) {
              break;
            }
            auto to_train = to_vector[it->second];
            TicketComp ticket_1(arriveTime - init_leaveTime, cost,
                                train.station_index, i, init_leaveTime,
                                arriveTime, train.trainID_hash, train.trainID);
            TicketComp ticket_2(to_train.arrivingTime - latetime,
                                to_train.price - trans.price,
                                trans.station_index,
                                to_train.station_index, latetime,
                                DateTime(
                                    trans_date + to_train.arrivingTime.date,
                                    to_train.arrivingTime.time),
                                trans.trainID_hash, trans.trainID);
            TicketTransComp ticket(ticket_1, ticket_2, trainInfo->stations[i]);
            queue.push(ticket);
            earlytime.date++;
            latetime.date++;
          }
        }
      }
    }
    if (queue.empty()) {
      std::cout << "0\n";
      return;
    } else {
      auto ticket = queue.top();
      vector<TicketDateInfo> ticketNum_1, ticketNum_2;
      ticket_db_->GetValue(TrainDate(ticket.ticket_1.trainID_hash,
                                     ticket.ticket_1.leavingTime.date),
                           &ticketNum_1);
      ticket_db_->GetValue(TrainDate(ticket.ticket_2.trainID_hash,
                                     ticket.ticket_2.leavingTime.date),
                           &ticketNum_2);
      std::cout << ticket.ticket_1.trainID << ' ' << from << ' '
          << ticket.ticket_1.leavingTime << " -> " << ticket.station << ' '
          << ticket.ticket_1.arrivingTime << ' ' << ticket.ticket_1.cost << ' '
          << ticketNum_1[0].getSeat(ticket.ticket_1.station_index_1,
                                    ticket.ticket_1.station_index_2)
          << '\n' << ticket.ticket_2.trainID << ' ' << ticket.station << ' '
          << ticket.ticket_2.leavingTime << " -> " << to << ' '
          << ticket.ticket_2.arrivingTime << ' ' << ticket.ticket_2.cost << ' '
          << ticketNum_2[0].getSeat(ticket.ticket_2.station_index_1,
                                    ticket.ticket_2.station_index_2)
          << '\n';
    }
  } else {
    priority_queue<TicketTransComp, TranSortByCost> queue;
    for (auto &train : train_list) {
      vector<TrainMeta> train_meta;
      train_system->train_db_->GetValue(train.trainID_hash, &train_meta);
      auto trainInfo = train_system->train_manager_->ReadPage(
          train_meta[0].page_id).As<TrainInfo>();

      auto leaveTime = DateTime(train.leavingTime.date + date,
                                train.leavingTime.time);
      auto arriveTime = leaveTime;

      auto init_leaveTime = leaveTime;
      int cost = 0;

      for (int i = train.station_index + 1; i < trainInfo->stationNum; ++i) {
        auto station_hash = ToHash(trainInfo->stations[i]);
        arriveTime += trainInfo->travelTimes[i - 1];
        leaveTime = arriveTime;
        if (i != trainInfo->stationNum - 1) {
          leaveTime += trainInfo->stopoverTimes[i - 1];
        } else {
          leaveTime.date += 100;
        }
        cost += trainInfo->prices[i - 1];

        vector<StationTrainInfo> trans_vector;
        station_db_->GetValue(station_hash, &trans_vector);
        for (auto &trans : trans_vector) {
          auto it = train_map.find(trans.trainID_hash);
          if (it == train_map.end() || trans.trainID_hash == train.trainID_hash
              ||
              trans.station_index >= to_vector[it->second].station_index) {
            continue;
          }

          //now trans refers to a train that go from station[i] to Destination
          //next we should check what days are available
          if (trans.saleDate.first + trans.arrivingTime.date > leaveTime.date ||
              trans.saleDate.second + trans.leavingTime.date < arriveTime.
              date) {
            continue;
          }
          DateTime earlytime(trans.saleDate.first + trans.arrivingTime.date,
                             trans.arrivingTime.time);
          DateTime latetime(trans.saleDate.first + trans.leavingTime.date,
                            trans.leavingTime.time);

          if (earlytime.time > arriveTime.time) {
            earlytime.date = arriveTime.date - 1;
            latetime.date = earlytime.date - trans.arrivingTime.date + trans.
                            leavingTime.date;
          } else {
            earlytime.date = arriveTime.date;
            latetime.date = earlytime.date - trans.arrivingTime.date + trans.
                            leavingTime.date;
          }

          while (TransferPossible(arriveTime, leaveTime, earlytime, latetime)) {
            auto trans_date = earlytime.date - trans.arrivingTime.date;
            if (trans_date > trans.saleDate.second) {
              break;
            }
            auto to_train = to_vector[it->second];
            TicketComp ticket_1(arriveTime - init_leaveTime, cost,
                                train.station_index, i, init_leaveTime,
                                arriveTime, train.trainID_hash, train.trainID);
            TicketComp ticket_2(to_train.arrivingTime - latetime,
                                to_train.price - trans.price,
                                trans.station_index,
                                to_train.station_index, latetime,
                                DateTime(
                                    trans_date + to_train.arrivingTime.date,
                                    to_train.arrivingTime.time),
                                trans.trainID_hash, trans.trainID);
            TicketTransComp ticket(ticket_1, ticket_2, trainInfo->stations[i]);
            queue.push(ticket);
            earlytime.date++;
            latetime.date++;
          }
        }
      }
    }
    if (queue.empty()) {
      std::cout << "0\n";
      return;
    } else {
      auto ticket = queue.top();
      vector<TicketDateInfo> ticketNum_1, ticketNum_2;
      ticket_db_->GetValue(TrainDate(ticket.ticket_1.trainID_hash,
                                     ticket.ticket_1.leavingTime.date),
                           &ticketNum_1);
      ticket_db_->GetValue(TrainDate(ticket.ticket_2.trainID_hash,
                                     ticket.ticket_2.leavingTime.date),
                           &ticketNum_2);
      std::cout << ticket.ticket_1.trainID << ' ' << from << ' '
          << ticket.ticket_1.leavingTime << " -> " << ticket.station << ' '
          << ticket.ticket_1.arrivingTime << ' ' << ticket.ticket_1.cost << ' '
          << ticketNum_1[0].getSeat(ticket.ticket_1.station_index_1,
                                    ticket.ticket_1.station_index_2)
          << '\n' << ticket.ticket_2.trainID << ' ' << ticket.station << ' '
          << ticket.ticket_2.leavingTime << " -> " << to << ' '
          << ticket.ticket_2.arrivingTime << ' ' << ticket.ticket_2.cost << ' '
          << ticketNum_2[0].getSeat(ticket.ticket_2.station_index_1,
                                    ticket.ticket_2.station_index_2)
          << '\n';
    }
  }
}

void Ticket::BuyTicket(std::string &username, std::string &trainID, num_t date,
                       num_t num, std::string &from, std::string &to,
                       std::string queue) {
}

void Ticket::RefundTicket(std::string &username, int n) {
}
} // namespace sjtu