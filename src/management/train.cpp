#include "management/train.h"

namespace sjtu {
  Train::Train(std::string &name) {
    HashComp comp;
    train_db_ = std::make_unique<BPlusTree<hash_t, TrainMeta, HashComp, HashComp> >("train_db", comp, comp, 128);
    train_manager_ = std::make_unique<BufferPoolManager>(512, "train_manager");
    auto header_page = train_manager_->WritePage(0).AsMut<BPlusTreeHeaderPage>();
    if (header_page->next_page_id_ == 0) {
      header_page->next_page_id_ = 1;
    }
    train_manager_->SetNextPageId(header_page->next_page_id_);
  }

  Train::~Train() {
    auto header_page = train_manager_->WritePage(0).AsMut<BPlusTreeHeaderPage>();
    header_page->next_page_id_ = train_manager_->GetNextPageId();
  }

  auto Train::AddTrain(TrainInfo &train) -> bool {
    vector<TrainMeta> train_vector;
    train_db_->GetValue(train.train_id_hash, &train_vector);
    if (!train_vector.empty()) {
      return false;
    }
    TrainMeta train_meta{train_manager_->GetNextPageId(), train.saleDate, false};
    train_db_->Insert(train.train_id_hash, train_meta);
    *train_manager_->WritePage(train_meta.page_id).AsMut<TrainInfo>() = train;
    return true;
  }

  auto Train::DeleteTrain(std::string &trainID) -> bool {
    auto train_hash = ToHash(trainID);
    vector<TrainMeta> train_vector;
    train_db_->GetValue(train_hash, &train_vector);
    if (train_vector.empty() || train_vector[0].is_released) {
      return false;
    }
    train_db_->Remove(train_hash);
    return true;
  }

  void Train::QueryTrain(std::string &trainID, num_t date) {
    auto train_hash = ToHash(trainID);
    vector<TrainMeta> train_vector;
    train_db_->GetValue(train_hash, &train_vector);
    if (train_vector.empty() ) {
      std::cout<<"-1\n";
      return;
    }
    if(train_vector[0].saleDate.second<date||train_vector[0].saleDate.first>date) {
      std::cout<<"-1\n";
      return;
    }
    auto train=train_manager_->ReadPage(train_vector[0].page_id);


  }



}
