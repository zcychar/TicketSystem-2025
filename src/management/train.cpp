#include "management/train.h"

#include "management/ticket.h"

namespace sjtu {
Train::Train(std::string &name, Ticket *ticket) : ticket_(ticket) {
  HashComp comp;
  train_db_ =
      std::make_unique<BPlusTree<hash_t, TrainMeta, HashComp, HashComp> >(
          "train_db", comp, comp, 256);
  train_manager_ = std::make_unique<BufferPoolManager>(128, "train_manager");
  header_page_id_ = train_manager_->NewPage();
  WritePageGuard guard = train_manager_->WritePage(header_page_id_);
  auto root_page = guard.AsMut<BPlusTreeHeaderPage>();
  train_manager_->SetNextPageId(root_page->next_page_id_);
}

Train::~Train() {
  auto header_page =
      train_manager_->WritePage(header_page_id_).AsMut<BPlusTreeHeaderPage>();
  header_page->next_page_id_ = train_manager_->GetNextPageId();
}

void Train::AddTrain(TrainInfo &train) {
  vector<TrainMeta> train_vector;
  train_db_->GetValue(train.train_id_hash, &train_vector);
  if (!train_vector.empty()) {
    std::cout << "-1\n";
    return;
  }
  TrainMeta train_meta{train_manager_->NewPage(), train.saleDate, false};
  train_db_->Insert(train.train_id_hash, train_meta);
  *train_manager_->WritePage(train_meta.page_id).AsMut<TrainInfo>() = train;
  std::cout << "0\n";
}

void Train::DeleteTrain(std::string &trainID) {
  auto train_hash = ToHash(trainID);
  vector<TrainMeta> train_vector;
  train_db_->GetValue(train_hash, &train_vector);
  if (train_vector.empty() || train_vector[0].is_released) {
    std::cout << "-1\n";
    return;
  }
  train_db_->Remove(train_hash);
  std::cout << "0\n";
  return;
}

void Train::QueryTrain(std::string &trainID, num_t date) {
  auto train_hash = ToHash(trainID);
  vector<TrainMeta> train_vector;
  train_db_->GetValue(train_hash, &train_vector);
  if (train_vector.empty() || date <= 0 || date > 92) {
    std::cout << "-1\n";
    return;
  }
  if (train_vector[0].saleDate.second < date ||
      train_vector[0].saleDate.first > date) {
    std::cout << "-1\n";
    return;
  }
  auto train =
      train_manager_->ReadPage(train_vector[0].page_id).As<TrainInfo>();
  std::cout << train->trainID << ' ' << train->type << '\n';
  if (train_vector[0].is_released) {
    sjtu::vector<TicketDateInfo> ticket_vector;
    TrainDate cur{train_hash, date};
    ticket_->ticket_db_->GetValue(cur, &ticket_vector);
    DateTime cur_time(date, train->startTime);
    int cur_price = 0;
    std::cout << train->stations[0] << " xx-xx xx:xx -> " << cur_time << ' '
              << cur_price << ' ' << ticket_vector[0].seatNum[0] << '\n';
    for (int i = 1; i < train->stationNum - 1; ++i) {
      cur_time += train->travelTimes[i - 1];
      std::cout << train->stations[i] << ' ' << cur_time << " -> ";
      cur_time += train->stopoverTimes[i - 1];
      cur_price += train->prices[i - 1];
      std::cout << cur_time << ' ' << cur_price << ' '
                << ticket_vector[0].seatNum[i] << '\n';
    }
    cur_time += train->travelTimes[train->stationNum - 2];
    cur_price += train->prices[train->stationNum - 2];
    std::cout << train->stations[train->stationNum - 1] << ' ' << cur_time
              << " -> xx-xx xx:xx " << cur_price << " x \n";
  } else {
    DateTime cur_time(date, train->startTime);
    int cur_price = 0;
    std::cout << train->stations[0] << " xx-xx xx:xx -> " << cur_time << ' '
              << cur_price << ' ' << train->seatNum << '\n';
    for (int i = 1; i < train->stationNum - 1; ++i) {
      cur_time += train->travelTimes[i - 1];
      std::cout << train->stations[i] << ' ' << cur_time << " -> ";
      cur_time += train->stopoverTimes[i - 1];
      cur_price += train->prices[i - 1];
      std::cout << cur_time << ' ' << cur_price << ' ' << train->seatNum
                << '\n';
    }
    cur_time += train->travelTimes[train->stationNum - 2];
    cur_price += train->prices[train->stationNum - 2];
    std::cout << train->stations[train->stationNum - 1] << ' ' << cur_time
              << " -> xx-xx xx:xx " << cur_price << " x \n";
  }
}

void Train::ReleaseTrain(std::string &trainID) {
  auto train_hash = ToHash(trainID);
  vector<TrainMeta> train_vector;
  train_db_->GetValue(train_hash, &train_vector);
  if (train_vector.empty() || train_vector[0].is_released) {
    std::cout << "-1\n";
    return;
  }
  train_vector[0].is_released = true;
  train_db_->Remove(train_hash);
  train_db_->Insert(train_hash, train_vector[0]);
  auto train =
      train_manager_->ReadPage(train_vector[0].page_id).As<TrainInfo>();
  TicketDateInfo cur_ticket(train->seatNum, train->stationNum);
  for (auto i = train->saleDate.first; i <= train->saleDate.second; ++i) {
    TrainDate train_date{train_hash, i};
    ticket_->ticket_db_->Insert(train_date, cur_ticket);
  }
  auto cur_time = DateTime(0, train->startTime);
  std::string str(train->stations[0]);
  auto station_hash = ToHash(str);
  StationTrainInfo station_train(train->train_id_hash, train->trainID, 0, 0,
                                 train->saleDate, cur_time, cur_time);

  ticket_->station_db_->Insert(StationTrain(station_hash, train_hash),
                               station_train);
  for (auto i = 1; i < train->stationNum - 1; ++i) {
    str = std::string(train->stations[i]);
    station_hash = ToHash(str);
    station_train.station_index = i;
    station_train.price += train->prices[i - 1];
    station_train.arrivingTime = station_train.leavingTime;
    station_train.arrivingTime += train->travelTimes[i - 1];
    station_train.leavingTime = station_train.arrivingTime;
    station_train.leavingTime += train->stopoverTimes[i - 1];
    ticket_->station_db_->Insert(StationTrain(station_hash, train_hash),
                                 station_train);
  }
  str = std::string(train->stations[train->stationNum - 1]);
  station_hash = ToHash(str);
  station_train.station_index = train->stationNum - 1;
  station_train.price += train->prices[train->stationNum - 2];
  station_train.arrivingTime = station_train.leavingTime;
  station_train.arrivingTime += train->travelTimes[train->stationNum - 2];
  station_train.leavingTime = station_train.arrivingTime;
  ticket_->station_db_->Insert(StationTrain(station_hash, train_hash),
                               station_train);
  std::cout << "0\n";
}
}  // namespace sjtu