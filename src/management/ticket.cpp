#include "management/ticket.h"

namespace sjtu {
  Ticket::Ticket(std::string &name, User *user): user_(user) {
    HashComp hashcomp;
    PairCompare<TrainDate> tdcomp;
    PairDegradedCompare<TrainDate> tdcomp_d;
    PairCompare<OrderTime> odcomp;
    PairDegradedCompare<OrderTime> odcomp_d;
    ticket_db_ = std::make_unique<BPlusTree<TrainDate, TicketDateInfo, PairCompare<TrainDate>,
      PairDegradedCompare<TrainDate> > >(name + "_ticket_db", tdcomp, tdcomp_d, 512);
    order_db_ = std::make_unique<BPlusTree<OrderTime, OrderInfo, PairCompare<OrderTime>,
      PairDegradedCompare<OrderTime> > >(name + "_order_db", odcomp, odcomp_d, 256);
    pending_db_ = std::make_unique<BPlusTree<TrainDate, PendingInfo, PairCompare<TrainDate>,
      PairCompare<TrainDate> > >(name + "_pending_db", tdcomp, tdcomp, 128);
    station_db_ = std::make_unique<BPlusTree<hash_t, StationTrainInfo, HashComp, HashComp> >(
      name + "_station_db", hashcomp, hashcomp, 256);
  }


  void Ticket::QueryTicket(std::string &from, std::string &to, num_t date, std::string comp) {
    auto from_hash = ToHash(from);
    auto to_hash = ToHash(to);
    vector<StationTrainInfo> from_vector, to_vector;
    station_db_->GetValue(from_hash, &from_vector);
    station_db_->GetValue(to_hash, &to_vector);
    if (from_vector.empty() || to_vector.empty()) {
      std::cout << "0\n";
      return;
    }
    map<hash_t,int>train_map;
    int vec_size=from_vector.size();
    for (int i=0;i<vec_size;++i) {
      auto early_date=from_vector[i].saleDate.first+from_vector[i].arrivingTime.date;
      auto late_date=from_vector[i].saleDate.second+from_vector[i].leavingTime.date;
      if(early_date<=date&&date<=late_date) {
        train_map.insert(from_vector[i].trainID_hash,i);
      }
    }
    for(auto train:to_vector) {
      auto it=train_map.find(train.trainID_hash);
      if(it!=train_map.end()) {

      }
    }
  }
}
