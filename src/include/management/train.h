#pragma once

#include <cstring>
#include "storage/b_plus_tree.h"
#include "buffer/buffer_pool_manager.h"
#include "common/config.h"
#include "management/ticket.h"


namespace sjtu {
struct TrainInfo {
    hash_t train_id_hash;
    char trainID[20]{};
    num_t stationNum;
    char stations[100][30]{};
    num_t seatNum;
    int prices[99]{};
    num_t startTime;
    num_t travelTimes[99]{};
    num_t stopoverTimes[98]{};
    DateRange saleDate;
    char type;

    TrainInfo(std::string &i, std::string &n, std::string &m, std::string &s,
              std::string &p, std::string &x, std::string &t, std::string &o,
              std::string &d,std::string &y) {
        train_id_hash=ToHash(i);
        strncpy(trainID,i.c_str(),20);
        stationNum=static_cast<num_t>(std::stoi(n));
        seatNum=static_cast<num_t>(std::stoi(m));
        startTime=TimeToNum(x);
        type=y[0];
        InsertStations(stations,s);
        InsertNum(travelTimes,t);
        InsertNum(stopoverTimes,o);
        InsertNum(prices,p);
        saleDate=DateRange{DateToNum(d.substr(0,5)),DateToNum(d.substr(6,5))};
    }

    TrainInfo& operator=(const TrainInfo &other) {
        train_id_hash = other.train_id_hash;
        stationNum = other.stationNum;
        seatNum = other.seatNum;
        startTime = other.startTime;
        type = other.type;
        saleDate = other.saleDate;
        for (int i = 0; i < stationNum; ++i) {
            strncpy(stations[i], other.stations[i], 30);
        }
        strncpy(trainID, other.trainID, 20);
        memcpy(prices, other.prices, (stationNum - 1) * sizeof(int));
        memcpy(travelTimes, other.travelTimes,
               (stationNum - 1) * sizeof(num_t));
        memcpy(stopoverTimes, other.stopoverTimes,
               (stationNum - 2) * sizeof(num_t));
        return *this;
    }
}; //3832 bytes, stored as single page
//Static info only

struct TrainMeta {
    page_id_t page_id;
    DateRange saleDate;
    bool is_released = false;
};


class Train {
public:
    explicit Train(std::string &name, Ticket *ticket);

    ~Train();

    void AddTrain(TrainInfo &train) ;

    void DeleteTrain(std::string &trainID) ;

    void QueryTrain(std::string &trainID, num_t date);

    void ReleaseTrain(std::string &trainID) ;

private:
    std::unique_ptr<BufferPoolManager> train_manager_;

    std::unique_ptr<BPlusTree<hash_t, TrainMeta, HashComp, HashComp> >
    train_db_;

    Ticket *ticket_;
};
}